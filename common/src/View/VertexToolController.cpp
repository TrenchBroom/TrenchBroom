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
#include "View/VertexTool.h"
#include "View/VertexHandleManager.h"

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        const Model::Hit& findHandleHit(const InputState& inputState) {
            const Model::Hit& vertexHit = inputState.pickResult().query().type(VertexHandleManager::HandleHit).occluded().first();
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
            SelectPartBase(tool, VertexHandleManager::HandleHit | EdgeHandleManager::HandleHit | FaceHandleManager::HandleHit) {}
        private:
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                SelectPartBase::doRender(inputState, renderContext, renderBatch);
                
                if (!anyToolDragging(inputState)) {
                    const Model::Hit& hit = findHandleHit(inputState);
                    if (hit.hasType(VertexHandleManager::HandleHit)) {
                        const Vec3& handle = hit.target<Vec3>();
                        m_tool->renderHighlight(renderContext, renderBatch, handle);

                        if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                            m_tool->renderGuide(renderContext, renderBatch, handle);
                    }
                }
            }
        };

        class VertexToolController::MoveVertexPart : public MovePartBase {
        public:
            MoveVertexPart(VertexTool* tool) :
            MovePartBase(tool) {}
        private:
            typedef enum {
                ST_Relative,
                ST_Absolute
            } SnapType;
            
            SnapType m_lastSnapType;
        private:
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
                m_lastSnapType = snapType(inputState);
                return MovePartBase::doStartMove(inputState);
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
                MovePartBase::doRender(inputState, renderContext, renderBatch);
                
                if (!thisToolDragging()) {
                    const Model::Hit& hit = findHandleHit(inputState);
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
            const Model::Hit& findDragHandle(const InputState& inputState) const {
                return findHandleHit(inputState);
            }
        };
        
        VertexToolController::VertexToolController(VertexTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveVertexPart(tool));
            addController(new SelectVertexPart(tool));
        }
    }
}
