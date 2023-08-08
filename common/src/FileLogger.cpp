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

#include <QString>

#include "Ensure.h"
#include "IO/DiskIO.h"
#include "IO/SystemPaths.h"

#include <cassert>
#include <string>

namespace TrenchBroom
{
namespace
{
std::ofstream openLogFile(const std::filesystem::path& path)
{
  return IO::Disk::createDirectory(path.parent_path())
    .transform([&](auto) {
      return std::ofstream{path, std::ios::out};
    })
    .if_error([](const auto& e) {
      throw std::runtime_error{"Could not open log file: " + e.msg};
    })
    .value();
}
} // namespace
FileLogger::FileLogger(const std::filesystem::path& filePath)
  : m_stream{openLogFile(filePath)}
{
  ensure(m_stream, "log file could not be opened");
}

FileLogger& FileLogger::instance()
{
  static FileLogger Instance(IO::SystemPaths::logFilePath());
  return Instance;
}

void FileLogger::doLog(const LogLevel /* level */, const std::string& message)
{
  assert(m_stream);
  if (m_stream)
  {
    m_stream << message << std::endl;
  }
}

void FileLogger::doLog(const LogLevel level, const QString& message)
{
  log(level, message.toStdString());
}
} // namespace TrenchBroom
