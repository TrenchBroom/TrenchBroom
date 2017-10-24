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

#include "EdgeToolController.h"

#include "View/EdgeTool.h"
#include "View/Lasso.h"
#include "View/MoveToolController.h"

namespace TrenchBroom {
    namespace View {
        class EdgeToolController::EdgePartBase : public PartBase {
        protected:
            EdgePartBase(EdgeTool* tool) :
            PartBase(tool) {}
        };
        
        class EdgeToolController::SelectEdgePart : public ToolControllerBase<PickingPolicy, NoKeyPolicy, MousePolicy, NoMouseDragPolicy, RenderPolicy, NoDropPolicy>, public EdgePartBase {
        public:
            SelectEdgePart(EdgeTool* tool) :
            EdgePartBase(tool) {}
        private:
            Tool* doGetTool() {
                return m_tool;
            }
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult) {
                m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
            }
            
            bool doMouseClick(const InputState& inputState) {
                return false;
            }
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}
            
            bool doCancel() {
                return false;
            }
        };
        
        class EdgeToolController::MoveEdgePart : public MoveToolController<NoPickingPolicy, NoMousePolicy>, public EdgePartBase {
        public:
            MoveEdgePart(EdgeTool* tool) :
            MoveToolController(tool->grid()),
            EdgePartBase(tool) {}
        private:
            Tool* doGetTool() {
                return m_tool;
            }
            
            MoveInfo doStartMove(const InputState& inputState) {
                
            }
            
            DragResult doMove(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                
            }
            
            void doEndMove(const InputState& inputState) {
                
            }
            
            void doCancelMove(const InputState& inputState) {
                
            }
            
            DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                
            }
            
            bool doCancel() {
                return false;
            }
        };
        
        EdgeToolController::EdgeToolController(EdgeTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
            addController(new MoveEdgePart(tool));
            addController(new SelectEdgePart(tool));
        }
        
        Tool* EdgeToolController::doGetTool() {
            return m_tool;
        }
    }
}
