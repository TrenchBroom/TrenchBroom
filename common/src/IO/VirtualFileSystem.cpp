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

#include "IO/FileSystemUtils.h"
#include "IO/PathInfo.h"

#include <kdl/path_utils.h>
#include <kdl/vector_utils.h>

#include <optional>

namespace TrenchBroom::IO
{

namespace
{

size_t getMountPointId()
{
  static auto mountPointId = size_t(0);
  return ++mountPointId;
}

template <typename F>
auto forEachMountPoint(
  const std::vector<VirtualMountPoint>& mountPoints,
  const std::filesystem::path& path,
  const F& f,
  decltype(f(std::declval<FileSystem>(), std::declval<std::filesystem::path>()))
    defaultResult = {})
{
  for (const auto& mountPoint : mountPoints)
  {
    if (kdl::path_has_prefix(
          kdl::path_to_lower(path), kdl::path_to_lower(mountPoint.path)))
    {
      const auto pathSuffix = kdl::path_clip(path, kdl::path_length(mountPoint.path));
      if (auto result = f(*mountPoint.mountedFileSystem, pathSuffix))
      {
        return result;
      }
    }
  }

  return defaultResult;
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

VirtualMountPointId VirtualFileSystem::mount(
  const std::filesystem::path& path, std::unique_ptr<FileSystem> fs)
{
  const auto id = VirtualMountPointId{};
  m_mountPoints.insert(m_mountPoints.begin(), VirtualMountPoint{id, path, std::move(fs)});
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

std::filesystem::path VirtualFileSystem::doMakeAbsolute(
  const std::filesystem::path& path) const
{
  auto absolutePath = forEachMountPoint(
    m_mountPoints,
    path,
    [](const FileSystem& fs, const std::filesystem::path& p)
      -> std::optional<std::filesystem::path> {
      return fs.pathInfo(p) != PathInfo::Unknown
               ? safeMakeAbsolute(p, [&](const auto& pp) { return fs.makeAbsolute(pp); })
               : std::nullopt;
    });

  if (absolutePath)
  {
    return *absolutePath;
  }
  throw FileSystemException("Cannot make absolute path of '" + path.string() + "'");
}

PathInfo VirtualFileSystem::doGetPathInfo(const std::filesystem::path& path) const
{
  if (
    auto result = forEachMountPoint(
      m_mountPoints,
      path,
      [](
        const FileSystem& fs, const std::filesystem::path& p) -> std::optional<PathInfo> {
        const auto pathInfo = fs.pathInfo(p);
        return pathInfo != PathInfo::Unknown ? std::optional{pathInfo} : std::nullopt;
      }))
  {
    return *result;
  }

  return std::any_of(
           m_mountPoints.begin(),
           m_mountPoints.end(),
           [&](const auto& mountPoint) {
             return kdl::path_has_prefix(
               kdl::path_to_lower(mountPoint.path), kdl::path_to_lower(path));
           })
           ? PathInfo::Directory
           : PathInfo::Unknown;
}

std::vector<std::filesystem::path> VirtualFileSystem::doGetDirectoryContents(
  const std::filesystem::path& path) const
{
  auto result = std::vector<std::filesystem::path>{};
  for (const auto& mountPoint : m_mountPoints)
  {
    if (kdl::path_has_prefix(
          kdl::path_to_lower(path), kdl::path_to_lower(mountPoint.path)))
    {
      const auto pathSuffix = kdl::path_clip(path, kdl::path_length(mountPoint.path));
      if (mountPoint.mountedFileSystem->pathInfo(pathSuffix) == PathInfo::Directory)
      {
        result = kdl::vec_concat(
          std::move(result), mountPoint.mountedFileSystem->directoryContents(pathSuffix));
      }
    }
    else if (
      kdl::path_length(path) < kdl::path_length(mountPoint.path)
      && kdl::path_has_prefix(
        kdl::path_to_lower(mountPoint.path), kdl::path_to_lower(path)))
    {
      result.push_back(kdl::path_clip(mountPoint.path, kdl::path_length(path), 1));
    }
  }

  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

std::shared_ptr<File> VirtualFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  return forEachMountPoint(
    m_mountPoints,
    path,
    [](const FileSystem& fs, const std::filesystem::path& p) -> std::shared_ptr<File> {
      return fs.pathInfo(p) != PathInfo::Unknown ? fs.openFile(p) : nullptr;
    });
}

WritableVirtualFileSystem::WritableVirtualFileSystem(
  VirtualFileSystem virtualFs, std::unique_ptr<WritableFileSystem> writableFs)
  : m_virtualFs{std::move(virtualFs)}
  , m_writableFs{*writableFs}
{
  m_virtualFs.mount(std::filesystem::path{}, std::move(writableFs));
}

std::filesystem::path WritableVirtualFileSystem::doMakeAbsolute(
  const std::filesystem::path& path) const
{
  return m_virtualFs.makeAbsolute(path);
}

PathInfo WritableVirtualFileSystem::doGetPathInfo(const std::filesystem::path& path) const
{
  return m_virtualFs.pathInfo(path);
}

std::vector<std::filesystem::path> WritableVirtualFileSystem::doGetDirectoryContents(
  const std::filesystem::path& path) const
{
  return m_virtualFs.directoryContents(path);
}

std::shared_ptr<File> WritableVirtualFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  return m_virtualFs.openFile(path);
}

void WritableVirtualFileSystem::doCreateFile(
  const std::filesystem::path& path, const std::string& contents)
{
  m_writableFs.createFile(path, contents);
}

void WritableVirtualFileSystem::doCreateDirectory(const std::filesystem::path& path)
{
  m_writableFs.createDirectory(path);
}

void WritableVirtualFileSystem::doDeleteFile(const std::filesystem::path& path)
{
  m_writableFs.deleteFile(path);
}

void WritableVirtualFileSystem::doCopyFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  m_writableFs.copyFile(sourcePath, destPath, overwrite);
}

void WritableVirtualFileSystem::doMoveFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  m_writableFs.moveFile(sourcePath, destPath, overwrite);
}

} // namespace TrenchBroom::IO
