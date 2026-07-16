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

#include "fs/DiskFileSystem.h"

#include "fs/CachedFileTree.h"
#include "fs/DiskIO.h"
#include "fs/File.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"

#include "kd/path_utils.h"
#include "kd/result.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <tuple>

namespace tb::fs
{
namespace
{

// "." lexically normalizes to itself rather than to an empty path, but the cached tree
// treats an empty path as a lookup of the root itself - so it needs to be collapsed
// explicitly here.
std::filesystem::path cacheLookupKey(const std::filesystem::path& normalizedPath)
{
  return kdl::path_to_lower(
    normalizedPath == std::filesystem::path{"."} ? std::filesystem::path{}
                                                 : normalizedPath);
}

} // namespace

DiskFileSystem::DiskFileSystem(const std::filesystem::path& root)
  : m_root{root.lexically_normal()}
{
  std::ignore = reload();
}

DiskFileSystem::~DiskFileSystem() = default;

const std::filesystem::path& DiskFileSystem::root() const
{
  return m_root;
}

Result<std::filesystem::path> DiskFileSystem::makeAbsolute(
  const std::filesystem::path& path) const
{
  const auto canonicalPath = path.lexically_normal();
  if (!canonicalPath.empty() && kdl::path_front(canonicalPath) == "..")
  {
    return Error{fmt::format("Failed to make absolute path of {}", path)};
  }
  return canonicalPath.empty() ? m_root : m_root / canonicalPath;
}

Result<void> DiskFileSystem::reload()
{
  const auto lock = std::unique_lock{m_mutex};

  auto error = std::error_code{};
  auto it = std::filesystem::recursive_directory_iterator{
    m_root, std::filesystem::directory_options::follow_directory_symlink, error};
  const auto end = std::filesystem::recursive_directory_iterator{};

  if (error)
  {
    m_cacheRoot = nullptr;
    return kdl::void_success;
  }

  // build into a local tree and only publish it below once the scan completes fully -
  // otherwise a permission error, symlink loop, or race encountered partway through
  // would silently leave m_cacheRoot holding an incomplete tree while still reporting
  // success
  auto newCacheRoot = std::make_unique<DiskEntry>(DiskDirectoryEntry{{}, {}, {}});
  auto& rootDir = std::get<DiskDirectoryEntry>(*newCacheRoot);

  try
  {
    while (it != end)
    {
      const auto relativePath = it->path().lexically_relative(m_root);
      if (it->is_directory())
      {
        findOrCreateCachedDirectory(relativePath, rootDir);
      }
      else if (it->is_regular_file())
      {
        auto& directoryEntry =
          findOrCreateCachedDirectory(relativePath.parent_path(), rootDir);
        addCachedEntry(directoryEntry, DiskFileEntry{relativePath.filename(), {}});
      }
      // else: neither a directory nor a regular file (e.g. a FIFO, device, socket, or
      // broken symlink) - skip it, matching Disk::pathInfo's PathInfo::Unknown for such
      // entries instead of caching it as a file
      ++it;
    }
  }
  catch (const std::filesystem::filesystem_error& e)
  {
    // leave the previous cache (if any) in place rather than publishing a partial scan
    return Error{fmt::format("Failed to reload {}: {}", m_root, e.what())};
  }

  m_cacheRoot = std::move(newCacheRoot);
  return kdl::void_success;
}

PathInfo DiskFileSystem::pathInfo(const std::filesystem::path& path) const
{
  const auto normalized = path.lexically_normal();
  if (normalized.is_absolute())
  {
    return Disk::pathInfo(normalized);
  }
  if (kdl::path_front(normalized) == "..")
  {
    return PathInfo::Unknown;
  }

  const auto lock = std::shared_lock{m_mutex};
  if (!m_cacheRoot)
  {
    return PathInfo::Unknown;
  }

  const auto* entry = findCachedEntry(cacheLookupKey(normalized), *m_cacheRoot);
  return entry ? isDirectoryEntry(*entry) ? PathInfo::Directory : PathInfo::File
               : PathInfo::Unknown;
}

const FileSystemMetadata* DiskFileSystem::metadata(
  const std::filesystem::path&, const std::string&) const
{
  return nullptr;
}

Result<std::vector<std::filesystem::path>> DiskFileSystem::doFind(
  const std::filesystem::path& path, const TraversalMode& traversalMode) const
{
  const auto lock = std::shared_lock{m_mutex};

  auto result = std::vector<std::filesystem::path>{};
  // guards against a reload() racing between the base find() wrapper's pathInfo()
  // check and this call clearing the cache out from under it
  if (m_cacheRoot)
  {
    withCachedEntry(
      cacheLookupKey(path.lexically_normal()),
      *m_cacheRoot,
      {},
      [&](const DiskEntry& entry, const std::filesystem::path& entryPath) {
        collectCachedEntries(entry, entryPath, 0, traversalMode, result);
      });
  }
  return result;
}

Result<std::shared_ptr<File>> DiskFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  const auto lock = std::shared_lock{m_mutex};

