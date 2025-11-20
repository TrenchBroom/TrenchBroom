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

#include "mdl/Map.h"

#include "kd/contracts.h"

#include <string>

namespace tb::mdl
{

UndoableCommand::UndoableCommand(std::string name, const bool updateModificationCount)
  : Command{std::move(name)}
  , m_modificationCount{updateModificationCount ? 1u : 0u}
{
}

UndoableCommand::~UndoableCommand() = default;

bool UndoableCommand::performDo(Map& map)
{
  const auto result = Command::performDo(map);
  if (result)
  {
    setModificationCount(map);
  }
  return result;
}

bool UndoableCommand::performUndo(Map& map)
{
  m_state = CommandState::Undoing;
  const auto result = doPerformUndo(map);
  if (result)
  {
    resetModificationCount(map);
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
  contract_pre(&command != this);

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

void UndoableCommand::setModificationCount(Map& map) const
{
  if (m_modificationCount)
  {
    map.incModificationCount(m_modificationCount);
  }
}

void UndoableCommand::resetModificationCount(Map& map) const
{
  map.decModificationCount(m_modificationCount);
}

} // namespace tb::mdl
