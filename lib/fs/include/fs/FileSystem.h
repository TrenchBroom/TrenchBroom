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

#include "Result.h"
#include "io/File.h"
#include "io/FileSystemMetadata.h"
#include "io/PathMatcher.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace tb::io
{
class FileSystem;
enum class PathInfo;
struct TraversalMode;

class FileSystem
{
public:
  virtual ~FileSystem();

  /** Return an absolute path for the given relative path.
   *
   * @return an absolute path or an error if the given path is already an absolute path or
   * if the given path is invalid
   */
  virtual Result<std::filesystem::path> makeAbsolute(
    const std::filesystem::path& path) const = 0;

  /** Indicates whether the given path denotes a file, a directory, or is unknown.
   */
  virtual PathInfo pathInfo(const std::filesystem::path& path) const = 0;

  /** Returns the meta data associated with the given path and key, or null if no metadata
   * is associated with the given path and key.
   */
  virtual const FileSystemMetadata* metadata(
    const std::filesystem::path& path, const std::string& key) const = 0;

  /** Returns a vector of paths listing the contents of the directory  at the given path
   * that satisfy the given path matcher. The returned paths are relative to the root of
   * this file system.
   *
   * @param path the path to the directory to search
   * @param traversalMode the traversal mode
   * @param pathMatcher only return paths that satisfy this path matcher
   */
  Result<std::vector<std::filesystem::path>> find(
    const std::filesystem::path& path,
    const TraversalMode& traversalMode,
    const PathMatcher& pathMatcher = matchAnyPath) const;

  /** Open a file at the given path and return it.
   */
  Result<std::shared_ptr<File>> openFile(const std::filesystem::path& path) const;

protected:
  virtual Result<std::vector<std::filesystem::path>> doFind(
    const std::filesystem::path& path, const TraversalMode& traversalMode) const = 0;
  virtual Result<std::shared_ptr<File>> doOpenFile(
    const std::filesystem::path& path) const = 0;
};

class WritableFileSystem : public virtual FileSystem
{
public:
  ~WritableFileSystem() override;

  Result<void> createFileAtomic(
    const std::filesystem::path& path, const std::string& contents);
  Result<void> createFile(const std::filesystem::path& path, const std::string& contents);
  Result<bool> createDirectory(const std::filesystem::path& path);
  Result<bool> deleteFile(const std::filesystem::path& path);
  Result<void> copyFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);
  Result<void> moveFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);
  Result<void> renameDirectory(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);

private:
  virtual Result<void> doCreateFile(
    const std::filesystem::path& path, const std::string& contents) = 0;
  virtual Result<bool> doCreateDirectory(const std::filesystem::path& path) = 0;
  virtual Result<bool> doDeleteFile(const std::filesystem::path& path) = 0;
  virtual Result<void> doCopyFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
  virtual Result<void> doMoveFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
  virtual Result<void> doRenameDirectory(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
};

} // namespace tb::io
