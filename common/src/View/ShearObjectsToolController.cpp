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

#include "ShearObjectsToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Reference.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/Camera.h"
#include "View/InputState.h"
#include "View/ShearObjectsTool.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ShearObjectsToolController::ShearObjectsToolController(ShearObjectsTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            ensure(m_tool != nullptr, "tool is null");
        }
        
        ShearObjectsToolController::~ShearObjectsToolController() = default;
        
        Tool* ShearObjectsToolController::doGetTool() {
            return m_tool;
        }

        void ShearObjectsToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (m_tool->applies()) {
                // forward to either ShearObjectsTool::pick2D or ShearObjectsTool::pick3D
                doPick(inputState.pickRay(), inputState.camera(), pickResult);
            }
        }

        static std::tuple<DragRestricter*, DragSnapper*>
        getDragRestricterAndSnapper(const BBoxSide& side, const BBox3& bboxAtDragStart, const Renderer::Camera& camera, const Grid& grid, const bool vertical) {
            DragRestricter* restricter = nullptr;
            DragSnapper* snapper = nullptr;

            const vec3 sideCenter = centerForBBoxSide(bboxAtDragStart, side);

            if (camera.perspectiveProjection()) {
                if (side.normal == vec3::pos_z || side.normal == vec3::neg_z) {
                    restricter = new PlaneDragRestricter(Plane3(sideCenter, side.normal));
                    snapper = new DeltaDragSnapper(grid);
                } else if (!vertical) {
                    const Line3 sideways(sideCenter, normalize(cross(side.normal, vec3::pos_z)));

                    restricter = new LineDragRestricter(sideways);
                    snapper = new LineDragSnapper(grid, sideways);
                } else {
                    const Line3 verticalLine(sideCenter, vec3::pos_z);

                    restricter = new LineDragRestricter(verticalLine);
                    snapper = new LineDragSnapper(grid, verticalLine);
                }
            } else {
                assert(camera.orthographicProjection());

                const Line3 sideways(sideCenter, normalize(cross(side.normal, vec3(camera.direction()))));
                restricter = new LineDragRestricter(sideways);
                snapper = new LineDragSnapper(grid, sideways);
            }

            return {restricter, snapper};
        }

        void ShearObjectsToolController::doModifierKeyChange(const InputState& inputState) {
            // Modifiers are only used for the perspective camera
            if (!inputState.camera().perspectiveProjection()) {
                return;
            }

            const bool vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);

            if (!thisToolDragging()) {
                return;
            }

            const BBoxSide side = m_tool->dragStartHit().target<BBoxSide>();

            // Can't do vertical restraint on these
            if (side.normal == vec3::pos_z || side.normal == vec3::neg_z) {
                return;
            }

            MapDocumentSPtr document = lock(m_document);

            if (vertical != m_tool->constrainVertical()) {
                m_tool->setConstrainVertical(vertical);

                auto [restricter, snapper] = getDragRestricterAndSnapper(side, m_tool->bboxAtDragStart(), inputState.camera(), document->grid(), vertical);

                setRestricter(inputState, restricter, true);
                setSnapper(inputState, snapper, true);
            }

            // Mouse might be over a different handle now
            m_tool->refreshViews();
        }
        
        void ShearObjectsToolController::doMouseMove(const InputState& inputState) {
            if (m_tool->applies() && !anyToolDragging(inputState)) {
                m_tool->updatePickedSide(inputState.pickResult());
            }
        }

        // RestrictedDragPolicy

        RestrictedDragPolicy::DragInfo ShearObjectsToolController::doStartDrag(const InputState& inputState) {
            // based on CreateSimpleBrushToolController3D::doStartDrag

            const bool vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);

            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                return DragInfo();
            }
            if (!(inputState.modifierKeysPressed(ModifierKeys::MKNone) || vertical)) {
                return DragInfo();
            }
            if (!m_tool->applies()) {
                return DragInfo();
            }

            MapDocumentSPtr document = lock(m_document);

            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().type(ShearObjectsTool::ShearToolSideHit).occluded().first();
            if (!hit.isMatch()) {
                return DragInfo();
            }

            m_tool->startShearWithHit(hit);
            m_tool->setConstrainVertical(vertical);

            const BBoxSide side = m_tool->dragStartHit().target<BBoxSide>();

            DragRestricter* restricter;
            DragSnapper* snapper;

            std::tie(restricter, snapper) = getDragRestricterAndSnapper(side, m_tool->bboxAtDragStart(), inputState.camera(), document->grid(), vertical);

            // Snap the initial point
            const vec3 initialPoint = [&]() {
                vec3 p = hit.hitPoint();
                restricter->hitPoint(inputState, p);
                snapper->snap(inputState, vec3::zero, vec3::zero, p);
                return p;
            }();

            return DragInfo(restricter, snapper, initialPoint);
        }

        RestrictedDragPolicy::DragResult ShearObjectsToolController::doDrag(const InputState& inputState, const vec3& lastHandlePosition, const vec3& nextHandlePosition) {
            const auto delta = nextHandlePosition - lastHandlePosition;
            m_tool->shearByDelta(delta);

            return DR_Continue;
        }

        void ShearObjectsToolController::doEndDrag(const InputState& inputState) {
            m_tool->commitShear();

            // The mouse is in a different place now so update the highlighted side
            m_tool->updatePickedSide(inputState.pickResult());
        }

        void ShearObjectsToolController::doCancelDrag() {
            m_tool->cancelShear();
        }


        void ShearObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
        }
        
        void ShearObjectsToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            // render sheared box
            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                const auto mat = m_tool->bboxShearMatrix();
                const auto op = [&](const vec3& start, const vec3& end) {
                    renderService.renderLine(vec3f(mat * start), vec3f(mat * end));
                };
                eachBBoxEdge(m_tool->bboxAtDragStart(), op);
            }

            // render shear handle
            {
                const Polygon3f poly = m_tool->shearHandle();
                if (poly.vertexCount() != 0) {
                    // fill
                    {
                        Renderer::RenderService renderService(renderContext, renderBatch);
                        renderService.setShowBackfaces();
                        renderService.setForegroundColor(pref(Preferences::ShearFillColor));
                        renderService.renderFilledPolygon(poly.vertices());
                    }

                    // outline
                    {
                        Renderer::RenderService renderService(renderContext, renderBatch);
                        renderService.setLineWidth(2.0);
                        renderService.setForegroundColor(pref(Preferences::ShearOutlineColor));
                        renderService.renderPolygonOutline(poly.vertices());
                    }
                }
            }
        }
        
        bool ShearObjectsToolController::doCancel() {
            return false;
        }

        // ShearObjectsToolController2D

        ShearObjectsToolController2D::ShearObjectsToolController2D(ShearObjectsTool* tool, MapDocumentWPtr document) :
        ShearObjectsToolController(tool, document) {}
        
        void ShearObjectsToolController2D::doPick(const Ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick2D(pickRay, camera, pickResult);
        }

        // ShearObjectsToolController3D

        ShearObjectsToolController3D::ShearObjectsToolController3D(ShearObjectsTool* tool, MapDocumentWPtr document) :
        ShearObjectsToolController(tool, document) {}
        
        void ShearObjectsToolController3D::doPick(const Ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick3D(pickRay, camera, pickResult);
        }
    }
}

