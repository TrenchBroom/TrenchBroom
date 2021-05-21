/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/Camera.h"
#include "View/InputState.h"
#include "View/ScaleObjectsTool.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vecmath/segment.h>
#include <vecmath/polygon.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ScaleObjectsToolController::ScaleObjectsToolController(ScaleObjectsTool* tool, std::weak_ptr<MapDocument> document) :
        m_tool{tool},
        m_document{document} {
            ensure(m_tool != nullptr, "tool is null");
        }

        ScaleObjectsToolController::~ScaleObjectsToolController() = default;

        Tool* ScaleObjectsToolController::doGetTool() {
            return m_tool;
        }

        const Tool* ScaleObjectsToolController::doGetTool() const {
            return m_tool;
        }

        void ScaleObjectsToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (m_tool->applies()) {
                doPick(inputState.pickRay(), inputState.camera(), pickResult);
            }
        }

        static std::tuple<DragRestricter*, DragSnapper*, vm::vec3>
        getDragRestricterSnapperAndInitialPoint(const InputState& inputState,
                                                const Grid& grid,
                                                const Model::Hit& dragStartHit,
                                                const vm::bbox3& bboxAtDragStart) {
            const bool scaleAllAxes = inputState.modifierKeysDown(ModifierKeys::MKShift);

            DragRestricter* restricter = nullptr;
            DragSnapper* snapper = nullptr;

            if (dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHitType
                && inputState.camera().orthographicProjection()
                && !scaleAllAxes)
            {
                const auto plane = vm::plane3{dragStartHit.hitPoint(), vm::vec3{inputState.camera().direction()} * -1.0};

                restricter = new PlaneDragRestricter{plane};
                snapper = new DeltaDragSnapper{grid};
            } else {
                assert(dragStartHit.type() == ScaleObjectsTool::ScaleToolSideHitType
                       || dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHitType
                       || dragStartHit.type() == ScaleObjectsTool::ScaleToolCornerHitType);

                const vm::line3 handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);

                restricter = new LineDragRestricter{handleLine};
                snapper = new LineDragSnapper{grid, handleLine};
            }

            // Snap the initial point
            const vm::vec3 initialPoint = [&]() {
                vm::vec3 p = dragStartHit.hitPoint();
                restricter->hitPoint(inputState, p);
                snapper->snap(inputState, vm::vec3::zero(), vm::vec3::zero(), p);
                return p;
            }();

            return std::make_tuple(restricter, snapper, initialPoint);
        }

        static std::pair<AnchorPos, ProportionalAxes> modifierSettingsForInputState(const InputState& inputState) {
            const auto centerAnchor = inputState.modifierKeysDown(ModifierKeys::MKAlt) ? AnchorPos::Center : AnchorPos::Opposite;

            ProportionalAxes scaleAllAxes = ProportionalAxes::None();
            if (inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                scaleAllAxes = ProportionalAxes::All();

                const auto& camera = inputState.camera();
                if (camera.orthographicProjection()) {
                    // special case for 2D: don't scale along the axis of the camera
                    const size_t cameraComponent = vm::find_abs_max_component(camera.direction());
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
            if (m_tool->applies() && !anyToolDragging(inputState)) {
                m_tool->updatePickedHandle(inputState.pickResult());
            }
        }

        // RestrictedDragPolicy

        RestrictedDragPolicy::DragInfo ScaleObjectsToolController::doStartDrag(const InputState& inputState) {
            // based on CreateSimpleBrushToolController3D::doStartDrag
            using namespace Model::HitFilters;

            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return DragInfo();
            }
            if (!m_tool->applies()) {
                return DragInfo();
            }

            auto document = kdl::mem_lock(m_document);

            const Model::Hit& hit = inputState.pickResult().first(type(
                ScaleObjectsTool::ScaleToolSideHitType
                | ScaleObjectsTool::ScaleToolEdgeHitType
                | ScaleObjectsTool::ScaleToolCornerHitType));
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

        RestrictedDragPolicy::DragResult ScaleObjectsToolController::doDrag(const InputState&, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) {
            const auto delta = nextHandlePosition - lastHandlePosition;
            m_tool->scaleByDelta(delta);

            return DR_Continue;
        }

        void ScaleObjectsToolController::doEndDrag(const InputState& inputState) {
            m_tool->commitScale();

            // The mouse is in a different place now so update the highlighted side
            m_tool->updatePickedHandle(inputState.pickResult());
        }

        void ScaleObjectsToolController::doCancelDrag() {
            m_tool->cancelScale();
        }

        void ScaleObjectsToolController::doSetRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
        }

        void ScaleObjectsToolController::doRender(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            using namespace Model::HitFilters;

            const auto& camera = renderContext.camera();

            // bounds and corner handles

            if (!m_tool->bounds().is_empty())  {
                // bounds
                {
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                    renderService.renderBounds(vm::bbox3f(m_tool->bounds()));
                }

                // corner handles
                for (const auto& corner : m_tool->cornerHandles()) {
                    const auto ray = vm::ray3{renderContext.camera().pickRay(vm::vec3f(corner))};

                    if (renderContext.camera().perspectiveProjection()) {
                        auto pr = Model::PickResult{};
                        doPick(ray, renderContext.camera(), pr);

                        if (pr.empty() || pr.all().front().type() != ScaleObjectsTool::ScaleToolCornerHitType) {
                            // this corner is occluded => don't render it.
                            continue;
                        }
                    }

                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
                    renderService.renderHandle(vm::vec3f(corner));
                }
            }

            // Highlight all sides that will be moving as a result of the Shift/Alt modifiers
            // (proporitional scaling or center anchor modifiers)

            auto highlightedPolys = m_tool->polygonsHighlightedByDrag();
            for (const auto& poly : highlightedPolys) {
                {
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setShowBackfaces();
                    renderService.setForegroundColor(pref(Preferences::ScaleFillColor));
                    renderService.renderFilledPolygon(poly.vertices());
                }

                // In 2D, additionally stroke the edges of this polyhedron, so it's visible even when looking at it
                // from an edge
                if (camera.orthographicProjection()) {
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setLineWidth(2.0);
                    renderService.setForegroundColor(Color(pref(Preferences::ScaleOutlineColor), pref(Preferences::ScaleOutlineDimAlpha)));
                    renderService.renderPolygonOutline(poly.vertices());
                }
            }

            // draw the main highlighted handle

            if (m_tool->hasDragSide()) {
                auto renderService = Renderer::RenderService{renderContext, renderBatch};
                renderService.setLineWidth(2.0);
                renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
                renderService.renderPolygonOutline(m_tool->dragSide().vertices());
            }

            if (m_tool->hasDragEdge()) {
                const auto line = m_tool->dragEdge();

                if (camera.orthographicProjection()
                    && vm::is_parallel(line.direction(), camera.direction())) {
                    // for the 2D view, for drag edges that are parallel to the camera,
                    // render the highlight with a ring around the handle
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                    renderService.renderHandleHighlight(line.start());
                } else {
                    // render as a thick line
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
                    renderService.setLineWidth(2.0);
                    renderService.renderLine(line.start(), line.end());
                }
            }

            if (m_tool->hasDragCorner()) {
                const auto corner = m_tool->dragCorner();

                // the filled circular handle
                {
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
                    renderService.renderHandle(corner);
                }

                // the ring around the handle
                {
                    auto renderService = Renderer::RenderService{renderContext, renderBatch};
                    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                    renderService.renderHandleHighlight(corner);
                }
            }
        }

        bool ScaleObjectsToolController::doCancel() {
            return false;
        }

        // ScaleObjectsToolController2D

        ScaleObjectsToolController2D::ScaleObjectsToolController2D(ScaleObjectsTool* tool, std::weak_ptr<MapDocument> document) :
        ScaleObjectsToolController(tool, document) {}

        void ScaleObjectsToolController2D::doPick(const vm::ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick2D(pickRay, camera, pickResult);
        }

        // ScaleObjectsToolController3D

        ScaleObjectsToolController3D::ScaleObjectsToolController3D(ScaleObjectsTool* tool, std::weak_ptr<MapDocument> document) :
        ScaleObjectsToolController(tool, document) {}

        void ScaleObjectsToolController3D::doPick(const vm::ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick3D(pickRay, camera, pickResult);
        }
    }
}
