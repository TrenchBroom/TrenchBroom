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
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        class EdgeToolController::SelectEdgePart : public SelectPartBase<Edge3> {
        public:
            SelectEdgePart(EdgeTool* tool) :
            SelectPartBase(tool, EdgeHandleManager::HandleHit) {}
        };
        
        class EdgeToolController::MoveEdgePart : public MovePartBase {
        public:
            MoveEdgePart(EdgeTool* tool) :
            MovePartBase(tool, EdgeHandleManager::HandleHit) {}
        private:
            const Model::Hit& findDragHandle(const InputState& inputState) const {
                return inputState.pickResult().query().type(EdgeHandleManager::HandleHit).occluded().first();
            }
        };
        
        EdgeToolController::EdgeToolController(EdgeTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveEdgePart(tool));
            addController(new SelectEdgePart(tool));
        }
    }
}
