/*
 Copyright (C) 2023 Kristian Duske

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

#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/PathInfo.h"

#include <kdl/overload.h>
#include <kdl/reflection_decl.h>
#include <kdl/result_forward.h>

#include <filesystem>
#include <memory>
#include <string>
#include <variant>

namespace TrenchBroom
{
namespace IO
{

struct FileSystemError;

struct Object
{
  int id;

  kdl_reflect_decl(Object, id);
};

std::shared_ptr<File> makeObjectFile(int id);

struct FileEntry
{
  constexpr static auto type = PathInfo::File;
  std::string name;
  std::shared_ptr<File> file;
};

struct DirectoryEntry;

using Entry = std::variant<FileEntry, DirectoryEntry>;

struct DirectoryEntry
{
  constexpr static auto type = PathInfo::Directory;
  std::string name;
  std::vector<Entry> entries;
};

class TestFileSystem : public FileSystem
{
private:
  Entry m_root;
  std::filesystem::path m_absolutePathPrefix;

public:
  explicit TestFileSystem(Entry root, std::filesystem::path absolutePathPrefix = {"/"});

  kdl::result<std::filesystem::path, FileSystemError> makeAbsolute(
    const std::filesystem::path& path) const override;
  PathInfo pathInfo(const std::filesystem::path& path) const override;

private:
  const Entry* findEntry(std::filesystem::path path) const;
  std::vector<std::filesystem::path> doFind(
    const std::filesystem::path& path, TraversalMode traversalMode) const override;
  kdl::result<std::shared_ptr<File>, FileSystemError> doOpenFile(
    const std::filesystem::path& path) const override;
};

} // namespace IO
} // namespace TrenchBroom
