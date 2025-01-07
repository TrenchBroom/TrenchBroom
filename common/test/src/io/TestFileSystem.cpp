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

#include "TestFileSystem.h"

#include "io/ObjectFile.h"
#include "io/TraversalMode.h"

#include "kdl/overload.h"
#include "kdl/path_utils.h"
#include "kdl/reflection_impl.h"

namespace tb::io
{

kdl_reflect_impl(Object);

std::shared_ptr<File> makeObjectFile(const int id)
{
  return std::make_shared<ObjectFile<Object>>(Object{id});
}

namespace
{
const std::string& getEntryName(const Entry& entry)
{
  return std::visit([](const auto& e) -> const std::string& { return e.name; }, entry);
}

PathInfo getEntryType(const Entry& entry)
{
  return std::visit([](const auto& e) { return e.type; }, entry);
}

const Entry* getChild(const Entry& entry, const std::string& name)
{
  return std::visit(
    kdl::overload(
      [&](const DirectoryEntry& d) -> const Entry* {
        const auto it =
          std::find_if(d.entries.begin(), d.entries.end(), [&](const auto& child) {
            return getEntryName(child) == name;
          });
        return it != d.entries.end() ? &*it : nullptr;
      },
      [](const auto&) -> const Entry* { return nullptr; }),
    entry);
}
} // namespace

TestFileSystem::TestFileSystem(
  Entry root,
  std::unordered_map<std::string, FileSystemMetadata> metadata,
  std::filesystem::path absolutePathPrefix)
  : m_root{std::move(root)}
  , m_metadata{std::move(metadata)}
  , m_absolutePathPrefix{std::move(absolutePathPrefix)}
{
}

const Entry* TestFileSystem::findEntry(std::filesystem::path path) const
{
  const Entry* entry = &m_root;
  while (!path.empty() && entry != nullptr)
  {
    entry = getChild(*entry, kdl::path_front(path).string());
    path = kdl::path_pop_front(path);
  }
  return entry;
}

PathInfo TestFileSystem::pathInfo(const std::filesystem::path& path) const
{
  const auto* entry = findEntry(path);
  return entry ? getEntryType(*entry) : PathInfo::Unknown;
}

const FileSystemMetadata* TestFileSystem::metadata(
  const std::filesystem::path& path, const std::string& key) const
{
  if (findEntry(path))
  {
    if (const auto it = m_metadata.find(key); it != m_metadata.end())
    {
      return &it->second;
    }
  }
  return nullptr;
}

Result<std::filesystem::path> TestFileSystem::makeAbsolute(
  const std::filesystem::path& path) const
{
  return m_absolutePathPrefix / path;
}

namespace
{
void doFindImpl(
  const Entry& entry,
  const std::filesystem::path& entryPath,
  const size_t depth,
  const TraversalMode& traversalMode,
  std::vector<std::filesystem::path>& result)
{
  if (!traversalMode.depth || depth <= *traversalMode.depth)
  {
    std::visit(
      kdl::overload(
        [&](const DirectoryEntry& d) {
          for (const auto& child : d.entries)
          {
            const auto childPath = entryPath / getEntryName(child);
            result.push_back(childPath);
            doFindImpl(child, childPath, depth + 1, traversalMode, result);
          }
        },
        [](const auto&) {}),
      entry);
  }
}
} // namespace

Result<std::vector<std::filesystem::path>> TestFileSystem::doFind(
  const std::filesystem::path& path, const TraversalMode& traversalMode) const
{
  auto result = std::vector<std::filesystem::path>{};
  if (const auto* entry = findEntry(path))
  {
    doFindImpl(*entry, path, 0, traversalMode, result);
  }
  return result;
}

Result<std::shared_ptr<File>> TestFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  if (const auto* fileEntry = std::get_if<FileEntry>(findEntry(path)))
  {
    return fileEntry->file;
  }
  return Error{};
}

} // namespace tb::io
