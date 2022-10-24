/*
 Copyright (C) 2022 Kristian Duske

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

#include "UpdateLinkedGroupsCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom
{
namespace View
{
UpdateLinkedGroupsCommand::UpdateLinkedGroupsCommand(
  std::vector<Model::GroupNode*> changedLinkedGroups)
  : UpdateLinkedGroupsCommandBase{
    "Update Linked Groups", true, std::move(changedLinkedGroups)}
{
}

UpdateLinkedGroupsCommand::~UpdateLinkedGroupsCommand() = default;

std::unique_ptr<CommandResult> UpdateLinkedGroupsCommand::doPerformDo(
  MapDocumentCommandFacade*)
{
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> UpdateLinkedGroupsCommand::doPerformUndo(
  MapDocumentCommandFacade*)
{
  return std::make_unique<CommandResult>(true);
}
} // namespace View
} // namespace TrenchBroom
