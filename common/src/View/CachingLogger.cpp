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

#include <string>

namespace TrenchBroom
{
namespace View
{
CachingLogger::Message::Message(const LogLevel i_level, const QString& i_str)
  : level(i_level)
  , str(i_str)
{
}

CachingLogger::CachingLogger()
  : m_logger(nullptr)
{
}

void CachingLogger::setParentLogger(Logger* logger)
{
  m_logger = logger;
  if (m_logger != nullptr)
  {
    for (const Message& message : m_cachedMessages)
    {
      log(message.level, message.str);
    }
    m_cachedMessages.clear();
  }
}

void CachingLogger::doLog(const LogLevel level, const std::string& message)
{
  doLog(level, QString::fromStdString(message));
}

void CachingLogger::doLog(const LogLevel level, const QString& message)
{
  if (m_logger == nullptr)
  {
    m_cachedMessages.push_back(Message(level, message));
  }
  else
  {
    m_logger->log(level, message);
  }
}
} // namespace View
} // namespace TrenchBroom
