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

#include "ImageFileSystem.h"

#include "Ensure.h"
#include "IO/DiskFileSystem.h"
#include "IO/File.h"
#include "IO/FileSystemError.h"
#include "IO/PathInfo.h"
#include "IO/TraversalMode.h"

#include <kdl/overload.h>
#include <kdl/path_utils.h>
#include <kdl/result.h>

#include <cassert>
#include <memory>

namespace TrenchBroom
{
namespace IO
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

template <typename I>
auto findEntry(I begin, I end, const std::filesystem::path& name)
{
  const auto nameLc = kdl::path_to_lower(name);
  return std::find_if(begin, end, [&](const auto& entry) {
    return kdl::path_to_lower(getName(entry)) == nameLc;
  });
}

template <typename F>
auto withEntry(
  const std::filesystem::path& searchPath,
  const ImageEntry& currentEntry,
  const std::filesystem::path& currentPath,
  const F& f,
  decltype(f(
    std::declval<const ImageEntry&>(),
    std::declval<const std::filesystem::path&>())) defaultResult)
{
  if (searchPath.empty())
  {
    return f(currentEntry, currentPath);
  }

  return std::visit(
    kdl::overload(
      [&](const ImageDirectoryEntry& directoryEntry) {
        const auto name = kdl::path_front(searchPath);
        const auto entryIt =
          findEntry(directoryEntry.entries.begin(), directoryEntry.entries.end(), name);

        return entryIt != directoryEntry.entries.end() ? withEntry(
                 kdl::path_pop_front(searchPath),
                 *entryIt,
                 currentPath / name,
                 f,
                 defaultResult)
                                                       : defaultResult;
      },
      [&](const ImageFileEntry&) { return defaultResult; }),
    currentEntry);
}

template <typename F>
void withEntry(
  const std::filesystem::path& searchPath,
  const ImageEntry& currentEntry,
  const std::filesystem::path& currentPath,
  const F& f)
{
  if (searchPath.empty())
  {
    f(currentEntry, currentPath);
  }
  else
  {
    std::visit(
      kdl::overload(
        [&](const ImageDirectoryEntry& directoryEntry) {
          const auto name = kdl::path_front(searchPath);
          const auto entryIt =
            findEntry(directoryEntry.entries.begin(), directoryEntry.entries.end(), name);

          if (entryIt != directoryEntry.entries.end())
          {
            withEntry(kdl::path_pop_front(searchPath), *entryIt, currentPath / name, f);
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
  auto entryIt = findEntry(parent.entries.begin(), parent.entries.end(), name);
  if (entryIt != parent.entries.end())
  {
    return std::visit(
      kdl::overload(
        [&](ImageDirectoryEntry& directoryEntry) -> ImageDirectoryEntry& {
          return findOrCreateDirectory(kdl::path_pop_front(path), directoryEntry);
        },
        [&](ImageFileEntry&) -> ImageDirectoryEntry& {
          *entryIt = ImageDirectoryEntry{std::move(name), {}};
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
        parent.entries.emplace_back(ImageDirectoryEntry{std::move(name), {}})));
  }
}
} // namespace

ImageFileSystemBase::ImageFileSystemBase(std::filesystem::path path)
  : m_path{std::move(path)}
  , m_root{ImageDirectoryEntry{{}, {}}}
{
}

ImageFileSystemBase::~ImageFileSystemBase() = default;

kdl::result<std::filesystem::path, FileSystemError> ImageFileSystemBase::makeAbsolute(
  const std::filesystem::path& path) const
{
  return kdl::result<std::filesystem::path, FileSystemError>{"/" / path};
}

void ImageFileSystemBase::reload()
{
  m_root = ImageDirectoryEntry{{}, {}};
  initialize();
}

void ImageFileSystemBase::initialize()
{
  try
  {
    doReadDirectory();
  }
  catch (const std::exception& e)
  {
    throw FileSystemException{
      "Could not initialize image file system '" + m_path.string() + "': " + e.what()};
  }
}

void ImageFileSystemBase::addFile(const std::filesystem::path& path, GetImageFile getFile)
{
  auto& directoryEntry =
    findOrCreateDirectory(path.parent_path(), std::get<ImageDirectoryEntry>(m_root));

  auto name = path.filename();
  if (const auto entryIt =
        findEntry(directoryEntry.entries.begin(), directoryEntry.entries.end(), name);
      entryIt != directoryEntry.entries.end())
  {
    *entryIt = ImageFileEntry{std::move(name), std::move(getFile)};
  }
  else
  {
    directoryEntry.entries.emplace_back(
      ImageFileEntry{std::move(name), std::move(getFile)});
  }
}

PathInfo ImageFileSystemBase::pathInfo(const std::filesystem::path& path) const
{
  const auto* entry = findEntry(path, m_root);
  return entry ? isDirectory(*entry) ? PathInfo::Directory : PathInfo::File
               : PathInfo::Unknown;
}

namespace
{
void doFindImpl(
  const ImageEntry& entry,
  const std::filesystem::path& entryPath,
  const TraversalMode traversalMode,
  std::vector<std::filesystem::path>& result)
{
  std::visit(
    kdl::overload(
      [&](const ImageDirectoryEntry& directoryEntry) {
        for (const auto& childEntry : directoryEntry.entries)
        {
          const auto childPath = entryPath / getName(childEntry);
          result.push_back(childPath);
          if (traversalMode == TraversalMode::Recursive)
          {
            doFindImpl(childEntry, childPath, traversalMode, result);
          }
        }
      },
      [](const ImageFileEntry&) {}),
    entry);
}
} // namespace

std::vector<std::filesystem::path> ImageFileSystemBase::doFind(
  const std::filesystem::path& path, const TraversalMode traversalMode) const
{
  auto result = std::vector<std::filesystem::path>{};
  withEntry(
    path,
    m_root,
    {},
    [&](const ImageEntry& entry, const std::filesystem::path& entryPath) {
      doFindImpl(entry, entryPath, traversalMode, result);
    });
  return result;
}

kdl::result<std::shared_ptr<File>, FileSystemError> ImageFileSystemBase::doOpenFile(
  const std::filesystem::path& path) const
{
  return withEntry(
    path,
    m_root,
    std::filesystem::path{},
    [](const ImageEntry& entry, const std::filesystem::path&) {
      return std::visit(
        kdl::overload(
          [&](const ImageDirectoryEntry&) -> std::shared_ptr<File> { return {}; },
          [](const ImageFileEntry& fileEntry) -> std::shared_ptr<File> {
            return fileEntry.getFile();
          }),
        entry);
    },
    {});
}

ImageFileSystem::ImageFileSystem(std::filesystem::path path)
  : ImageFileSystemBase{std::move(path)}
  , m_file{std::make_shared<CFile>(m_path)}
{
  ensure(m_path.is_absolute(), "path must be absolute");
}
} // namespace IO
} // namespace TrenchBroom
