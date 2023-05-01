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

#include <kdl/overload.h>
#include <kdl/reflection_impl.h>

namespace TrenchBroom
{
namespace IO
{

kdl_reflect_impl(Object);

std::shared_ptr<File> makeObjectFile(Path path, const int id)
{
  return std::make_shared<ObjectFile<Object>>(std::move(path), Object{id});
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

TestFileSystem::TestFileSystem(Entry root, Path absolutePathPrefix)
  : m_root{std::move(root)}
  , m_absolutePathPrefix{std::move(absolutePathPrefix)}
{
}

const Entry* TestFileSystem::findEntry(Path path) const
{
  const Entry* entry = &m_root;
  while (!path.empty() && entry != nullptr)
  {
    entry = getChild(*entry, path.firstComponent().string());
    path = path.deleteFirstComponent();
  }
  return entry;
}

Path TestFileSystem::doMakeAbsolute(const Path& path) const
{
  if (findEntry(path))
  {
    return m_absolutePathPrefix / path;
  }
  throw FileSystemException{};
}

PathInfo TestFileSystem::doGetPathInfo(const Path& path) const
{
  const auto* entry = findEntry(path);
  return entry ? getEntryType(*entry) : PathInfo::Unknown;
}

std::vector<Path> TestFileSystem::doGetDirectoryContents(const Path& path) const
{
  auto result = std::vector<Path>{};
  if (const auto* entry = findEntry(path))
  {
    std::visit(
      kdl::overload(
        [&](const DirectoryEntry& d) {
          for (const auto& child : d.entries)
          {
            const auto childPath = Path{getEntryName(child)};
            result.push_back(childPath);
          }
        },
        [](const auto&) {}),
      *entry);
  }
  return result;
}

std::shared_ptr<File> TestFileSystem::doOpenFile(const Path& path) const
{
  const auto* entry = findEntry(path);
  return entry ? std::visit(
           kdl::overload(
             [](const FileEntry& f) { return f.file; },
             [](const auto&) -> std::shared_ptr<File> { return nullptr; }),
           *entry)
               : nullptr;
}

} // namespace IO
} // namespace TrenchBroom
