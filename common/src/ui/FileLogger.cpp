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

#include "FileLogger.h"

#include "fs/DiskIO.h"
#include "ui/SystemPaths.h"

#include "kd/contracts.h"

#include <cassert>

namespace tb::ui
{
namespace
{

std::ofstream openLogFile(const std::filesystem::path& path)
{
  auto ec = std::error_code{};
  std::filesystem::create_directories(path.parent_path(), ec);
  contract_assert(!ec);

  return std::ofstream{path, std::ios::out};
}

} // namespace

FileLogger::FileLogger(const std::filesystem::path& filePath)
  : m_stream{openLogFile(filePath)}
{
  contract_assert(m_stream.is_open());
}

FileLogger& FileLogger::instance()
{
  static auto Instance = FileLogger{ui::SystemPaths::logFilePath()};
  return Instance;
}

void FileLogger::doLog(const LogLevel /* level */, const std::string_view message)
{
  if (m_stream)
  {
    m_stream << message << std::endl;
  }
}

} // namespace tb::ui
