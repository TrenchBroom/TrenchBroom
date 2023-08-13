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
#include "IO/FileSystemError.h"

#include <kdl/result.h>
#include <kdl/string_compare.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <variant>

namespace TrenchBroom::IO
{
class CFile;
class File;

using GetImageFile = std::function<kdl::result<std::shared_ptr<File>, FileSystemError>()>;

struct ImageFileEntry
{
  std::filesystem::path name;
  GetImageFile getFile;
};

struct ImageDirectoryEntry;
using ImageEntry = std::variant<ImageDirectoryEntry, ImageFileEntry>;

struct ImageDirectoryEntry
{
  std::filesystem::path name;
  std::vector<ImageEntry> entries;
};

class ImageFileSystemBase : public FileSystem
{
protected:
  std::filesystem::path m_path;
  ImageEntry m_root;

  explicit ImageFileSystemBase(std::filesystem::path path);

public:
  ~ImageFileSystemBase() override;

  kdl::result<std::filesystem::path, FileSystemError> makeAbsolute(
    const std::filesystem::path& path) const override;

  /**
   * Reload this file system.
   */
  kdl::result<void, FileSystemError> reload();

protected:
  void addFile(const std::filesystem::path& path, GetImageFile getFile);

  PathInfo pathInfo(const std::filesystem::path& path) const override;

private:
  kdl::result<std::vector<std::filesystem::path>, FileSystemError> doFind(
    const std::filesystem::path& path, TraversalMode traversalMode) const override;
  kdl::result<std::shared_ptr<File>, FileSystemError> doOpenFile(
    const std::filesystem::path& path) const override;

  virtual kdl::result<void, FileSystemError> doReadDirectory() = 0;
};

class ImageFileSystem : public ImageFileSystemBase
{
protected:
  std::shared_ptr<CFile> m_file;

public:
  explicit ImageFileSystem(std::filesystem::path path);
};

template <typename T, typename... Args>
kdl::result<std::unique_ptr<T>, FileSystemError> createImageFileSystem(Args&&... args)
{
  auto fs = std::make_unique<T>(std::forward<Args>(args)...);
  return fs->reload().transform([&]() { return std::move(fs); });
}

} // namespace TrenchBroom::IO
