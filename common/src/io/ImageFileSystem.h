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

#pragma once

#include "Ensure.h"
#include "Result.h"
#include "io/FileSystem.h"
#include "io/FileSystemMetadata.h"

#include "kdl/path_hash.h"
#include "kdl/result.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>
#include <variant>

namespace tb::io
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
  std::unordered_map<std::filesystem::path, size_t, kdl::path_hash> entryMapLC;
};

class ImageFileSystemBase : public FileSystem
{
protected:
  ImageEntry m_root;
  std::unordered_map<std::string, FileSystemMetadata> m_metadata;

  ImageFileSystemBase();

public:
  ~ImageFileSystemBase() override;

  Result<std::filesystem::path> makeAbsolute(
    const std::filesystem::path& path) const override;

  /**
   * Reload this file system.
   */
  Result<void> reload();

  void setMetadata(std::unordered_map<std::string, FileSystemMetadata> metadata);

protected:
  void addFile(const std::filesystem::path& path, GetImageFile getFile);

  PathInfo pathInfo(const std::filesystem::path& path) const override;

  const FileSystemMetadata* metadata(
    const std::filesystem::path& path, const std::string& key) const override;

private:
  Result<std::vector<std::filesystem::path>> doFind(
    const std::filesystem::path& path, const TraversalMode& traversalMode) const override;
  Result<std::shared_ptr<File>> doOpenFile(
    const std::filesystem::path& path) const override;

  virtual Result<void> doReadDirectory() = 0;
};

template <typename FileType>
class ImageFileSystem : public ImageFileSystemBase
{
protected:
  std::shared_ptr<FileType> m_file;

public:
  explicit ImageFileSystem(std::shared_ptr<FileType> file)
    : m_file{std::move(file)}
  {
    ensure(m_file, "file must not be null");
  }
};

std::unordered_map<std::string, FileSystemMetadata> makeImageFileSystemMetadata(
  std::filesystem::path imageFilePath);

template <typename T, typename... Args>
Result<std::unique_ptr<T>> createImageFileSystem(Args&&... args)
{
  auto fs = std::make_unique<T>(std::forward<Args>(args)...);
  return fs->reload() | kdl::transform([&]() { return std::move(fs); });
}

} // namespace tb::io
