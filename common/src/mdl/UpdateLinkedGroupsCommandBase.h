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
#include "mdl/UndoableCommand.h"
#include "mdl/UpdateLinkedGroupsHelper.h"

#include <memory>
#include <string>

namespace tb::mdl
{
class MapDocument;

class UpdateLinkedGroupsCommandBase : public UndoableCommand
{
private:
  UpdateLinkedGroupsHelper m_updateLinkedGroupsHelper;

protected:
  UpdateLinkedGroupsCommandBase(
    std::string name,
    bool updateModificationCount,
    std::vector<GroupNode*> changedLinkedGroups = {});

public:
  ~UpdateLinkedGroupsCommandBase() override;

  std::unique_ptr<CommandResult> performDo(Map& map) override;
  std::unique_ptr<CommandResult> performUndo(Map& map) override;

  bool collateWith(UndoableCommand& command) override;

private:
  deleteCopyAndMove(UpdateLinkedGroupsCommandBase);
};

} // namespace tb::mdl
