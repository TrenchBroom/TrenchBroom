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

#include "UndoableCommand.h"

#include "ui/MapDocument.h"

#include <string>

namespace tb::ui
{

UndoableCommand::UndoableCommand(std::string name, const bool updateModificationCount)
  : Command{std::move(name)}
  , m_modificationCount{updateModificationCount ? 1u : 0u}
{
}

UndoableCommand::~UndoableCommand() = default;

std::unique_ptr<CommandResult> UndoableCommand::performDo(MapDocument& document)
{
  auto result = Command::performDo(document);
  if (result->success())
  {
    setModificationCount(document);
  }
  return result;
}

std::unique_ptr<CommandResult> UndoableCommand::performUndo(MapDocument& document)
{
  m_state = CommandState::Undoing;
  auto result = doPerformUndo(document);
  if (result->success())
  {
    resetModificationCount(document);
    m_state = CommandState::Default;
  }
  else
  {
    m_state = CommandState::Done;
  }
  return result;
}

bool UndoableCommand::collateWith(UndoableCommand& command)
{
  assert(&command != this);
  if (doCollateWith(command))
  {
    m_modificationCount += command.m_modificationCount;
    return true;
  }
  return false;
}

bool UndoableCommand::doCollateWith(UndoableCommand&)
{
  return false;
}

void UndoableCommand::setModificationCount(MapDocument& document) const
{
  if (m_modificationCount)
  {
    document.incModificationCount(m_modificationCount);
  }
}

void UndoableCommand::resetModificationCount(MapDocument& document) const
{
  document.decModificationCount(m_modificationCount);
}

} // namespace tb::ui
