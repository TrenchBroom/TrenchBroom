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

#include "UpdateLinkedGroupsCommandBase.h"

#include "Logger.h"
#include "mdl/Map.h"
#include "mdl/UpdateLinkedGroupsCommand.h"

#include "kdl/result.h"

#include <string>

namespace tb::mdl
{

UpdateLinkedGroupsCommandBase::UpdateLinkedGroupsCommandBase(
  std::string name,
  const bool updateModificationCount,
  std::vector<GroupNode*> changedLinkedGroups)
  : UndoableCommand{std::move(name), updateModificationCount}
  , m_updateLinkedGroupsHelper{std::move(changedLinkedGroups)}
{
}

UpdateLinkedGroupsCommandBase::~UpdateLinkedGroupsCommandBase() = default;

std::unique_ptr<CommandResult> UpdateLinkedGroupsCommandBase::performDo(Map& map)
{
  // reimplemented from UndoableCommand::performDo
  auto commandResult = Command::performDo(map);
  if (!commandResult->success())
  {
    return commandResult;
  }

  return m_updateLinkedGroupsHelper.applyLinkedGroupUpdates(map) | kdl::transform([&]() {
           setModificationCount(map);
           return std::move(commandResult);
         })
         | kdl::transform_error([&](auto e) {
             doPerformUndo(map);
             map.logger().error() << e.msg;
             return std::make_unique<CommandResult>(false);
           })
         | kdl::value();
}

std::unique_ptr<CommandResult> UpdateLinkedGroupsCommandBase::performUndo(Map& map)
{
  auto commandResult = UndoableCommand::performUndo(map);
  if (commandResult->success())
  {
    m_updateLinkedGroupsHelper.undoLinkedGroupUpdates(map);
  }
  return commandResult;
}

bool UpdateLinkedGroupsCommandBase::collateWith(UndoableCommand& command)
{
  assert(&command != this);

  if (
    auto* updateLinkedGroupsCommand = dynamic_cast<UpdateLinkedGroupsCommand*>(&command))
  {
    m_updateLinkedGroupsHelper.collateWith(
      updateLinkedGroupsCommand->m_updateLinkedGroupsHelper);
    return true;
  }

  if (UndoableCommand::collateWith(command))
  {
    if (
      auto* updateLinkedGroupsCommandBase =
        dynamic_cast<UpdateLinkedGroupsCommandBase*>(&command))
    {
      m_updateLinkedGroupsHelper.collateWith(
        updateLinkedGroupsCommandBase->m_updateLinkedGroupsHelper);
    }
    return true;
  }

  return false;
}

} // namespace tb::mdl
