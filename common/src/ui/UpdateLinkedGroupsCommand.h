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

#pragma once

#include "Macros.h"
#include "ui/UpdateLinkedGroupsCommandBase.h"

#include <vector>

namespace tb::mdl
{
class GroupNode;
} // namespace tb::mdl

namespace tb::ui
{

class UpdateLinkedGroupsCommand : public UpdateLinkedGroupsCommandBase
{
public:
  explicit UpdateLinkedGroupsCommand(std::vector<mdl::GroupNode*> changedLinkedGroups);
  ~UpdateLinkedGroupsCommand() override;

  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade& document) override;
  std::unique_ptr<CommandResult> doPerformUndo(
    MapDocumentCommandFacade& document) override;

  deleteCopyAndMove(UpdateLinkedGroupsCommand);
};

} // namespace tb::ui
