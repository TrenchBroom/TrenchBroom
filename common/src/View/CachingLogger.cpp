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

#include "CachingLogger.h"

namespace tb::View
{

void CachingLogger::setParentLogger(Logger* parentLogger)
{
  const auto lock = std::lock_guard{m_cacheMutex};

  m_parentLogger = parentLogger;
  if (m_parentLogger)
  {
    m_cache.getCachedMessages([this](const auto level, const auto& message) {
      m_parentLogger->log(level, message);
    });
  }
}

void CachingLogger::doLog(const LogLevel level, const std::string_view message)
{
  if (!cacheMessage(level, message))
  {
    m_parentLogger->log(level, message);
  }
}

bool CachingLogger::cacheMessage(const LogLevel level, const std::string_view message)
{
  auto lock = std::lock_guard{m_cacheMutex};

  if (!m_parentLogger)
  {
    m_cache.cacheMessage(level, message);
    return true;
  }

  return false;
}

} // namespace tb::View
