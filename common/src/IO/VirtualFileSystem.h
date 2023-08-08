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

#pragma once

#include "IO/FileSystem.h"

#include <kdl/result_forward.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace TrenchBroom::IO
{

struct FileSystemError;

class VirtualMountPointId
{
private:
  size_t m_id;

  VirtualMountPointId();

public:
  friend bool operator==(const VirtualMountPointId& lhs, const VirtualMountPointId& rhs);
  friend bool operator!=(const VirtualMountPointId& lhs, const VirtualMountPointId& rhs);

  friend class VirtualFileSystem;
};

struct VirtualMountPoint
{
  VirtualMountPointId id;
  std::filesystem::path path;
  std::unique_ptr<FileSystem> mountedFileSystem;
};

class VirtualFileSystem : public FileSystem
{
private:
  std::vector<VirtualMountPoint> m_mountPoints;

public:
  kdl::result<std::filesystem::path, FileSystemError> makeAbsolute(
    const std::filesystem::path& path) const override;
  PathInfo pathInfo(const std::filesystem::path& path) const override;

  VirtualMountPointId mount(
    const std::filesystem::path& path, std::unique_ptr<FileSystem> fs);
  bool unmount(const VirtualMountPointId& id);
  void unmountAll();

protected:
  std::vector<std::filesystem::path> doGetDirectoryContents(
    const std::filesystem::path& path) const override;
  std::shared_ptr<File> doOpenFile(const std::filesystem::path& path) const override;
};

class WritableVirtualFileSystem : public WritableFileSystem
{
private:
  VirtualFileSystem m_virtualFs;
  WritableFileSystem& m_writableFs;

public:
  WritableVirtualFileSystem(
    VirtualFileSystem virtualFs, std::unique_ptr<WritableFileSystem> writableFs);

  using FileSystem::directoryContents;

  kdl::result<std::filesystem::path, FileSystemError> makeAbsolute(
    const std::filesystem::path& path) const override;
  PathInfo pathInfo(const std::filesystem::path& path) const override;

private:
  std::vector<std::filesystem::path> doGetDirectoryContents(
    const std::filesystem::path& path) const override;
  std::shared_ptr<File> doOpenFile(const std::filesystem::path& path) const override;

  kdl::result<void, FileSystemError> doCreateFile(
    const std::filesystem::path& path, const std::string& contents) override;
  kdl::result<bool, FileSystemError> doCreateDirectory(
    const std::filesystem::path& path) override;
  void doDeleteFile(const std::filesystem::path& path) override;
  void doCopyFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath) override;
  void doMoveFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath) override;
};

} // namespace TrenchBroom::IO
