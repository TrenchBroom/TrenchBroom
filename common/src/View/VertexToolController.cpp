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

#include "View/Grid.h"
#include "View/VertexTool.h"

namespace TrenchBroom {
    namespace View {
        /*
         * This is a bit awkward, but I'd rather not duplicate this logic into the two part classes, and I can't move
         * it up the inheritance hierarchy either. Nor can I introduce a separate common base class for the two parts
         * to contain this method due to the call to the inherited findDraggableHandle method.
         */
        const Model::Hit& VertexToolController::findHandleHit(const InputState& inputState, const VertexToolController::PartBase& base) {
            const Model::Hit& vertexHit = base.findDraggableHandle(inputState, VertexHandleManager::HandleHit);
            if (vertexHit.isMatch())
                return vertexHit;
            const Model::Hit& edgeHit = inputState.pickResult().query().type(EdgeHandleManager::HandleHit).first();
            if (edgeHit.isMatch())
                return edgeHit;
            return inputState.pickResult().query().type(FaceHandleManager::HandleHit).first();
        }

        class VertexToolController::SelectVertexPart : public SelectPartBase<Vec3> {
        public:
            SelectVertexPart(VertexTool* tool) :
            SelectPartBase(tool, VertexHandleManager::HandleHit) {}
        private:
            const Model::Hit& doFindDraggableHandle(const InputState& inputState) const override {
                return VertexToolController::findHandleHit(inputState, *this);
            }

            bool equalHandles(const Vec3& lhs, const Vec3& rhs) const override {
                return lhs.squaredDistanceTo(rhs) < MaxHandleDistance * MaxHandleDistance;
            }
        };

        class VertexToolController::MoveVertexPart : public MovePartBase {
        public:
            MoveVertexPart(VertexTool* tool) :
            MovePartBase(tool, VertexHandleManager::HandleHit) {}
        private:
            typedef enum {
                ST_Relative,
                ST_Absolute
            } SnapType;
            
            SnapType m_lastSnapType;
            Vec3 m_handleOffset;
        private:
            void doModifierKeyChange(const InputState& inputState) override {
                MoveToolController::doModifierKeyChange(inputState);
                
                if (Super::thisToolDragging()) {
                    const SnapType currentSnapType = snapType(inputState);
                    if (currentSnapType != m_lastSnapType) {
                        setSnapper(inputState, doCreateDragSnapper(inputState), false);
                        m_lastSnapType = currentSnapType;
                    }
                }
            }

            bool doMouseClick(const InputState& inputState) override {
                if (inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                    inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift) &&
                    m_tool->handleManager().selectedHandleCount() == 1) {
                    
                    const Model::Hit& hit = VertexToolController::findHandleHit(inputState, *this);
                    if (hit.hasType(VertexHandleManager::HandleHit)) {
                        const Vec3 sourcePos = m_tool->handleManager().selectedHandles().front();
                        const Vec3 targetPos = hit.target<Vec3>();
                        const Vec3 delta = targetPos - sourcePos;
                        m_tool->moveSelection(delta);
                        return true;
                    }
                }
                
                return false;
            }
            
            MoveInfo doStartMove(const InputState& inputState) override {
                const MoveInfo info = MovePartBase::doStartMove(inputState);
                if (info.move) {
                    m_lastSnapType = snapType(inputState);
                    const Model::Hit& hit = findDraggableHandle(inputState);
                    const Vec3& handlePos = hit.target<Vec3>();
                    m_handleOffset = handlePos - hit.hitPoint();
                }
                return info;
            }

            DragResult doMove(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) override {
                const auto result = MovePartBase::doMove(inputState, lastHandlePosition, nextHandlePosition);
                if (result == DR_Continue && m_tool->handleManager().contains(nextHandlePosition + m_handleOffset)) {
                    if ((snapType(inputState) == ST_Absolute && m_grid.snap(nextHandlePosition + m_handleOffset) != nextHandlePosition + m_handleOffset) ||
                        (snapType(inputState) == ST_Relative && m_grid.offset(lastHandlePosition + m_handleOffset) != m_grid.offset(nextHandlePosition + m_handleOffset))) {
                        restartDrag(inputState);
                    }
                }
                return result;
            }

            DragSnapper* doCreateDragSnapper(const InputState& inputState) const override {
                switch (snapType(inputState)) {
                    case ST_Absolute:
                        return new AbsoluteDragSnapper(m_tool->grid(), m_handleOffset);
                    case ST_Relative:
                        return new DeltaDragSnapper(m_tool->grid());
                        switchDefault();
                }
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                MovePartBase::doRender(inputState, renderContext, renderBatch);
                
                if (!thisToolDragging()) {
                    const Model::Hit& hit = findDraggableHandle(inputState);
                    if (hit.hasType(EdgeHandleManager::HandleHit | FaceHandleManager::HandleHit)) {
                        const Vec3 handle = m_tool->getHandlePosition(hit);
                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool->renderHandle(renderContext, renderBatch, handle, pref(Preferences::SelectedHandleColor));
                        else
                            m_tool->renderHandle(renderContext, renderBatch, handle);
                        m_tool->renderHighlight(renderContext, renderBatch, handle);
                    }
                }
            }
        private:
            SnapType snapType(const InputState& inputState) const {
                if (inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd))
                    return ST_Absolute;
                return ST_Relative;
            }
        private:
            const Model::Hit& doFindDraggableHandle(const InputState& inputState) const override {
                return VertexToolController::findHandleHit(inputState, *this);
            }
        };
        
        VertexToolController::VertexToolController(VertexTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveVertexPart(tool));
            addController(new SelectVertexPart(tool));
        }
    }
}
