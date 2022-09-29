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

#include "ReparentNodesCommand.h"

#include "Model/UpdateLinkedGroupsError.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
namespace View {
std::unique_ptr<ReparentNodesCommand> ReparentNodesCommand::reparent(
  std::map<Model::Node*, std::vector<Model::Node*>> nodesToAdd,
  std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove) {
  return std::make_unique<ReparentNodesCommand>(std::move(nodesToAdd), std::move(nodesToRemove));
}

ReparentNodesCommand::ReparentNodesCommand(
  std::map<Model::Node*, std::vector<Model::Node*>> nodesToAdd,
  std::map<Model::Node*, std::vector<Model::Node*>> nodesToRemove)
  : UpdateLinkedGroupsCommandBase("Reparent Objects", true)
  , m_nodesToAdd(std::move(nodesToAdd))
  , m_nodesToRemove(std::move(nodesToRemove)) {}

std::unique_ptr<CommandResult> ReparentNodesCommand::doPerformDo(
  MapDocumentCommandFacade* document) {
  document->performRemoveNodes(m_nodesToRemove);
  document->performAddNodes(m_nodesToAdd);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> ReparentNodesCommand::doPerformUndo(
  MapDocumentCommandFacade* document) {
  document->performRemoveNodes(m_nodesToAdd);
  document->performAddNodes(m_nodesToRemove);
  return std::make_unique<CommandResult>(true);
}
} // namespace View
} // namespace TrenchBroom
