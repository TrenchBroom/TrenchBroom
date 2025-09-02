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

#include "ui/AddRemoveNodesUtils.h"

namespace tb::ui
{

std::unique_ptr<ReparentNodesCommand> ReparentNodesCommand::reparent(
  std::map<mdl::Node*, std::vector<mdl::Node*>> nodesToAdd,
  std::map<mdl::Node*, std::vector<mdl::Node*>> nodesToRemove)
{
  return std::make_unique<ReparentNodesCommand>(
    std::move(nodesToAdd), std::move(nodesToRemove));
}

ReparentNodesCommand::ReparentNodesCommand(
  std::map<mdl::Node*, std::vector<mdl::Node*>> nodesToAdd,
  std::map<mdl::Node*, std::vector<mdl::Node*>> nodesToRemove)
  : UpdateLinkedGroupsCommandBase{"Reparent Objects", true}
  , m_nodesToAdd{std::move(nodesToAdd)}
  , m_nodesToRemove{std::move(nodesToRemove)}
{
}

std::unique_ptr<CommandResult> ReparentNodesCommand::doPerformDo(MapDocument& document)
{
  removeNodesAndNotify(m_nodesToRemove, document);
  addNodesAndNotify(m_nodesToAdd, document);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> ReparentNodesCommand::doPerformUndo(MapDocument& document)
{
  removeNodesAndNotify(m_nodesToAdd, document);
  addNodesAndNotify(m_nodesToRemove, document);
  return std::make_unique<CommandResult>(true);
}

} // namespace tb::ui
