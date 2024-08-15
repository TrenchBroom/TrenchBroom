/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "CachingLogger.h"

namespace TrenchBroom::View
{

void CachingLogger::setParentLogger(Logger* parentLogger)
{
  m_parentLogger = parentLogger;
  if (m_parentLogger)
  {
    for (const auto& message : m_cachedMessages)
    {
      log(message.level, message.str);
    }
    m_cachedMessages.clear();
  }
}

void CachingLogger::doLog(const LogLevel level, const std::string_view message)
{
  if (!m_parentLogger)
  {
    m_cachedMessages.push_back(Message{level, std::string{message}});
  }
  else
  {
    m_parentLogger->log(level, message);
  }
}

} // namespace TrenchBroom::View
