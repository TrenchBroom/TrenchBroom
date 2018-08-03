/*
 Copyright (C) 2010-2017 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScaleObjectsToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Reference.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/Camera.h"
#include "View/InputState.h"
#include "View/ScaleObjectsTool.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ScaleObjectsToolController::ScaleObjectsToolController(ScaleObjectsTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document)
        {
            ensure(m_tool != nullptr, "tool is null");
        }
        
        ScaleObjectsToolController::~ScaleObjectsToolController() = default;
        
        Tool* ScaleObjectsToolController::doGetTool() {
            return m_tool;
        }

        void ScaleObjectsToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (handleInput(inputState)) {
                doPick(inputState.pickRay(), inputState.camera(), pickResult);
            }
        }

        static std::tuple<DragRestricter*, DragSnapper*, Vec3>
        getDragRestricterSnapperAndInitialPoint(const InputState& inputState,
                                                const Grid& grid,
                                                const Model::Hit& dragStartHit,
                                                const BBox3& bboxAtDragStart) {
            const bool scaleAllAxes = inputState.modifierKeysDown(ModifierKeys::MKShift);

            DragRestricter* restricter = nullptr;
            DragSnapper* snapper = nullptr;

            if (dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHit
                && inputState.camera().orthographicProjection()
                && !scaleAllAxes)
            {
                const Plane3 plane(dragStartHit.hitPoint(), inputState.camera().direction() * -1.0);

                restricter = new PlaneDragRestricter(plane);
                snapper = new DeltaDragSnapper(grid);
            } else {
                assert(dragStartHit.type() == ScaleObjectsTool::ScaleToolFaceHit
                       || dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHit
                       || dragStartHit.type() == ScaleObjectsTool::ScaleToolCornerHit);

                const Line3 handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);

                restricter = new LineDragRestricter(handleLine);
                snapper = new LineDragSnapper(grid, handleLine);
            }

            // HACK: Snap the initial point
            const Vec3 initialPoint = [&]() {
                Vec3 p = dragStartHit.hitPoint();
                restricter->hitPoint(inputState, p);
                snapper->snap(inputState, Vec3::Null, Vec3::Null, p);
                return p;
            }();

            return {restricter, snapper, initialPoint};
        }

        static std::pair<AnchorPos, ProportionalAxes> modifierSettingsForInputState(const InputState& inputState) {
            const auto centerAnchor = inputState.modifierKeysDown(ModifierKeys::MKAlt) ? AnchorPos::Center : AnchorPos::Opposite;

            ProportionalAxes scaleAllAxes = ProportionalAxes::None();
            if (inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                scaleAllAxes = ProportionalAxes::All();

                const auto& camera = inputState.camera();
                if (camera.orthographicProjection()) {
                    // special case for 2D: don't scale along the axis of the camea
                    const size_t cameraComponent = camera.direction().firstComponent();
                    scaleAllAxes.setAxisProportional(cameraComponent, false);
                }
            }

            return {centerAnchor, scaleAllAxes};
        }


        void ScaleObjectsToolController::doModifierKeyChange(const InputState& inputState) {
            const auto [centerAnchor, scaleAllAxes] = modifierSettingsForInputState(inputState);

            if ((centerAnchor != m_tool->anchorPos()) || (scaleAllAxes != m_tool->proportionalAxes())) {
                // update state
                m_tool->setProportionalAxes(scaleAllAxes);
                m_tool->setAnchorPos(centerAnchor);

                if (thisToolDragging()) {
                    const auto tuple = getDragRestricterSnapperAndInitialPoint(inputState, m_document.lock()->grid(), m_tool->dragStartHit(), m_tool->bboxAtDragStart());

                    // false to keep the initial point. This is necessary to get the right behaviour when switching proportional scaling on and off.
                    setRestricter(inputState, std::get<0>(tuple), false);
                    setSnapper(inputState, std::get<1>(tuple), false);

                    // Re-trigger the dragging logic with a delta of 0, so the new modifiers are applied right away.
                    // TODO: Feels like there should be a clearer API for this
                    doDrag(inputState, currentHandlePosition(), currentHandlePosition());
                }
            }

            // Mouse might be over a different handle now
            m_tool->refreshViews();
        }
        
        void ScaleObjectsToolController::doMouseMove(const InputState& inputState) {
            if (handleInput(inputState) && !anyToolDragging(inputState)) {
                m_tool->updateDragFaces(inputState.pickResult());
            }
        }

        // RestrictedDragPolicy

        RestrictedDragPolicy::DragInfo ScaleObjectsToolController::doStartDrag(const InputState& inputState) {
            // based on CreateSimpleBrushToolController3D::doStartDrag

            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return DragInfo();
            }
            if (!m_tool->applies()) {
                return DragInfo();
            }

            MapDocumentSPtr document = lock(m_document);

            const Model::PickResult& pickResult = inputState.pickResult();

            // TODO: why did .pickable() break it?
            const Model::Hit& hit = pickResult.query().type(
                    ScaleObjectsTool::ScaleToolFaceHit
                    | ScaleObjectsTool::ScaleToolEdgeHit
                    | ScaleObjectsTool::ScaleToolCornerHit).occluded().first();
            if (!hit.isMatch()) {
                return DragInfo();
            }

            m_tool->startScaleWithHit(hit);

            // update modifier settings
            const auto [centerAnchor, scaleAllAxes] = modifierSettingsForInputState(inputState);
            m_tool->setAnchorPos(centerAnchor);
            m_tool->setProportionalAxes(scaleAllAxes);

            const auto tuple = getDragRestricterSnapperAndInitialPoint(inputState, m_document.lock()->grid(), m_tool->dragStartHit(), m_tool->bboxAtDragStart());

            return DragInfo(std::get<0>(tuple),
                            std::get<1>(tuple),
                            std::get<2>(tuple));
        }

        RestrictedDragPolicy::DragResult ScaleObjectsToolController::doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
            //std::cout << "ScaleObjectsToolController::doDrag: last " << lastHandlePosition << " next " << nextHandlePosition << "\n";

            m_lastDragDebug = lastHandlePosition;
            m_currentDragDebug = nextHandlePosition;

            const auto delta = nextHandlePosition - lastHandlePosition;
            m_tool->dragScale(delta);

            return DR_Continue;
        }

        void ScaleObjectsToolController::doEndDrag(const InputState& inputState) {
            m_tool->commitScale();

            // The mouse is in a different place now so update the highlighted side
            m_tool->updateDragFaces(inputState.pickResult());
        }

        void ScaleObjectsToolController::doCancelDrag() {
            m_tool->cancelScale();
        }

        void ScaleObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
        }
        
        void ScaleObjectsToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            // debug visualizations
#if 0
            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(Color(255, 255, 0, 1.0f));
                renderService.renderLine(m_handleLineDebug.point, m_handleLineDebug.point + (m_handleLineDebug.direction * 1024.0));
            }
            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(Color(255, 0, 0, 1.0f));
                renderService.renderHandle(m_lastDragDebug);
            }
            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(Color(0, 255, 0, 1.0f));
                renderService.renderHandle(m_currentDragDebug);
            }
            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(Color(0, 255, 0, 1.0f));
                renderService.renderBounds(m_tool->bboxAtDragStart());
            }
#endif

            // bounds and corner handles

            if (!m_tool->bounds().empty())  {
                // bounds
                {
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                    renderService.renderBounds(m_tool->bounds());
                }

                // corner handles
                for (const Vec3 &corner : m_tool->cornerHandles()) {
                    const auto ray = renderContext.camera().pickRay(corner);

                    if (renderContext.camera().perspectiveProjection()) {
                        Model::PickResult pr;
                        doPick(ray, renderContext.camera(), pr);

                        if (pr.query().first().type() != ScaleObjectsTool::ScaleToolCornerHit) {
                            // this corner is occluded => don't render it.
                            continue;
                        }
                    }

                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
                    renderService.renderHandle(corner);
                }
            }
            
            // highlighted stuff
            
            // highlight the polygons that will be dragged
            auto highlightedPolys = m_tool->polygonsHighlightedByDrag();
            for (const auto& poly : highlightedPolys) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setShowBackfaces();
                renderService.setForegroundColor(pref(Preferences::ScaleFillColor));
                renderService.renderFilledPolygon(poly.vertices());
            }
            
            if (m_tool->hasDragPolygon()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setLineWidth(2.0);
                renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
                renderService.renderPolygonOutline(m_tool->dragPolygon().vertices());
            }
            
            if (m_tool->hasDragEdge()) {
                const auto line = m_tool->dragEdge();
                const auto& camera = renderContext.camera();

                if (camera.orthographicProjection()
                    && line.direction().parallelTo(camera.direction())) {
                    // for the 2D view, for drag edges that are parallel to the camera,
                    // render the highlight with a ring around the handle
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                    renderService.renderHandleHighlight(line.start());
                } else {
                    // render as a thick line
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
                    renderService.setLineWidth(2.0);
                    renderService.renderLine(line.start(), line.end());
                }
            }
            
            if (m_tool->hasDragCorner()) {
                const auto corner = m_tool->dragCorner();

                // the filled circular handle
                {
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
                    renderService.renderHandle(corner);
                }

                // the ring around the handle
                {
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                    renderService.renderHandleHighlight(corner);
                }
            }
        }
        
        bool ScaleObjectsToolController::doCancel() {
            return false;
        }
        
        bool ScaleObjectsToolController::handleInput(const InputState& inputState) const {
            return m_tool->applies();
        }
        
        ScaleObjectsToolController2D::ScaleObjectsToolController2D(ScaleObjectsTool* tool, MapDocumentWPtr document) :
        ScaleObjectsToolController(tool, document) {}
        
        void ScaleObjectsToolController2D::doPick(const Ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick2D(pickRay, camera, pickResult);
        }
        
        ScaleObjectsToolController3D::ScaleObjectsToolController3D(ScaleObjectsTool* tool, MapDocumentWPtr document) :
        ScaleObjectsToolController(tool, document) {}
        
        void ScaleObjectsToolController3D::doPick(const Ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick3D(pickRay, camera, pickResult);
        }
    }
}

