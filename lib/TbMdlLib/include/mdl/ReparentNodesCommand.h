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

#pragma once

#include "Macros.h"
#include "mdl/UpdateLinkedGroupsCommandBase.h"

#include <map>
#include <memory>
#include <vector>

namespace tb::mdl
{
class GroupNode;
class Map;
class Node;

class ReparentNodesCommand : public UpdateLinkedGroupsCommandBase
{
private:
  std::map<Node*, std::vector<Node*>> m_nodesToAdd;
  std::map<Node*, std::vector<Node*>> m_nodesToRemove;

public:
  static std::unique_ptr<ReparentNodesCommand> reparent(
    std::map<Node*, std::vector<Node*>> nodesToAdd,
    std::map<Node*, std::vector<Node*>> nodesToRemove);

  ReparentNodesCommand(
    std::map<Node*, std::vector<Node*>> nodesToAdd,
    std::map<Node*, std::vector<Node*>> nodesToRemove);

private:
  bool doPerformDo(Map& map) override;
  bool doPerformUndo(Map& map) override;

  deleteCopyAndMove(ReparentNodesCommand);
};

} // namespace tb::mdl
