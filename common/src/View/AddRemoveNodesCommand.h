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
#include "View/UpdateLinkedGroupsCommandBase.h"

#include <map>
#include <memory>
#include <vector>

namespace tb::mdl
{
class Node;
} // namespace tb::mdl

namespace tb::View
{
class AddRemoveNodesCommand : public UpdateLinkedGroupsCommandBase
{
private:
  enum class Action
  {
    Add,
    Remove
  };

  Action m_action;
  std::map<mdl::Node*, std::vector<mdl::Node*>> m_nodesToAdd;
  std::map<mdl::Node*, std::vector<mdl::Node*>> m_nodesToRemove;

public:
  static std::unique_ptr<AddRemoveNodesCommand> add(
    mdl::Node* parent, const std::vector<mdl::Node*>& children);
  static std::unique_ptr<AddRemoveNodesCommand> add(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes);
  static std::unique_ptr<AddRemoveNodesCommand> remove(
    const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes);

  AddRemoveNodesCommand(
    Action action, const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes);
  ~AddRemoveNodesCommand() override;

private:
  static std::string makeName(Action action);

  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade& document) override;
  std::unique_ptr<CommandResult> doPerformUndo(
    MapDocumentCommandFacade& document) override;

  void doAction(MapDocumentCommandFacade& document);
  void undoAction(MapDocumentCommandFacade& document);

  deleteCopyAndMove(AddRemoveNodesCommand);
};

} // namespace tb::View
