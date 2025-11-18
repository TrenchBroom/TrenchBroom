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

#include "AddRemoveNodesCommand.h"

#include "Ensure.h"
#include "Macros.h"
#include "mdl/AddRemoveNodesUtils.h"
#include "mdl/Map.h"
#include "mdl/Node.h"

#include "kd/map_utils.h"

namespace tb::mdl
{

std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::add(
  Node* parent, const std::vector<Node*>& children)
{
  ensure(parent != nullptr, "parent is null");
  auto nodes = std::map<Node*, std::vector<Node*>>{};
  nodes[parent] = children;

  return add(nodes);
}

std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::add(
  const std::map<Node*, std::vector<Node*>>& nodes)
{
  return std::make_unique<AddRemoveNodesCommand>(Action::Add, nodes);
}

std::unique_ptr<AddRemoveNodesCommand> AddRemoveNodesCommand::remove(
  const std::map<Node*, std::vector<Node*>>& nodes)
{
  return std::make_unique<AddRemoveNodesCommand>(Action::Remove, nodes);
}

AddRemoveNodesCommand::~AddRemoveNodesCommand()
{
  kdl::map_clear_and_delete(m_nodesToAdd);
}

AddRemoveNodesCommand::AddRemoveNodesCommand(
  const Action action, const std::map<Node*, std::vector<Node*>>& nodes)
  : UpdateLinkedGroupsCommandBase{makeName(action), true}
  , m_action{action}
{
  switch (m_action)
  {
  case Action::Add:
    m_nodesToAdd = nodes;
    break;
  case Action::Remove:
    m_nodesToRemove = nodes;
    break;
    switchDefault();
  }
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

std::unique_ptr<CommandResult> AddRemoveNodesCommand::doPerformDo(Map& map)
{
  doAction(map);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> AddRemoveNodesCommand::doPerformUndo(Map& map)
{
  undoAction(map);
  return std::make_unique<CommandResult>(true);
}

void AddRemoveNodesCommand::doAction(Map& map)
{
  switch (m_action)
  {
  case Action::Add:
    addNodesAndNotify(m_nodesToAdd, map);
    break;
  case Action::Remove:
    removeNodesAndNotify(m_nodesToRemove, map);
    break;
  }

  using std::swap;
  swap(m_nodesToAdd, m_nodesToRemove);
}

void AddRemoveNodesCommand::undoAction(Map& map)
{
  switch (m_action)
  {
  case Action::Add:
    removeNodesAndNotify(m_nodesToRemove, map);
    break;
  case Action::Remove:
    addNodesAndNotify(m_nodesToAdd, map);
    break;
  }

  using std::swap;
  swap(m_nodesToAdd, m_nodesToRemove);
}

} // namespace tb::mdl
