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
        m_dragStartHit(Model::Hit::NoHit),
        m_document(document) {
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
        
        void ScaleObjectsToolController::doModifierKeyChange(const InputState& inputState) {

            const bool shear = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
            const bool centerAnchor = inputState.modifierKeysDown(ModifierKeys::MKAlt);
            const bool scaleAllAxes = inputState.modifierKeysDown(ModifierKeys::MKShift);

            m_tool->setAnchorPos(centerAnchor  ? AnchorPos::Center : AnchorPos::Opposite);
            m_tool->setScaleAllAxes(scaleAllAxes);

            // Modifiers that can be enabled/disabled any time:
            // - proportional (shift)
            // - vertical (alt)
            
            if (thisToolDragging()) {
                // FIXME: should manually "update drag" using RestrictedDragPolicy.currentHandlePosition()
                // regresh view

                //updateResize(inputState);
            }
//            else {
//                m_tool->setShearing(shear);
//            }

            m_tool->refreshViews();
        }
        
        void ScaleObjectsToolController::doMouseMove(const InputState& inputState) {
            if (handleInput(inputState) && !anyToolDragging(inputState))
                m_tool->updateDragFaces(inputState.pickResult());
        }

        #if 0
        bool ScaleObjectsToolController::updateResize(const InputState& inputState) {
            const bool vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);

            return m_tool->resize(inputState.pickRay(),
                                  inputState.camera(),
                                  vertical);
        }


        bool ScaleObjectsToolController::doStartMouseDrag(const InputState& inputState) {
            if (!handleInput(inputState))
                return false;
            
            m_tool->updateDragFaces(inputState.pickResult());

            if (m_tool->beginResize(inputState.pickResult())) {
                m_tool->updateDragFaces(inputState.pickResult());
                return true;
            }
            return false;
        }
        
        bool ScaleObjectsToolController::doMouseDrag(const InputState& inputState) {
            return updateResize(inputState);
        }

        void ScaleObjectsToolController::doEndMouseDrag(const InputState& inputState) {
            m_tool->commitResize();
            m_tool->updateDragFaces(inputState.pickResult());
        }
        
        void ScaleObjectsToolController::doCancelMouseDrag() {
            m_tool->cancelResize();
        }
#endif

        // RestrictedDragPolicy

        RestrictedDragPolicy::DragInfo ScaleObjectsToolController::doStartDrag(const InputState& inputState) {
            // based on CreateSimpleBrushToolController3D::doStartDrag

            printf("ScaleObjectsTool::doStartDrag\n");

            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return DragInfo();
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return DragInfo();

//            if (!m_tool->applies()) {
//                return DragInfo();
//            }
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelection()) {
                return DragInfo();
            }

            const Model::PickResult& pickResult = inputState.pickResult();

            // TODO: why did .pickable() break it?
            const Model::Hit& hit = pickResult.query().type(
                    ScaleObjectsTool::ScaleToolFaceHit
                    | ScaleObjectsTool::ScaleToolEdgeHit
                    | ScaleObjectsTool::ScaleToolCornerHit).occluded().first();
            if (!hit.isMatch()) {
                return DragInfo();
            }

            m_bboxAtDragStart = m_tool->bounds();
            m_debugInitialPoint = hit.hitPoint();
            m_dragStartHit = hit;

            if (hit.type() == ScaleObjectsTool::ScaleToolEdgeHit
                && inputState.camera().orthographicProjection()) {
                std::cout << "ortho corner hit\n";

                m_handleLineDebug = Line3();
                m_dragCumulativeDelta = Vec3::Null;

                const Plane3 plane(hit.hitPoint(), inputState.camera().direction() * -1.0);

                auto restricter = new PlaneDragRestricter(plane);
                auto snapper = new DeltaDragSnapper(document->grid());

                // FIXME: rework hack
                m_isOrthoFree = true;
                return DragInfo(restricter, snapper, hit.hitPoint());
            }  else {
                // FIXME: rework hack
                m_isOrthoFree = false;
            }

            const Line3 handleLine = handleLineForHit(m_bboxAtDragStart, hit);

            m_handleLineDebug = handleLine;
            m_dragCumulativeDelta = Vec3::Null;

            auto restricter = new LineDragRestricter(handleLine);
            auto snapper = new LineDragSnapper(document->grid(), handleLine);

            // HACK: Snap the initial point
            const Vec3 initialPoint = [&]() {
                Vec3 p = hit.hitPoint();
                restricter->hitPoint(inputState, p);
                snapper->snap(inputState, Vec3::Null, Vec3::Null, p);
                return p;
            }();

            return DragInfo(restricter, snapper, initialPoint);
        }

        RestrictedDragPolicy::DragResult ScaleObjectsToolController::doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {

            std::cout << "ScaleObjectsTool::doDrag: last " << lastHandlePosition << " next " << nextHandlePosition << "\n";

            m_lastDragDebug = lastHandlePosition;
            m_currentDragDebug = nextHandlePosition;

            const auto delta = nextHandlePosition - lastHandlePosition;

            m_dragCumulativeDelta += delta;

            std::cout << "total: " << m_dragCumulativeDelta << " ( added " << delta << ")\n";


            MapDocumentSPtr document = lock(m_document);

//            if (m_isOrthoFree) {
//
//
//
//            } else {
                //const auto& hit = m_dragStartHit;
                const auto newBox = moveBBoxForHit(m_bboxAtDragStart, m_dragStartHit, m_dragCumulativeDelta,
                                                   m_tool->scaleAllAxes(), m_tool->anchorPos());

                std::cout << "resize to " << newBox << "\n";

                if (!newBox.empty()) {
                    document->scaleObjects(m_tool->bounds(), newBox);
                }
//            }

            return DR_Continue;
        }

        void ScaleObjectsToolController::doEndDrag(const InputState& inputState) {
            printf("ScaleObjectsTool::doEndDrag\n");

//            m_tool->commitResize();
//            m_tool->updateDragFaces(inputState.pickResult());
        }

        void ScaleObjectsToolController::doCancelDrag() {
            printf("ScaleObjectsTool::doCancelDrag\n");

            //m_tool->cancelResize();
        }


        void ScaleObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
            // TODO: force rendering of all other map views if the input applies and the tool has drag faces
        }
        
        void ScaleObjectsToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderScale(inputState, renderContext, renderBatch);
        }

        void ScaleObjectsToolController::renderScale(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            // debug

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

#if 0
            // draw anchor crosshair
            if (m_tool->hasDragAnchor()) {
                const float scale = renderContext.camera().perspectiveScalingFactor(m_tool->dragAnchor());
                const float radius = 32.0f * scale;

                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setShowOccludedObjects();

                renderService.renderCoordinateSystem(BBox3f(radius).translated(m_tool->dragAnchor()));
            }
#endif
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

