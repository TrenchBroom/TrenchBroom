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
#include "ui/MapDocument.h"

namespace tb::mdl
{
namespace
{

void doPushGroup(mdl::GroupNode& groupNode, ui::MapDocument& document)
{
  document.editorContext().pushGroup(groupNode);
  document.groupWasOpenedNotifier(groupNode);
}

mdl::GroupNode& doPopGroup(ui::MapDocument& document)
{
  auto& editorContext = document.editorContext();
  auto& previousGroup = *editorContext.currentGroup();
  editorContext.popGroup();
  document.groupWasClosedNotifier(previousGroup);
  return previousGroup;
}

} // namespace

std::unique_ptr<CurrentGroupCommand> CurrentGroupCommand::push(mdl::GroupNode* group)
{
  return std::make_unique<CurrentGroupCommand>(group);
}

std::unique_ptr<CurrentGroupCommand> CurrentGroupCommand::pop()
{
  return std::make_unique<CurrentGroupCommand>(nullptr);
}

CurrentGroupCommand::CurrentGroupCommand(mdl::GroupNode* group)
  : UndoableCommand{group ? "Push Group" : "Pop Group", false}
  , m_group{group}
{
}

std::unique_ptr<CommandResult> CurrentGroupCommand::doPerformDo(ui::MapDocument& document)
{
  if (m_group)
  {
    doPushGroup(*m_group, document);
    m_group = nullptr;
  }
  else
  {
    m_group = &doPopGroup(document);
  }
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> CurrentGroupCommand::doPerformUndo(
  ui::MapDocument& document)
{
  return doPerformDo(document);
}

} // namespace tb::mdl
