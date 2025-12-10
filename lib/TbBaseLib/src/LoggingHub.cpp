/*
 Copyright (C) 2025 Kristian Duske

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

#include "LoggingHub.h"

namespace tb
{

void LoggingHub::setTargetLogger(Logger* targetLogger)
{
  const auto lock = std::lock_guard{m_cacheMutex};

  m_targetLogger = targetLogger;
  if (m_targetLogger)
  {
    m_cache.getCachedMessages([this](const auto level, const auto& message) {
      m_targetLogger->log(level, message);
    });
  }
}

void LoggingHub::doLog(const LogLevel level, const std::string_view message)
{
  auto lock = std::lock_guard{m_cacheMutex};

  if (m_targetLogger)
  {
    m_targetLogger->log(level, message);
  }
  else
  {
    m_cache.cacheMessage(level, message);
  }
}

} // namespace tb
