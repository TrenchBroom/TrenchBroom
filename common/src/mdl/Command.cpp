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

#include "Command.h"

#include <string>

namespace tb::mdl
{

CommandResult::CommandResult(const bool success)
  : m_success{success}
{
}

CommandResult::~CommandResult() = default;

bool CommandResult::success() const
{
  return m_success;
}

Command::Command(std::string name)
  : m_state{CommandState::Default}
  , m_name{std::move(name)}
{
}

Command::~Command() = default;

Command::CommandState Command::state() const
{
  return m_state;
}

const std::string& Command::name() const
{
  return m_name;
}

std::unique_ptr<CommandResult> Command::performDo(Map& map)
{
  m_state = CommandState::Doing;
  auto result = doPerformDo(map);
  m_state = result->success() ? CommandState::Done : CommandState::Default;
  return result;
}

} // namespace tb::mdl
