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

namespace TrenchBroom {
    namespace View {
        class EdgeToolController::SelectEdgePart : public SelectPartBase {
        public:
            SelectEdgePart(EdgeTool* tool) :
            SelectPartBase(tool) {}
        private:
            Model::Hit::List firstHits(const Model::PickResult& pickResult) const {
                // TODO: implement me
                return Model::Hit::List();
            }
        };
        
        class EdgeToolController::MoveEdgePart : public MovePartBase {
        public:
            MoveEdgePart(EdgeTool* tool) :
            MovePartBase(tool) {}
        private:
            MoveInfo doStartMove(const InputState& inputState) {
                if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                      (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKCtrlCmd) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                       inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift)
                       )))
                    return MoveInfo();
                return MoveInfo();
            }
            
            DragResult doMove(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                return DR_Continue;
            }
            
            void doEndMove(const InputState& inputState) {
            }
            
            void doCancelMove() {
            }
            
            DragSnapper* doCreateDragSnapper(const InputState& inputState) const {
                return new DeltaDragSnapper(m_tool->grid());
            }
        };
        
        EdgeToolController::EdgeToolController(EdgeTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveEdgePart(tool));
            addController(new SelectEdgePart(tool));
        }
    }
}
