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

#include "Error.h"
#include "IO/FileSystem.h"
#include "Result.h"

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

using GetImageFile = std::function<Result<std::shared_ptr<File>>()>;

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
  ImageEntry m_root;

  ImageFileSystemBase();

public:
  ~ImageFileSystemBase() override;

  Result<std::filesystem::path> makeAbsolute(
    const std::filesystem::path& path) const override;

  /**
   * Reload this file system.
   */
  Result<void> reload();

protected:
  void addFile(const std::filesystem::path& path, GetImageFile getFile);

  PathInfo pathInfo(const std::filesystem::path& path) const override;

private:
  Result<std::vector<std::filesystem::path>> doFind(
    const std::filesystem::path& path, TraversalMode traversalMode) const override;
  Result<std::shared_ptr<File>> doOpenFile(
    const std::filesystem::path& path) const override;

  virtual Result<void> doReadDirectory() = 0;
};

class ImageFileSystem : public ImageFileSystemBase
{
protected:
  std::shared_ptr<CFile> m_file;

public:
  explicit ImageFileSystem(std::shared_ptr<CFile> file);
};

template <typename T, typename... Args>
Result<std::unique_ptr<T>> createImageFileSystem(Args&&... args)
{
  auto fs = std::make_unique<T>(std::forward<Args>(args)...);
  return fs->reload().transform([&]() { return std::move(fs); });
}

} // namespace TrenchBroom::IO
