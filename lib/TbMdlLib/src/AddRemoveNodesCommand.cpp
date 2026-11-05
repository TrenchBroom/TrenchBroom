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

#include "mdl/AddRemoveNodesCommand.h"

#include "base/Macros.h"
#include "mdl/AddRemoveNodesUtils.h"
#include "mdl/Map.h"
#include "mdl/Node.h"

#include <utility>

namespace tb::mdl
{

std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::add(
  kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>> nodes)
{
  return std::make_unique<AddRemoveNodesCommand>(
    Action::Add, std::move(nodes), kdl::flat_map<Node*, std::vector<Node*>>{});
}

std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::remove(
  const kdl::flat_map<Node*, std::vector<Node*>>& nodes)
{
  return std::make_unique<AddRemoveNodesCommand>(
    Action::Remove, kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>{}, nodes);
}

AddRemoveNodesCommand::~AddRemoveNodesCommand() = default;

AddRemoveNodesCommand::AddRemoveNodesCommand(
  const Action action,
  kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>> nodesToAdd,
  kdl::flat_map<Node*, std::vector<Node*>> nodesToRemove)
  : UpdateLinkedGroupsCommandBase{makeName(action), true}
  , m_action{action}
  , m_nodesToAdd{std::move(nodesToAdd)}
  , m_nodesToRemove{std::move(nodesToRemove)}
{
}

std::string AddRemoveNodesCommand::makeName(const Action action)
{
  switch (action)
  {
  case Action::Add:
    return "Add Objects";
  case Action::Remove:
    return "Remove Objects";
    switchDefault();
  }
}

bool AddRemoveNodesCommand::doPerformDo(Map& map)
{
  doAction(map);
  return true;
}

bool AddRemoveNodesCommand::doPerformUndo(Map& map)
{
  undoAction(map);
  return true;
}

void AddRemoveNodesCommand::doAction(Map& map)
{
  switch (m_action)
  {
  case Action::Add:
    m_nodesToRemove = addNodesAndNotify(std::exchange(m_nodesToAdd, {}), map);
    break;
  case Action::Remove:
    m_nodesToAdd = removeNodesAndNotify(std::exchange(m_nodesToRemove, {}), map);
    break;
  }
}

void AddRemoveNodesCommand::undoAction(Map& map)
{
  switch (m_action)
  {
  case Action::Add:
    m_nodesToAdd = removeNodesAndNotify(std::exchange(m_nodesToRemove, {}), map);
    break;
  case Action::Remove:
    m_nodesToRemove = addNodesAndNotify(std::exchange(m_nodesToAdd, {}), map);
    break;
  }
}

} // namespace tb::mdl