  // guards against a reload() racing between the base openFile() wrapper's
  // pathInfo() check and this call clearing the cache out from under it
  if (!m_cacheRoot)
  {
    return Result<std::shared_ptr<File>>{Error{fmt::format("{} not found", path)}};
  }

  return withCachedEntry(
    cacheLookupKey(path.lexically_normal()),
    *m_cacheRoot,
    std::filesystem::path{},
    [&](const DiskEntry&, const std::filesystem::path& entryPath) {
      return Disk::openFile(m_root / entryPath) | kdl::transform([](auto cFile) {
               return std::static_pointer_cast<File>(cFile);
             });
    },
    Result<std::shared_ptr<File>>{Error{fmt::format("{} not found", path)}});
}

WritableDiskFileSystem::WritableDiskFileSystem(const std::filesystem::path& root)
  : DiskFileSystem{root}
{
}

Result<void> WritableDiskFileSystem::doCreateFile(
  const std::filesystem::path& path, const std::string& contents)
{
  return reloadAfterWrite(makeAbsolute(path) | kdl::and_then([&](const auto& absPath) {
                            return Disk::withOutputStream(
                              absPath, [&](auto& stream) { stream << contents; });
                          }));
}

Result<bool> WritableDiskFileSystem::doCreateDirectory(const std::filesystem::path& path)
{
  return reloadAfterWrite(makeAbsolute(path) | kdl::and_then(Disk::createDirectory));
}

Result<bool> WritableDiskFileSystem::doDeleteFile(const std::filesystem::path& path)
{
  return reloadAfterWrite(makeAbsolute(path) | kdl::and_then(Disk::deleteFile));
}

Result<void> WritableDiskFileSystem::doCopyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return reloadAfterWrite(
    makeAbsolute(sourcePath).join(makeAbsolute(destPath))
    | kdl::and_then(Disk::copyFile));
}

Result<void> WritableDiskFileSystem::doMoveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return reloadAfterWrite(
    makeAbsolute(sourcePath).join(makeAbsolute(destPath))
    | kdl::and_then(Disk::moveFile));
}

Result<void> WritableDiskFileSystem::doRenameDirectory(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return reloadAfterWrite(
    makeAbsolute(sourcePath).join(makeAbsolute(destPath))
    | kdl::and_then(Disk::renameDirectory));
}

Result<bool> WritableDiskFileSystem::reloadAfterWrite(Result<bool> result)
{
  return std::move(result) | kdl::transform([&](const bool value) {
           std::ignore = reload();
           return value;
         });
}

Result<void> WritableDiskFileSystem::reloadAfterWrite(Result<void> result)
{
  return std::move(result) | kdl::transform([&]() { std::ignore = reload(); });
}

} // namespace tb::fs
