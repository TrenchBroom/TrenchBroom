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

#pragma once

#include "Logger.h"
#include "LoggerCache.h"

#include <mutex>
#include <string_view>

namespace TrenchBroom::View
{

class CachingLogger : public Logger
{
private:
  LoggerCache m_cache;
  std::mutex m_cacheMutex;

  Logger* m_parentLogger = nullptr;

public:
  void setParentLogger(Logger* logger);

private:
  void doLog(LogLevel level, std::string_view message) override;
  bool cacheMessage(LogLevel level, std::string_view message);
};

} // namespace TrenchBroom::View
