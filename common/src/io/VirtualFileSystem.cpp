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

#include "VirtualFileSystem.h"

#include "io/File.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <optional>
#include <unordered_map>

namespace tb::io
{

namespace
{

size_t getMountPointId()
{
  static auto mountPointId = size_t(0);
  return ++mountPointId;
}

bool matches(const VirtualMountPoint& mountPoint, const std::filesystem::path& path)
{
  return kdl::path_has_prefix(
    kdl::path_to_lower(path), kdl::path_to_lower(mountPoint.path));
}

std::filesystem::path suffix(
  const VirtualMountPoint& mountPoint, const std::filesystem::path& path)
{
  assert(matches(mountPoint, path));
  return kdl::path_clip(path, kdl::path_length(mountPoint.path));
}

} // namespace

VirtualMountPointId::VirtualMountPointId()
  : m_id{getMountPointId()}
{
}

bool operator==(const VirtualMountPointId& lhs, const VirtualMountPointId& rhs)
{
  return lhs.m_id == rhs.m_id;
}

bool operator!=(const VirtualMountPointId& lhs, const VirtualMountPointId& rhs)
{
  return !(lhs == rhs);
}

Result<std::filesystem::path> VirtualFileSystem::makeAbsolute(
  const std::filesystem::path& path) const
{
  for (auto it = m_mountPoints.rbegin(); it != m_mountPoints.rend(); ++it)
  {
    const auto& mountPoint = *it;
    if (matches(mountPoint, path))
    {
      const auto pathSuffix = suffix(mountPoint, path);
      const auto absPath = mountPoint.mountedFileSystem->makeAbsolute(pathSuffix);
      if (
        absPath.is_success()
        && mountPoint.mountedFileSystem->pathInfo(pathSuffix) != PathInfo::Unknown)
      {
        return absPath;
      }
    }
  }

  return Error{fmt::format("Failed to make absolute path of {}", fmt::streamed(path))};
}

PathInfo VirtualFileSystem::pathInfo(const std::filesystem::path& path) const
{
  for (auto it = m_mountPoints.rbegin(); it != m_mountPoints.rend(); ++it)
  {
    const auto& mountPoint = *it;
    if (matches(mountPoint, path))
    {
      const auto pathSuffix = suffix(mountPoint, path);
      if (const auto pathInfo = mountPoint.mountedFileSystem->pathInfo(pathSuffix);
          pathInfo != PathInfo::Unknown)
      {
        return pathInfo;
      }
    }
  }

  return std::any_of(
           m_mountPoints.rbegin(),
           m_mountPoints.rend(),
           [&](const auto& mountPoint) {
             return kdl::path_has_prefix(
               kdl::path_to_lower(mountPoint.path), kdl::path_to_lower(path));
           })
           ? PathInfo::Directory
           : PathInfo::Unknown;
}

const FileSystemMetadata* VirtualFileSystem::metadata(
  const std::filesystem::path& path, const std::string& key) const
{
  for (auto it = m_mountPoints.rbegin(); it != m_mountPoints.rend(); ++it)
  {
    const auto& mountPoint = *it;
    if (matches(mountPoint, path))
    {
      const auto pathSuffix = suffix(mountPoint, path);
      if (const auto pathInfo = mountPoint.mountedFileSystem->pathInfo(pathSuffix);
          pathInfo != PathInfo::Unknown)
      {
        return mountPoint.mountedFileSystem->metadata(pathSuffix, key);
      }
    }
  }

  return nullptr;
}

VirtualMountPointId VirtualFileSystem::mount(
  const std::filesystem::path& path, std::unique_ptr<FileSystem> fs)
{
  const auto id = VirtualMountPointId{};
  m_mountPoints.push_back({id, path, std::move(fs)});
  return id;
}

bool VirtualFileSystem::unmount(const VirtualMountPointId& id)
{
  if (const auto it = std::find_if(
        m_mountPoints.begin(),
        m_mountPoints.end(),
        [&](const auto& mountPoint) { return mountPoint.id == id; });
      it != m_mountPoints.end())
  {
    m_mountPoints.erase(it);
    return true;
  }
  return false;
}

void VirtualFileSystem::unmountAll()
{
  m_mountPoints.clear();
}

namespace
{
std::vector<std::filesystem::path> findMatchesForCommonPrefix(
  const VirtualMountPoint& mountPoint,
  const std::filesystem::path& path,
  const TraversalMode& traversalMode)
{
  assert(kdl::path_has_prefix(mountPoint.path, path));

  const auto suffixOffset = kdl::path_length(path);
  const auto suffixLength = kdl::path_length(mountPoint.path) - suffixOffset;
  const auto suffix = kdl::path_clip(mountPoint.path, suffixOffset, suffixLength);

  const auto maxDepth =
    traversalMode.depth ? std::min(*traversalMode.depth + 1, suffixLength) : suffixLength;

  auto result = std::vector<std::filesystem::path>{};
  for (size_t i = 0; i < maxDepth; ++i)
  {
    result.push_back(kdl::path_clip(mountPoint.path, 0, suffixOffset + i + 1));
  }
  return result;
}

Result<std::vector<std::filesystem::path>> findInMountedFileSystem(
  const VirtualMountPoint& mountPoint,
  const std::filesystem::path& path,
  const TraversalMode& traversalMode)
{
  if (mountPoint.mountedFileSystem->pathInfo(path) == PathInfo::Directory)
  {
    return mountPoint.mountedFileSystem->find(path, traversalMode)
           | kdl::transform([&](auto paths) {
               return kdl::vec_transform(
                 std::move(paths), [&](auto p) { return mountPoint.path / p; });
             });
  }
  return std::vector<std::filesystem::path>{};
}

Result<std::vector<std::filesystem::path>> findMatchesForMountedFileSystem(
  const VirtualMountPoint& mountPoint,
  const std::filesystem::path& path,
  const TraversalMode& traversalMode)
{
  if (kdl::path_has_prefix(mountPoint.path, path))
  {
    // search path is a prefix of the mountpoint
    auto matchesForMountPointSuffix =
      findMatchesForCommonPrefix(mountPoint, path, traversalMode);

    // we might traverse further into the mounted file system depending on traversalMode
    if (
      const auto nestedTraversalMode = traversalMode.reduceDepth(
        kdl::path_length(mountPoint.path) - kdl::path_length(path)))
    {
      // traverse into the mounted file system
      return findInMountedFileSystem(mountPoint, "", *nestedTraversalMode)
             | kdl::transform([&](auto matchesForMountedFileSystem) {
                 return kdl::vec_concat(
                   std::move(matchesForMountPointSuffix),
                   std::move(matchesForMountedFileSystem));
               });
    }
    return matchesForMountPointSuffix;
  }
  else if (kdl::path_has_prefix(path, mountPoint.path))
  {
    // mountpoint path is a prefix of search path, so search the mounted file system
    const auto pathSuffix = kdl::path_clip(path, kdl::path_length(mountPoint.path));
    return findInMountedFileSystem(mountPoint, pathSuffix, traversalMode);
  }

  return std::vector<std::filesystem::path>{};
}

} // namespace

Result<std::vector<std::filesystem::path>> VirtualFileSystem::doFind(
  const std::filesystem::path& path, const TraversalMode& traversalMode) const
{
  return kdl::vec_transform(
           m_mountPoints,
           [&](const auto& mountPoint) {
             return findMatchesForMountedFileSystem(mountPoint, path, traversalMode);
           })
         | kdl::fold | kdl::transform([](auto nestedPaths) {
             if (nestedPaths.empty())
             {
               return std::vector<std::filesystem::path>{};
             }
             if (nestedPaths.size() == 1)
             {
               return std::move(nestedPaths.front());
             }

             auto result = std::vector<std::filesystem::path>{};
             auto lastOccurrence = std::unordered_map<std::string, size_t>{};

             for (size_t i = 0; i < nestedPaths.size(); ++i)
             {
               const auto& paths = nestedPaths[i];
               for (const auto& p : paths)
               {
                 lastOccurrence[p.generic_string()] = i;
               }
             }

             for (size_t i = 0; i < nestedPaths.size(); ++i)
             {
               auto& paths = nestedPaths[i];
               for (auto& p : paths)
               {
                 if (lastOccurrence[p.generic_string()] == i)
                 {
                   result.push_back(std::move(p));
                 }
               }
             }


             return result;
           });
}

Result<std::shared_ptr<File>> VirtualFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  for (auto it = m_mountPoints.rbegin(); it != m_mountPoints.rend(); ++it)
  {
    const auto& mountPoint = *it;
    if (matches(mountPoint, path))
    {
      const auto pathSuffix = suffix(mountPoint, path);
      if (mountPoint.mountedFileSystem->pathInfo(pathSuffix) != PathInfo::Unknown)
      {
        return mountPoint.mountedFileSystem->openFile(pathSuffix);
      }
    }
  }

