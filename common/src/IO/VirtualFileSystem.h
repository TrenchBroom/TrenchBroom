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
#include "IO/Path.h"

#include <memory>
#include <vector>

namespace TrenchBroom::IO
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
  Path path;
  std::unique_ptr<FileSystem> mountedFileSystem;
};

class VirtualFileSystem : public FileSystem
{
private:
  std::vector<VirtualMountPoint> m_mountPoints;

public:
  VirtualMountPointId mount(const Path& path, std::unique_ptr<FileSystem> fs);
  bool unmount(const VirtualMountPointId& id);
  void unmountAll();

protected:
  Path doMakeAbsolute(const Path& path) const override;
  PathInfo doGetPathInfo(const Path& path) const override;
  std::vector<Path> doGetDirectoryContents(const Path& path) const override;
  std::shared_ptr<File> doOpenFile(const Path& path) const override;
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

  Path doMakeAbsolute(const Path& path) const override;
  PathInfo doGetPathInfo(const Path& path) const override;
  std::vector<Path> doGetDirectoryContents(const Path& path) const override;
  std::shared_ptr<File> doOpenFile(const Path& path) const override;

  void doCreateFile(const Path& path, const std::string& contents) override;
  void doCreateDirectory(const Path& path) override;
  void doDeleteFile(const Path& path) override;
  void doCopyFile(const Path& sourcePath, const Path& destPath, bool overwrite) override;
  void doMoveFile(const Path& sourcePath, const Path& destPath, bool overwrite) override;
};

} // namespace TrenchBroom::IO
