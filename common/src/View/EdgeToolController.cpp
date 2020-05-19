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

#include <vecmath/polygon.h>

namespace TrenchBroom {
    namespace View {
        class EdgeToolController::SelectEdgePart : public SelectPartBase<vm::segment3> {
        public:
            SelectEdgePart(EdgeTool* tool) :
            SelectPartBase(tool, EdgeHandleManager::HandleHitType) {}
        private:
            bool equalHandles(const vm::segment3& lhs, const vm::segment3& rhs) const override {
                return compare(lhs, rhs, MaxHandleDistance) == 0;
            }
        };

        class EdgeToolController::MoveEdgePart : public MovePartBase {
        public:
            MoveEdgePart(EdgeTool* tool) :
            MovePartBase(tool, EdgeHandleManager::HandleHitType) {}
        };

        EdgeToolController::EdgeToolController(EdgeTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveEdgePart(tool));
            addController(new SelectEdgePart(tool));
        }
    }
}
