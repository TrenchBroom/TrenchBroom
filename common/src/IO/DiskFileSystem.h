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

#pragma once

#include "IO/FileSystem.h"

#include <filesystem>
#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
class DiskFileSystem : public virtual FileSystem
{
protected:
  std::filesystem::path m_root;

public:
  explicit DiskFileSystem(const std::filesystem::path& root, bool ensureExists = true);

  const std::filesystem::path& root() const;

protected:
  std::filesystem::path doMakeAbsolute(const std::filesystem::path& path) const override;
  PathInfo doGetPathInfo(const std::filesystem::path& path) const override;
  std::vector<std::filesystem::path> doGetDirectoryContents(
    const std::filesystem::path& path) const override;
  std::shared_ptr<File> doOpenFile(const std::filesystem::path& path) const override;
};

#ifdef _MSC_VER
// MSVC complains about the fact that this class inherits some (pure virtual) method
// declarations several times from different base classes, even though there is only one
// definition.
#pragma warning(push)
#pragma warning(disable : 4250)
#endif
class WritableDiskFileSystem : public DiskFileSystem, public WritableFileSystem
{
public:
  WritableDiskFileSystem(const std::filesystem::path& root, bool create);

private:
  void doCreateFile(
    const std::filesystem::path& path, const std::string& contents) override;
  bool doCreateDirectory(const std::filesystem::path& path) override;
  void doDeleteFile(const std::filesystem::path& path) override;
  void doCopyFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath,
    bool overwrite) override;
  void doMoveFile(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destPath,
    bool overwrite) override;
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif
} // namespace IO
} // namespace TrenchBroom
