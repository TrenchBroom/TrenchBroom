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
#include "View/UndoableCommand.h"
#include "View/UpdateLinkedGroupsHelper.h"

#include <memory>
#include <string>

namespace TrenchBroom {
namespace View {
class MapDocumentCommandFacade;

class UpdateLinkedGroupsCommandBase : public UndoableCommand {
private:
  UpdateLinkedGroupsHelper m_updateLinkedGroupsHelper;

protected:
  UpdateLinkedGroupsCommandBase(
    std::string name, bool updateModificationCount,
    std::vector<Model::GroupNode*> changedLinkedGroups = {});

public:
  virtual ~UpdateLinkedGroupsCommandBase();

  std::unique_ptr<CommandResult> performDo(MapDocumentCommandFacade* document) override;
  std::unique_ptr<CommandResult> performUndo(MapDocumentCommandFacade* document) override;

  bool collateWith(UndoableCommand& command) override;

private:
  deleteCopyAndMove(UpdateLinkedGroupsCommandBase);
};
} // namespace View
} // namespace TrenchBroom
