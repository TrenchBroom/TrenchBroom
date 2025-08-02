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

#include "CurrentGroupCommand.h"

#include "mdl/EditorContext.h"
#include "mdl/Map.h"

namespace tb::mdl
{
namespace
{

void doPushGroup(GroupNode& groupNode, Map& map)
{
  map.editorContext().pushGroup(groupNode);
  map.groupWasOpenedNotifier(groupNode);
}

GroupNode& doPopGroup(Map& map)
{
  auto& editorContext = map.editorContext();
  auto& previousGroup = *editorContext.currentGroup();
  editorContext.popGroup();
  map.groupWasClosedNotifier(previousGroup);
  return previousGroup;
}

} // namespace

std::unique_ptr<CurrentGroupCommand> CurrentGroupCommand::push(GroupNode* group)
{
  return std::make_unique<CurrentGroupCommand>(group);
}

std::unique_ptr<CurrentGroupCommand> CurrentGroupCommand::pop()
{
  return std::make_unique<CurrentGroupCommand>(nullptr);
}

CurrentGroupCommand::CurrentGroupCommand(GroupNode* group)
  : UndoableCommand{group ? "Push Group" : "Pop Group", false}
  , m_group{group}
{
}

std::unique_ptr<CommandResult> CurrentGroupCommand::doPerformDo(Map& map)
{
  if (m_group)
  {
    doPushGroup(*m_group, map);
    m_group = nullptr;
  }
  else
  {
    m_group = &doPopGroup(map);
  }
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> CurrentGroupCommand::doPerformUndo(Map& map)
{
  return doPerformDo(map);
}

} // namespace tb::mdl
