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

#include "ReparentNodesCommand.h"

#include "mdl/AddRemoveNodesUtils.h"

namespace tb::mdl
{

std::unique_ptr<ReparentNodesCommand> ReparentNodesCommand::reparent(
  std::map<Node*, std::vector<Node*>> nodesToAdd,
  std::map<Node*, std::vector<Node*>> nodesToRemove)
{
  return std::make_unique<ReparentNodesCommand>(
    std::move(nodesToAdd), std::move(nodesToRemove));
}

ReparentNodesCommand::ReparentNodesCommand(
  std::map<Node*, std::vector<Node*>> nodesToAdd,
  std::map<Node*, std::vector<Node*>> nodesToRemove)
  : UpdateLinkedGroupsCommandBase{"Reparent Objects", true}
  , m_nodesToAdd{std::move(nodesToAdd)}
  , m_nodesToRemove{std::move(nodesToRemove)}
{
}

bool ReparentNodesCommand::doPerformDo(Map& map)
{
  removeNodesAndNotify(m_nodesToRemove, map);
  addNodesAndNotify(m_nodesToAdd, map);
  return true;
}

bool ReparentNodesCommand::doPerformUndo(Map& map)
{
  removeNodesAndNotify(m_nodesToAdd, map);
  addNodesAndNotify(m_nodesToRemove, map);
  return true;
}

} // namespace tb::mdl