  return Error{fmt::format("{} not found", fmt::streamed(path))};
}

WritableVirtualFileSystem::WritableVirtualFileSystem(
  VirtualFileSystem virtualFs, std::unique_ptr<WritableFileSystem> writableFs)
  : m_virtualFs{std::move(virtualFs)}
  , m_writableFs{*writableFs}
{
  m_virtualFs.mount(std::filesystem::path{}, std::move(writableFs));
}

Result<std::filesystem::path> WritableVirtualFileSystem::makeAbsolute(
  const std::filesystem::path& path) const
{
  return m_virtualFs.makeAbsolute(path);
}

PathInfo WritableVirtualFileSystem::pathInfo(const std::filesystem::path& path) const
{
  return m_virtualFs.pathInfo(path);
}

const FileSystemMetadata* WritableVirtualFileSystem::metadata(
  const std::filesystem::path& path, const std::string& key) const
{
  return m_virtualFs.metadata(path, key);
}

Result<std::vector<std::filesystem::path>> WritableVirtualFileSystem::doFind(
  const std::filesystem::path& path, const TraversalMode& traversalMode) const
{
  return m_virtualFs.find(path, traversalMode);
}

Result<std::shared_ptr<File>> WritableVirtualFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  return m_virtualFs.openFile(path);
}

Result<void> WritableVirtualFileSystem::doCreateFile(
  const std::filesystem::path& path, const std::string& contents)
{
  return m_writableFs.createFile(path, contents);
}

Result<bool> WritableVirtualFileSystem::doCreateDirectory(
  const std::filesystem::path& path)
{
  return m_writableFs.createDirectory(path);
}

Result<bool> WritableVirtualFileSystem::doDeleteFile(const std::filesystem::path& path)
{
  return m_writableFs.deleteFile(path);
}

Result<void> WritableVirtualFileSystem::doCopyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return m_writableFs.copyFile(sourcePath, destPath);
}

Result<void> WritableVirtualFileSystem::doMoveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return m_writableFs.moveFile(sourcePath, destPath);
}

Result<void> WritableVirtualFileSystem::doRenameDirectory(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return m_writableFs.renameDirectory(sourcePath, destPath);
}

} // namespace tb::io
