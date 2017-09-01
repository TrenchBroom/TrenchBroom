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

#include "VertexToolController.h"

#include "Renderer/RenderContext.h"
#include "View/Lasso.h"
#include "View/MoveToolController.h"
#include "View/VertexTool.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexToolController::MaxVertexDistance = 0.25;
        
        class VertexToolController::VertexPartBase {
        protected:
            VertexTool* m_tool;
        public:
            VertexPartBase(VertexTool* tool) :
            m_tool(tool) {
                ensure(m_tool != nullptr, "tool is null");
            }
        protected:
            Model::Hit::List firstHits(const Model::PickResult& pickResult) const {
                Model::Hit::List result;
                Model::BrushSet visitedBrushes;
                
                static const Model::Hit::HitType any = VertexTool::AnyHandleHit;
                const Model::Hit& first = pickResult.query().type(any).occluded().first();
                if (first.isMatch()) {
                    const Vec3 firstHandle = first.target<Vec3>();
                    
                    const Model::Hit::List matches = pickResult.query().type(any).all();
                    for (const Model::Hit& match : matches) {
                        const Vec3 handle = match.target<Vec3>();
                        
                        if (handle.squaredDistanceTo(firstHandle) < MaxVertexDistance * MaxVertexDistance) {
                            if (allIncidentBrushesVisited(handle, visitedBrushes))
                                result.push_back(match);
                        }
                    }
                }
                
                return result;
            }
        private:
            bool allIncidentBrushesVisited(const Vec3& handle, Model::BrushSet& visitedBrushes) const {
                bool result = true;
                for (auto brush : m_tool->findIncidentBrushes(handle)) {
                    const bool unvisited = visitedBrushes.insert(brush).second;
                    result &= unvisited;
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
            m_lasso(nullptr) {}
            
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
            
            DragResult doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                ensure(m_lasso != nullptr, "lasso is null");
                m_lasso->setPoint(nextHandlePosition);
                return DR_Continue;
            }
            
            void doEndDrag(const InputState& inputState) {
                ensure(m_lasso != nullptr, "lasso is null");
                m_tool->select(*m_lasso, inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
                delete m_lasso;
                m_lasso = nullptr;
            }
            
            void doCancelDrag() {
                ensure(m_lasso != nullptr, "lasso is null");
                delete m_lasso;
                m_lasso = nullptr;
            }
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                renderContext.setForceHideSelectionGuide();
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_tool->renderHandles(renderContext, renderBatch);
                if (m_lasso != nullptr)
                    m_lasso->render(renderContext, renderBatch);
                if (!anyToolDragging(inputState)) {
                    const Model::Hit& hit = inputState.pickResult().query().type(VertexTool::AnyHandleHit).occluded().first();
                    if (hit.isMatch()) {
                        const Vec3& handle = hit.target<Vec3>();
                        m_tool->renderHighlight(renderContext, renderBatch, handle);

                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool->renderGuide(renderContext, renderBatch, handle);
                    }
                }
            }

            bool doCancel() {
                return m_tool->deselectAll();
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
                        setSnapper(inputState, doCreateDragSnapper(inputState), false);
                        m_lastSnapType = currentSnapType;
                    }
                }
            }

            MoveInfo doStartMove(const InputState& inputState) {
                if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                      (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd))))
                    return MoveInfo();
                
                const Model::Hit& hit = inputState.pickResult().query().type(VertexTool::AnyHandleHit).occluded().first();
                if (!hit.isMatch())
                    return MoveInfo();
                
                if (!m_tool->startMove(hit))
                    return MoveInfo();
                
                m_lastSnapType = snapType(inputState);
                return MoveInfo(hit.target<Vec3>());
            }

            DragResult doMove(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                switch (m_tool->move(nextHandlePosition - lastHandlePosition)) {
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
                    m_tool->renderDragHighlight(renderContext, renderBatch);
                    m_tool->renderDragGuide(renderContext, renderBatch);
                }
            }
            
            bool doCancel() {
                return m_tool->deselectAll();
            }
        private:
            SnapType snapType(const InputState& inputState) const {
                if (inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd))
                    return ST_Absolute;
                return ST_Relative;
            }
        };
        
        VertexToolController::VertexToolController(VertexTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
            addController(new MoveVertexPart(tool));
            addController(new SelectVertexPart(tool));
        }

        
        Tool* VertexToolController::doGetTool() {
            return m_tool;
        }
    }
}
