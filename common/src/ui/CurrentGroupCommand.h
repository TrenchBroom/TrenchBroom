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
#include "ui/UndoableCommand.h"

#include <memory>

namespace tb::mdl
{
class GroupNode;
}

namespace tb::ui
{
class CurrentGroupCommand : public UndoableCommand
{
private:
  mdl::GroupNode* m_group = nullptr;

public:
  static std::unique_ptr<CurrentGroupCommand> push(mdl::GroupNode* group);
  static std::unique_ptr<CurrentGroupCommand> pop();

  explicit CurrentGroupCommand(mdl::GroupNode* group);

private:
  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade& document) override;
  std::unique_ptr<CommandResult> doPerformUndo(
    MapDocumentCommandFacade& document) override;

  deleteCopyAndMove(CurrentGroupCommand);
};

} // namespace tb::ui
