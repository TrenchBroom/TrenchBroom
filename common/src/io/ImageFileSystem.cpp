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

#include "ImageFileSystem.h"

#include "io/File.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"

#include "kdl/overload.h"
#include "kdl/path_utils.h"

#include <cassert>
#include <memory>

namespace tb::io
{
namespace
{

const std::filesystem::path& getName(const ImageEntry& entry)
{
  return std::visit(
    [](const auto& x) -> const std::filesystem::path& { return x.name; }, entry);
}

bool isDirectory(const ImageEntry& entry)
{
  return std::visit(
    kdl::overload(
      [](const ImageDirectoryEntry&) { return true; },
      [](const ImageFileEntry&) { return false; }),
    entry);
}

auto& addEntry(ImageDirectoryEntry& parent, ImageEntry&& entry)
{
  parent.entryMapLC[kdl::path_to_lower(getName(entry))] = parent.entries.size();
  parent.entries.push_back(std::move(entry));
  return parent.entries.back();
}

template <typename Entry>
auto findEntry(Entry& directoryEntry, const std::filesystem::path& nameLC)
{
  using difftype = std::vector<ImageEntry>::difference_type;

  const auto entryIndexIt = directoryEntry.entryMapLC.find(nameLC);
  return entryIndexIt != directoryEntry.entryMapLC.end()
           ? std::next(directoryEntry.entries.begin(), difftype(entryIndexIt->second))
           : directoryEntry.entries.end();
}

template <typename F>
auto withEntry(
  const std::filesystem::path& searchPathLC,
  const ImageEntry& currentEntry,
  const std::filesystem::path& currentPath,
  const F& f,
  decltype(f(
    std::declval<const ImageEntry&>(),
    std::declval<const std::filesystem::path&>())) defaultResult)
{
  if (searchPathLC.empty())
  {
    return f(currentEntry, currentPath);
  }

  return std::visit(
    kdl::overload(
      [&](const ImageDirectoryEntry& directoryEntry) {
        const auto nameLC = kdl::path_front(searchPathLC);
        const auto entryIt = findEntry(directoryEntry, nameLC);

        return entryIt != directoryEntry.entries.end()
                 ? withEntry(
                     kdl::path_pop_front(searchPathLC),
                     *entryIt,
                     currentPath / nameLC,
                     f,
                     defaultResult)
                 : defaultResult;
      },
      [&](const ImageFileEntry&) { return defaultResult; }),
    currentEntry);
}

template <typename F>
void withEntry(
  const std::filesystem::path& searchPathLC,
  const ImageEntry& currentEntry,
  const std::filesystem::path& currentPath,
  const F& f)
{
  if (searchPathLC.empty())
  {
    f(currentEntry, currentPath);
  }
  else
  {
    std::visit(
      kdl::overload(
        [&](const ImageDirectoryEntry& directoryEntry) {
          const auto nameLC = kdl::path_front(searchPathLC);
          const auto entryIt = findEntry(directoryEntry, nameLC);

          if (entryIt != directoryEntry.entries.end())
          {
            withEntry(
              kdl::path_pop_front(searchPathLC), *entryIt, currentPath / nameLC, f);
          }
        },
        [&](const ImageFileEntry&) {}),
      currentEntry);
  }
}

const ImageEntry* findEntry(const std::filesystem::path& path, const ImageEntry& parent)
{
  return withEntry(
    path,
    parent,
    std::filesystem::path{},
    [](const ImageEntry& entry, const std::filesystem::path&) { return &entry; },
    nullptr);
}

ImageDirectoryEntry& findOrCreateDirectory(
  const std::filesystem::path& path, ImageDirectoryEntry& parent)
{
  if (path.empty())
  {
    return parent;
  }

  auto name = kdl::path_front(path);
  auto nameLC = kdl::path_to_lower(name);
  auto entryIt = findEntry(parent, nameLC);
  if (entryIt != parent.entries.end())
  {
    return std::visit(
      kdl::overload(
        [&](ImageDirectoryEntry& directoryEntry) -> ImageDirectoryEntry& {
          return findOrCreateDirectory(kdl::path_pop_front(path), directoryEntry);
        },
        [&](ImageFileEntry&) -> ImageDirectoryEntry& {
          *entryIt = ImageDirectoryEntry{std::move(name), {}, {}};
          return findOrCreateDirectory(
            kdl::path_pop_front(path), std::get<ImageDirectoryEntry>(*entryIt));
        }),
      *entryIt);
  }
  else
  {
    return findOrCreateDirectory(
      kdl::path_pop_front(path),
      std::get<ImageDirectoryEntry>(
        addEntry(parent, ImageDirectoryEntry{std::move(name), {}, {}})));
  }
}
} // namespace

ImageFileSystemBase::ImageFileSystemBase()
  : m_root{ImageDirectoryEntry{{}, {}, {}}}
{
}

ImageFileSystemBase::~ImageFileSystemBase() = default;

Result<std::filesystem::path> ImageFileSystemBase::makeAbsolute(
  const std::filesystem::path& path) const
{
  return Result<std::filesystem::path>{"/" / path};
}

Result<void> ImageFileSystemBase::reload()
{
  m_root = ImageDirectoryEntry{{}, {}, {}};
  return doReadDirectory();
}

void ImageFileSystemBase::setMetadata(
  std::unordered_map<std::string, FileSystemMetadata> metadata)
{
  m_metadata = std::move(metadata);
}

void ImageFileSystemBase::addFile(const std::filesystem::path& path, GetImageFile getFile)
{
  auto& directoryEntry =
    findOrCreateDirectory(path.parent_path(), std::get<ImageDirectoryEntry>(m_root));

  auto name = path.filename();
  auto nameLC = kdl::path_to_lower(name);
  if (const auto entryIt = findEntry(directoryEntry, nameLC);
      entryIt != directoryEntry.entries.end())
  {
    *entryIt = ImageFileEntry{std::move(name), std::move(getFile)};
  }
  else
  {
    addEntry(directoryEntry, ImageFileEntry{std::move(name), std::move(getFile)});
  }
}

PathInfo ImageFileSystemBase::pathInfo(const std::filesystem::path& path) const
{
  const auto* entry = findEntry(kdl::path_to_lower(path), m_root);
  return entry ? isDirectory(*entry) ? PathInfo::Directory : PathInfo::File
               : PathInfo::Unknown;
}

const FileSystemMetadata* ImageFileSystemBase::metadata(
  const std::filesystem::path& path, const std::string& key) const
{
  if (findEntry(kdl::path_to_lower(path), m_root))
  {
    if (const auto it = m_metadata.find(key); it != m_metadata.end())
    {
      return &it->second;
    }
  }
  return nullptr;
}

namespace
{
void doFindImpl(
  const ImageEntry& entry,
  const std::filesystem::path& entryPath,
  const size_t depth,
  const TraversalMode& traversalMode,
  std::vector<std::filesystem::path>& result)
{
  if (!traversalMode.depth || depth <= *traversalMode.depth)
  {
    std::visit(
      kdl::overload(
        [&](const ImageDirectoryEntry& directoryEntry) {
          for (const auto& childEntry : directoryEntry.entries)
          {
            const auto childPath = entryPath / getName(childEntry);
            result.push_back(childPath);
            doFindImpl(childEntry, childPath, depth + 1, traversalMode, result);
          }
        },
        [](const ImageFileEntry&) {}),
      entry);
  }
}
} // namespace

Result<std::vector<std::filesystem::path>> ImageFileSystemBase::doFind(
  const std::filesystem::path& path, const TraversalMode& traversalMode) const
{
  auto result = std::vector<std::filesystem::path>{};
  withEntry(
    kdl::path_to_lower(path),
    m_root,
    {},
    [&](const ImageEntry& entry, const std::filesystem::path& entryPath) {
      doFindImpl(entry, entryPath, 0, traversalMode, result);
    });
  return result;
}

Result<std::shared_ptr<File>> ImageFileSystemBase::doOpenFile(
  const std::filesystem::path& path) const
{
  return withEntry(
    kdl::path_to_lower(path),
    m_root,
    std::filesystem::path{},
    [&](const ImageEntry& entry, const std::filesystem::path&) {
      return std::visit(
        kdl::overload(
          [&](const ImageDirectoryEntry&) {
            return Result<std::shared_ptr<File>>{
              Error{"Cannot open directory entry at '" + path.string() + "'"}};
          },
          [](const ImageFileEntry& fileEntry) { return fileEntry.getFile(); }),
        entry);
    },
    Result<std::shared_ptr<File>>{Error{"'" + path.string() + "' not found"}});
}

std::unordered_map<std::string, FileSystemMetadata> makeImageFileSystemMetadata(
  std::filesystem::path imageFilePath)
{
  return {
    {FileSystemMetadataKeys::ImageFilePath, FileSystemMetadata{imageFilePath}},
  };
}

} // namespace tb::io
