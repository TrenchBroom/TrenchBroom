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

#pragma once

#include "Macros.h"
#include "View/UndoableCommand.h"
#include "View/UpdateLinkedGroupsHelper.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
namespace Model {
class GroupNode;
class Node;
} // namespace Model

namespace View {
class ReparentNodesCommand : public UndoableCommand {
public:
  static const CommandType Type;

private:
  std::map<Model::Node*, std::vector<Model::Node*>> m_nodesToAdd;
  std::map<Model::Node*, std::vector<Model::Node*>> m_nodesToRemove;
  UpdateLinkedGroupsHelper m_updateLinkedGroupsHelper;

public:
  static std::unique_ptr<ReparentNodesCommand> reparent(
    std::map<Model::Node*, std::vector<Model::Node*>> nodesToAdd,
    std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove,
    std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>>
      linkedGroupsToUpdate);

  ReparentNodesCommand(
    std::map<Model::Node*, std::vector<Model::Node*>> nodesToAdd,
    std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove,
    std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>>
      linkedGroupsToUpdate);

private:
  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
  std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

  void doAction(MapDocumentCommandFacade* document);
  void undoAction(MapDocumentCommandFacade* document);

  bool doCollateWith(UndoableCommand& command) override;

  deleteCopyAndMove(ReparentNodesCommand);
};
} // namespace View
} // namespace TrenchBroom
