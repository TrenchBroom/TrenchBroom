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

#include "fs/TestUtils.h"

#include "fs/DiskIO.h"
#include "fs/ReaderException.h"

#include "kd/result.h"

#include <string>

namespace tb::fs
{

std::string readTextFile(const std::filesystem::path& path)
{
  const auto fixedPath = Disk::fixPath(path);
  return Disk::withInputStream(
           fixedPath,
           [](auto& stream) {
             return std::string{
               (std::istreambuf_iterator<char>(stream)),
               std::istreambuf_iterator<char>()};
           })
         | kdl::value();
}

Result<std::string> readTextFile(const FileSystem& fs, const std::filesystem::path& path)
{
  try
  {
    return fs.openFile(path) | kdl::transform([](const auto file) {
             return file->reader().readString(file->size());
           });
  }
  catch (const ReaderException& e)
  {
    return Error{fmt::format("Failed to read file {}: {}", path, e.what())};
  }
}

} // namespace tb::fs
