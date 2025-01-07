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

#include "Result.h"
#include "io/FileSystem.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace tb::io
{

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
  Result<std::filesystem::path> makeAbsolute(
    const std::filesystem::path& path) const override;
  PathInfo pathInfo(const std::filesystem::path& path) const override;
  const FileSystemMetadata* metadata(
    const std::filesystem::path& path, const std::string& key) const override;

  VirtualMountPointId mount(
    const std::filesystem::path& path, std::unique_ptr<FileSystem> fs);
  bool unmount(const VirtualMountPointId& id);
  void unmountAll();

protected:
  Result<std::vector<std::filesystem::path>> doFind(
    const std::filesystem::path& path, const TraversalMode& traversalMode) const override;
  Result<std::shared_ptr<File>> doOpenFile(
    const std::filesystem::path& path) const override;
};

class WritableVirtualFileSystem : public WritableFileSystem
{
private:
  VirtualFileSystem m_virtualFs;
  WritableFileSystem& m_writableFs;

public:
  WritableVirtualFileSystem(
    VirtualFileSystem virtualFs, std::unique_ptr<WritableFileSystem> writableFs);

  using FileSystem::find;

  Result<std::filesystem::path> makeAbsolute(
    const std::filesystem::path& path) const override;
  PathInfo pathInfo(const std::filesystem::path& path) const override;
  const FileSystemMetadata* metadata(
    const std::filesystem::path& path, const std::string& key) const override;

private:
  Result<std::vector<std::filesystem::path>> doFind(
    const std::filesystem::path& path, const TraversalMode& traversalMode) const override;
  Result<std::shared_ptr<File>> doOpenFile(
    const std::filesystem::path& path) const override;

  Result<void> doCreateFile(
    const std::filesystem::path& path, const std::string& contents) override;
  Result<bool> doCreateDirectory(const std::filesystem::path& path) override;
  Result<bool> doDeleteFile(const std::filesystem::path& path) override;
  Result<void> doCopyFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath) override;
  Result<void> doMoveFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath) override;
  Result<void> doRenameDirectory(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath) override;
};

} // namespace tb::io
