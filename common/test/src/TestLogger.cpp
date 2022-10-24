/*
 Copyright (C) 2020 Kristian Duske

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

#include "TestLogger.h"

namespace TrenchBroom
{
std::size_t TestLogger::countMessages() const
{
  return countMessages(LogLevel::Debug) + countMessages(LogLevel::Info)
         + countMessages(LogLevel::Warn) + countMessages(LogLevel::Error);
}

std::size_t TestLogger::countMessages(const LogLevel level) const
{
  const auto it = m_messages.find(level);
  if (it == std::end(m_messages))
  {
    return 0u;
  }
  else
  {
    return it->second;
  }
}

void TestLogger::doLog(const LogLevel level, const std::string&)
{
  m_messages[level] += 1u;
}

void TestLogger::doLog(const LogLevel level, const QString&)
{
  m_messages[level] += 1u;
}
} // namespace TrenchBroom
