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

#include "ui/EdgeToolController.h"

#include "ui/EdgeTool.h"
#include "ui/NodeHandleToolControllerParts.h"

#include <memory>

namespace tb::ui
{
namespace
{

class MoveEdgePart : public NodeHandleToolMovePartBase<EdgeTool>
{
public:
  explicit MoveEdgePart(EdgeTool& tool)
    : NodeHandleToolMovePartBase{tool, mdl::EdgeHandle::HandleHitType}
  {
  }
};

} // namespace

EdgeToolController::EdgeToolController(EdgeTool& tool)
  : NodeHandleToolControllerBase{tool}
{
  addController(std::make_unique<MoveEdgePart>(tool));
  addController(std::make_unique<NodeHandleToolSelectPart<EdgeTool>>(
    tool, mdl::EdgeHandle::HandleHitType));
}

} // namespace tb::ui
