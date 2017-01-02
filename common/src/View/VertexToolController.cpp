/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "VertexToolController.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/Lasso.h"
#include "View/MapDocument.h"
#include "View/VertexTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexToolController::MaxVertexDistance = 0.25;

        class VertexToolController::VertexPartBase {
        protected:
            VertexTool* m_tool;
        public:
            VertexPartBase(VertexTool* tool) :
            m_tool(tool) {
                ensure(m_tool != NULL, "tool is null");
            }
        protected:
            Model::Hit::List firstHits(const Model::PickResult& pickResult) const {
                Model::Hit::List result;
                Model::BrushSet brushes;
                
                static const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
                const Model::Hit& first = pickResult.query().type(any).occluded().first();
                if (first.isMatch()) {
                    const Vec3 firstHitPosition = first.target<Vec3>();
                    
                    const Model::Hit::List matches = pickResult.query().type(any).all();
                    for (const Model::Hit& hit : matches) {
                        const Vec3 hitPosition = hit.target<Vec3>();
                        
                        if (hitPosition.distanceTo(firstHitPosition) < MaxVertexDistance) {
                            const bool newBrush = m_tool->handleBrushes(hitPosition, brushes);
                            if (newBrush)
                                result.push_back(hit);
                        }
                    }
                }
                
                return result;
            }
        };
        
        class VertexToolController::SelectVertexPart : public ToolControllerBase<PickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, public VertexPartBase {
        private:
            Lasso* m_lasso;
        public:
            SelectVertexPart(VertexTool* tool) :
            VertexPartBase(tool),
            m_lasso(NULL) {}
            
            ~SelectVertexPart() {
                delete m_lasso;
            }
        private:
            Tool* doGetTool() {
                return m_tool;
            }

            void doPick(const InputState& inputState, Model::PickResult& pickResult) {
                m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
            }
            
            bool doMouseClick(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No))
                    return false;
                
                const Model::Hit::List hits = firstHits(inputState.pickResult());
                if (hits.empty())
                    return m_tool->deselectAll();
                else
                    return m_tool->select(hits, inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd));
            }
            
            DragInfo doStartDrag(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No))
                    return DragInfo();
                
                const Model::Hit::List hits = firstHits(inputState.pickResult());
                if (!hits.empty())
                    return DragInfo();
                
                const Renderer::Camera& camera = inputState.camera();
                const FloatType distance = 64.0f;
                const Plane3 plane = orthogonalDragPlane(camera.defaultPoint(distance), camera.direction());
                const Vec3 initialPoint = inputState.pickRay().pointAtDistance(plane.intersectWithRay(inputState.pickRay()));
                
                m_lasso = new Lasso(camera, distance, initialPoint);
                return DragInfo(new PlaneDragRestricter(plane), new NoDragSnapper(), initialPoint);
            }
            
            DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                ensure(m_lasso != NULL, "lasso is null");
                m_lasso->setPoint(curPoint);
                return DR_Continue;
            }
            
            void doEndDrag(const InputState& inputState) {
                ensure(m_lasso != NULL, "lasso is null");
                m_tool->select(*m_lasso, inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
                delete m_lasso;
                m_lasso = NULL;
            }
            
            void doCancelDrag() {
                ensure(m_lasso != NULL, "lasso is null");
                delete m_lasso;
                m_lasso = NULL;
            }
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                renderContext.setForceHideSelectionGuide();
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_tool->renderHandles(renderContext, renderBatch);
                if (m_lasso != NULL)
                    m_lasso->render(renderContext, renderBatch);
            }

            bool doCancel() {
                return m_tool->cancel();
            }
        };
        
        class VertexToolController::MoveVertexPart : public MoveToolController<NoPickingPolicy, MousePolicy>, public VertexPartBase {
        public:
            MoveVertexPart(VertexTool* tool) :
            MoveToolController(tool->grid()),
            VertexPartBase(tool) {}
        private:
            typedef enum {
                ST_Relative,
                ST_Absolute
            } SnapType;
            
            SnapType m_lastSnapType;
        private:
            Tool* doGetTool() {
                return m_tool;
            }

            void doModifierKeyChange(const InputState& inputState) {
                MoveToolController::doModifierKeyChange(inputState);
                
                if (Super::thisToolDragging()) {
                    const SnapType currentSnapType = snapType(inputState);
                    if (currentSnapType != m_lastSnapType) {
                        setSnapper(inputState, doCreateDragSnapper(inputState));
                        m_lastSnapType = currentSnapType;
                    }
                }
            }
            
            bool doMouseDoubleClick(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.checkModifierKeys(MK_No, MK_No, MK_No))
                    return false;
                
                const Model::Hit::List hits = firstHits(inputState.pickResult());
                if (hits.empty())
                    return false;
                
                const Model::Hit& hit = hits.front();
                return m_tool->handleDoubleClicked(hit);
            }

            MoveInfo doStartMove(const InputState& inputState) {
                if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                      (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd))))
                    return MoveInfo();
                
                static const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
                const Model::Hit& hit = inputState.pickResult().query().type(any).occluded().first();
                if (!hit.isMatch())
                    return MoveInfo();
                
                if (!m_tool->beginMove(hit))
                    return MoveInfo();
                
                m_lastSnapType = snapType(inputState);
                return MoveInfo(hit.target<Vec3>());
            }
            
            DragResult doMove(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                switch (m_tool->move(curPoint - lastPoint)) {
                    case VertexTool::MR_Continue:
                        return DR_Continue;
                    case VertexTool::MR_Deny:
                        return DR_Deny;
                    case VertexTool::MR_Cancel:
                        return DR_Cancel;
                    switchDefault()
                }
            }
            
            void doEndMove(const InputState& inputState) {
                m_tool->endMove();
            }
            
            void doCancelMove() {
                m_tool->cancelMove();
            }
            
            DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                switch (snapType(inputState)) {
                    case ST_Absolute:
                        return new AbsoluteDragSnapper(m_tool->grid());
                    case ST_Relative:
                        return new DeltaDragSnapper(m_tool->grid());
                    switchDefault();
                }
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                MoveToolController::doRender(inputState, renderContext, renderBatch);
                
                if (thisToolDragging()) {
                    m_tool->renderHighlight(renderContext, renderBatch);
                    m_tool->renderGuide(renderContext, renderBatch);
                } else if (!anyToolDragging(inputState)) {
                    static const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
                    const Model::Hit& hit = inputState.pickResult().query().type(any).occluded().first();
                    if (hit.isMatch()) {
                        const Vec3 position = hit.target<Vec3>();
                        m_tool->renderHighlight(renderContext, renderBatch, position);
                        if (!m_tool->handleSelected(position)) {
                            if (hit.type() == VertexHandleManager::EdgeHandleHit)
                                m_tool->renderEdgeHighlight(renderContext, renderBatch, position);
                            else if (hit.type() == VertexHandleManager::FaceHandleHit)
                                m_tool->renderFaceHighlight(renderContext, renderBatch, position);
                        }
                        
                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool->renderGuide(renderContext, renderBatch, position);
                    }
                }
            }

            bool doCancel() {
                return m_tool->cancel();
            }
        private:
            SnapType snapType(const InputState& inputState) const {
                if (inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd))
                    return ST_Absolute;
                return ST_Relative;
            }
        };
        
        class VertexToolController::SnapVertexPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, NoMouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public VertexPartBase {
        public:
            SnapVertexPart(VertexTool* tool) :
            VertexPartBase(tool) {}
        private:
            Tool* doGetTool() {
                return m_tool;
            }

            bool doMouseClick(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.checkModifierKeys(MK_No, MK_Yes, MK_Yes))
                    return false;
                
                const Model::Hit& hit = inputState.pickResult().query().type(VertexHandleManager::VertexHandleHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                
                m_tool->mergeVertices(hit);
                return true;
            }
            
            bool doCancel() {
                return m_tool->cancel();
            }
        };
        
        VertexToolController::VertexToolController(VertexTool* tool) :
        m_tool(tool) {
            ensure(m_tool != NULL, "tool is null");
            addController(new MoveVertexPart(tool));
            addController(new SelectVertexPart(tool));
            addController(new SnapVertexPart(tool));
        }

        VertexToolController::~VertexToolController() {}

        Tool* VertexToolController::doGetTool() {
            return m_tool;
        }
    }
}
