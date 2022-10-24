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

#include "FileLogger.h"

#include "Ensure.h"
#include "IO/DiskIO.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"

#include <cassert>
#include <string>

#include <QString>

namespace TrenchBroom
{
FileLogger::FileLogger(const IO::Path& filePath)
  : m_file(nullptr)
{
  const auto fixedPath = IO::Disk::fixPath(filePath);
  IO::Disk::ensureDirectoryExists(fixedPath.deleteLastComponent());
  m_file = openPathAsFILE(fixedPath, "w");
  ensure(m_file != nullptr, "log file could not be opened");
}

FileLogger::~FileLogger()
{
  if (m_file != nullptr)
  {
    fclose(m_file);
    m_file = nullptr;
  }
}

FileLogger& FileLogger::instance()
{
  static FileLogger Instance(IO::SystemPaths::logFilePath());
  return Instance;
}

void FileLogger::doLog(const LogLevel /* level */, const std::string& message)
{
  assert(m_file != nullptr);
  if (m_file != nullptr)
  {
    std::fprintf(m_file, "%s\n", message.c_str());
    std::fflush(m_file);
  }
}

void FileLogger::doLog(const LogLevel level, const QString& message)
{
  log(level, message.toStdString());
}
} // namespace TrenchBroom
