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

#include "Exceptions.h"
#include "IO/PathMatcher.h"

#include <kdl/result_forward.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

class File;
class FileSystem;
struct FileSystemError;
enum class PathInfo;

class FileSystem
{
public:
  virtual ~FileSystem();

  /** Return an absolute path for the given relative path.
   *
   * @return an absolute path or an error if the given path is already an absolute path or
   * if the given path is invalid
   */
  virtual kdl::result<std::filesystem::path, FileSystemError> makeAbsolute(
    const std::filesystem::path& path) const = 0;

  /** Indicates whether the given path denotes a file, a directory, or is unknown.
   */
  virtual PathInfo pathInfo(const std::filesystem::path& path) const = 0;

  /** Returns a vector of paths listing the contents of the directory  at the given path
   * that satisfy the given path matcher. The returned paths are relative to the root of
   * this file system.
   *
   * @param path the path to the directory to search
   * @param pathMatcher only return paths that satisfy this path matcher
   */
  std::vector<std::filesystem::path> find(
    const std::filesystem::path& path,
    const PathMatcher& pathMatcher = matchAnyPath) const;

  /** Returns a vector of paths listing the contents of the directory recursively at the
   * given path that satisfy the given path matcher. The returned paths are relative to
   * the root of this file system.
   *
   * @param path the path to the directory to search
   * @param pathMatcher only return paths that satisfy this path matcher
   */
  std::vector<std::filesystem::path> findRecursively(
    const std::filesystem::path& path,
    const PathMatcher& pathMatcher =
      [](const std::filesystem::path&, const GetPathInfo&) { return true; }) const;

  /** Returns the contents of the directory at the given paths.
   *
   * Returns an empty list if there is no directory at the given path. The returned paths
   * are relative to the given path.
   *
   * @param path the path to the directory to list
   * @throws FileSystemException if the given path is an absolute path or if the given
   * path is invalid
   */
  std::vector<std::filesystem::path> directoryContents(
    const std::filesystem::path& path) const;

  /** Open a file at the given path and return it.
   *
   * @throws FileSystemException if the given path is absolute, if it is invalid invalid
   * or if it does not denote a file
   */
  std::shared_ptr<File> openFile(const std::filesystem::path& path) const;

protected:
  virtual std::vector<std::filesystem::path> doGetDirectoryContents(
    const std::filesystem::path& path) const = 0;
  virtual std::shared_ptr<File> doOpenFile(const std::filesystem::path& path) const = 0;
};

class WritableFileSystem : public virtual FileSystem
{
public:
  ~WritableFileSystem() override;

  kdl::result<void, FileSystemError> createFileAtomic(
    const std::filesystem::path& path, const std::string& contents);
  kdl::result<void, FileSystemError> createFile(
    const std::filesystem::path& path, const std::string& contents);
  kdl::result<bool, FileSystemError> createDirectory(const std::filesystem::path& path);
  kdl::result<bool, FileSystemError> deleteFile(const std::filesystem::path& path);
  kdl::result<void, FileSystemError> copyFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);
  void moveFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);

private:
  virtual kdl::result<void, FileSystemError> doCreateFile(
    const std::filesystem::path& path, const std::string& contents) = 0;
  virtual kdl::result<bool, FileSystemError> doCreateDirectory(
    const std::filesystem::path& path) = 0;
  virtual kdl::result<bool, FileSystemError> doDeleteFile(
    const std::filesystem::path& path) = 0;
  virtual kdl::result<void, FileSystemError> doCopyFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
  virtual void doMoveFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
};

} // namespace TrenchBroom::IO
