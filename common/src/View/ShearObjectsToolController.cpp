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

#include "ShearObjectsToolController.h"

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
#include "View/ShearObjectsTool.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ShearObjectsToolController::ShearObjectsToolController(ShearObjectsTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_dragStartHit(Model::Hit::NoHit),
        m_document(document) {
            ensure(m_tool != nullptr, "tool is null");
        }
        
        ShearObjectsToolController::~ShearObjectsToolController() = default;
        
        Tool* ShearObjectsToolController::doGetTool() {
            return m_tool;
        }

        void ShearObjectsToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            if (handleInput(inputState)) {
                doPick(inputState.pickRay(), inputState.camera(), pickResult);
            }
        }
        
        void ShearObjectsToolController::doModifierKeyChange(const InputState& inputState) {
        }
        
        void ShearObjectsToolController::doMouseMove(const InputState& inputState) {
            if (handleInput(inputState) && !anyToolDragging(inputState))
                m_tool->updateDragFaces(inputState.pickResult());
        }

        #if 0
        bool ShearObjectsToolController::updateResize(const InputState& inputState) {
            const bool vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);

            return m_tool->resize(inputState.pickRay(),
                                  inputState.camera(),
                                  vertical);
        }


        bool ShearObjectsToolController::doStartMouseDrag(const InputState& inputState) {
            if (!handleInput(inputState))
                return false;
            
            m_tool->updateDragFaces(inputState.pickResult());

            if (m_tool->beginResize(inputState.pickResult())) {
                m_tool->updateDragFaces(inputState.pickResult());
                return true;
            }
            return false;
        }
        
        bool ShearObjectsToolController::doMouseDrag(const InputState& inputState) {
            return updateResize(inputState);
        }

        void ShearObjectsToolController::doEndMouseDrag(const InputState& inputState) {
            m_tool->commitResize();
            m_tool->updateDragFaces(inputState.pickResult());
        }
        
        void ShearObjectsToolController::doCancelMouseDrag() {
            m_tool->cancelResize();
        }
#endif

        // RestrictedDragPolicy

        RestrictedDragPolicy::DragInfo ShearObjectsToolController::doStartDrag(const InputState& inputState) {
            // based on CreateSimpleBrushToolController3D::doStartDrag

            printf("ShearObjectsTool::doStartDrag\n");

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
                    ShearObjectsTool::ShearToolFaceHit).occluded().first();
            if (!hit.isMatch()) {
                return DragInfo();
            }

            m_bboxAtDragStart = m_tool->bounds();
            m_debugInitialPoint = hit.hitPoint();
            m_dragStartHit = hit;

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

        RestrictedDragPolicy::DragResult ShearObjectsToolController::doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {

            std::cout << "ShearObjectsTool::doDrag: last " << lastHandlePosition << " next " << nextHandlePosition << "\n";

            m_lastDragDebug = lastHandlePosition;
            m_currentDragDebug = nextHandlePosition;

            const auto delta = nextHandlePosition - lastHandlePosition;

            m_dragCumulativeDelta += delta;

            std::cout << "total: " << m_dragCumulativeDelta << " ( added " << delta << ")\n";


            MapDocumentSPtr document = lock(m_document);

            const auto& hit = m_dragStartHit;

//
//            std::cout << "resize to " << newBox << "\n";
//
//            document->scaleObjects(m_tool->bounds(), newBox);

            return DR_Continue;
        }

        void ShearObjectsToolController::doEndDrag(const InputState& inputState) {
            printf("ShearObjectsTool::doEndDrag\n");

//            m_tool->commitResize();
//            m_tool->updateDragFaces(inputState.pickResult());
        }

        void ShearObjectsToolController::doCancelDrag() {
            printf("ShearObjectsTool::doCancelDrag\n");

            //m_tool->cancelResize();
        }


        void ShearObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
            // TODO: force rendering of all other map views if the input applies and the tool has drag faces
        }
        
        void ShearObjectsToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            // render sheared box
            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
                const auto mat = m_tool->bboxShearMatrix();
                const auto op = [&](const Vec3 &start, const Vec3 &end) {
                    renderService.renderLine(mat * start, mat * end);
                };
                eachBBoxEdge(m_tool->bboxAtDragStart(), op);
            }

            // render shear handle

            {
                Renderer::RenderService renderService(renderContext, renderBatch);
                const Polygon3f poly = m_tool->shearHandle();
                if (poly.vertexCount() != 0) {
                    renderService.setLineWidth(2.0);
                    renderService.setForegroundColor(pref(Preferences::ShearOutlineColor));
                    renderService.renderPolygonOutline(poly.vertices());
                }
            }
        }
        
        bool ShearObjectsToolController::doCancel() {
            return false;
        }
        
        bool ShearObjectsToolController::handleInput(const InputState& inputState) const {
            return m_tool->applies();
        }
        
        ShearObjectsToolController2D::ShearObjectsToolController2D(ShearObjectsTool* tool, MapDocumentWPtr document) :
        ShearObjectsToolController(tool, document) {}
        
        void ShearObjectsToolController2D::doPick(const Ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick2D(pickRay, camera, pickResult);
        }
        
        ShearObjectsToolController3D::ShearObjectsToolController3D(ShearObjectsTool* tool, MapDocumentWPtr document) :
        ShearObjectsToolController(tool, document) {}
        
        void ShearObjectsToolController3D::doPick(const Ray3 &pickRay, const Renderer::Camera &camera,
                                                  Model::PickResult &pickResult) {
            m_tool->pick3D(pickRay, camera, pickResult);
        }
    }
}

