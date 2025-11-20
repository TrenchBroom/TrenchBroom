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

#include "Contracts.h"
#include "io/DiskIO.h"
#include "io/SystemPaths.h"

#include <cassert>

namespace tb
{
namespace
{

std::ofstream openLogFile(const std::filesystem::path& path)
{
  return io::Disk::createDirectory(path.parent_path())
         | kdl::transform([&](auto) { return std::ofstream{path, std::ios::out}; })
         | kdl::if_error([](const auto& e) {
             throw std::runtime_error{"Could not open log file: " + e.msg};
           })
         | kdl::value();
}

} // namespace

FileLogger::FileLogger(const std::filesystem::path& filePath)
  : m_stream{openLogFile(filePath)}
{
  contract_assert(m_stream.is_open());
}

FileLogger& FileLogger::instance()
{
  static auto Instance = FileLogger{io::SystemPaths::logFilePath()};
  return Instance;
}

void FileLogger::doLog(const LogLevel /* level */, const std::string_view message)
{
  assert(m_stream);
  if (m_stream)
  {
    m_stream << message << std::endl;
  }
}

} // namespace tb
