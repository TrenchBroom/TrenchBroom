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

#include "ui/FaceToolController.h"

#include "mdl/NodeHandles.h"
#include "ui/FaceTool.h"
#include "ui/NodeHandleToolControllerParts.h"

#include <memory>

namespace tb::ui
{
namespace
{

class SelectFacePart : public NodeHandleToolSelectPartBase<FaceTool, mdl::FaceHandle>
{
public:
  explicit SelectFacePart(FaceTool& tool)
    : NodeHandleToolSelectPartBase{tool, mdl::FaceHandle::HandleHitType}
  {
  }

private:
  bool equalHandles(const mdl::FaceHandle& lhs, const mdl::FaceHandle& rhs) const override
  {
    return compareUnoriented(lhs.position, rhs.position, MaxHandleDistance) == 0;
  }
};

class MoveFacePart : public NodeHandleToolMovePartBase<FaceTool>
{
public:
  explicit MoveFacePart(FaceTool& tool)
    : NodeHandleToolMovePartBase{tool, mdl::FaceHandle::HandleHitType}
  {
  }
};

} // namespace

FaceToolController::FaceToolController(FaceTool& tool)
  : NodeHandleToolControllerBase(tool)
{
  addController(std::make_unique<MoveFacePart>(tool));
  addController(std::make_unique<SelectFacePart>(tool));
}

} // namespace tb::ui
