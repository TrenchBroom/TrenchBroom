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
#include "View/VertexHandleManager.h"

#include <memory>

namespace TrenchBroom {
namespace View {
class FaceToolController::SelectFacePart : public SelectPartBase<vm::polygon3> {
public:
  explicit SelectFacePart(FaceTool& tool)
    : SelectPartBase(tool, FaceHandleManager::HandleHitType) {}

private:
  bool equalHandles(const vm::polygon3& lhs, const vm::polygon3& rhs) const override {
    return compareUnoriented(lhs, rhs, MaxHandleDistance) == 0;
  }
};

class FaceToolController::MoveFacePart : public MovePartBase {
public:
  explicit MoveFacePart(FaceTool& tool)
    : MovePartBase(tool, FaceHandleManager::HandleHitType) {}
};

FaceToolController::FaceToolController(FaceTool& tool)
  : VertexToolControllerBase(tool) {
  addController(std::make_unique<MoveFacePart>(tool));
  addController(std::make_unique<SelectFacePart>(tool));
}
} // namespace View
} // namespace TrenchBroom
