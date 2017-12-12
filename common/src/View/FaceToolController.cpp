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

#include "FaceToolController.h"

#include "View/FaceTool.h"
#include "View/Lasso.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        class FaceToolController::SelectFacePart : public SelectPartBase<Polygon3> {
        public:
            SelectFacePart(FaceTool* tool) :
            SelectPartBase(tool, FaceHandleManager::HandleHit) {}
        private:
            bool equalHandles(const Polygon3& lhs, const Polygon3& rhs) const override {
                if (lhs.vertexCount() != rhs.vertexCount())
                    return false;

                auto lc = std::begin(lhs);
                auto rc = std::begin(rhs);
                const auto le = std::end(lhs);
                while (lc != le) {
                    assert(rc != std::end(rhs));
                    if (lc->squaredDistanceTo(*rc) >= MaxHandleDistance * MaxHandleDistance)
                        return false;
                    ++lc; ++rc;
                }
                assert(rc == std::end(rhs));

                return true;
            }
        };
        
        class FaceToolController::MoveFacePart : public MovePartBase {
        public:
            MoveFacePart(FaceTool* tool) :
            MovePartBase(tool, FaceHandleManager::HandleHit) {}
        private:
            const Model::Hit& findDragHandle(const InputState& inputState) const {
                return inputState.pickResult().query().type(FaceHandleManager::HandleHit).occluded().first();
            }
        };
        
        FaceToolController::FaceToolController(FaceTool* tool) :
        VertexToolControllerBase(tool) {
            addController(new MoveFacePart(tool));
            addController(new SelectFacePart(tool));
        }
    }
}

