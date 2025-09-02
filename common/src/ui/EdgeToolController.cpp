/*
 Copyright (C) 2010 Kristian Duske

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

#include "ui/EdgeTool.h"

#include <memory>

namespace tb::ui
{

class EdgeToolController::SelectEdgePart : public SelectPartBase<vm::segment3d>
{
public:
  explicit SelectEdgePart(EdgeTool& tool)
    : SelectPartBase{tool, mdl::EdgeHandleManager::HandleHitType}
  {
  }

private:
  bool equalHandles(const vm::segment3d& lhs, const vm::segment3d& rhs) const override
  {
    return compare(lhs, rhs, MaxHandleDistance) == 0;
  }
};

class EdgeToolController::MoveEdgePart : public MovePartBase
{
public:
  explicit MoveEdgePart(EdgeTool& tool)
    : MovePartBase{tool, mdl::EdgeHandleManager::HandleHitType}
  {
  }
};

EdgeToolController::EdgeToolController(EdgeTool& tool)
  : VertexToolControllerBase{tool}
{
  addController(std::make_unique<MoveEdgePart>(tool));
  addController(std::make_unique<SelectEdgePart>(tool));
}

} // namespace tb::ui
