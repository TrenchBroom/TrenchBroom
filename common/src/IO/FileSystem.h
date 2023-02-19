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

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

class File;
class FileSystem;
class Path;
enum class PathInfo;

class FileSystem
{
public:
  virtual ~FileSystem();

  /** Return an absolute path for the given relative path.
   *
   * @return an absolute path
   * @throws FileSystemException if the given path is already an absolute path or if the
   * given path is invalid
   */
  Path makeAbsolute(const Path& path) const;

  /** Indicates whether the given path denotes a file, a directory, or is unknown.
   */
  PathInfo pathInfo(const Path& path) const;

  /** Returns a vector of paths listing the contents of the directory  at the given path
   * that satisfy the given path matcher. The returned paths are relative to the root of
   * this file system.
   *
   * @param path the path to the directory to search
   * @param pathMatcher only return paths that satisfy this path matcher
   */
  std::vector<Path> find(
    const Path& path,
    const PathMatcher& pathMatcher = [](const Path&, const GetPathInfo&) {
      return true;
    }) const;

  /** Returns a vector of paths listing the contents of the directory recursively at the
   * given path that satisfy the given path matcher. The returned paths are relative to
   * the root of this file system.
   *
   * @param path the path to the directory to search
   * @param pathMatcher only return paths that satisfy this path matcher
   */
  std::vector<Path> findRecursively(
    const Path& path,
    const PathMatcher& pathMatcher = [](const Path&, const GetPathInfo&) {
      return true;
    }) const;

  /** Returns the contents of the directory at the given paths.
   *
   * Returns an empty list if there is no directory at the given path. The returned paths
   * are relative to the given path.
   *
   * @param path the path to the directory to list
   * @throws FileSystemException if the given path is an absolute path or if the given
   * path is invalid
   */
  std::vector<Path> directoryContents(const Path& path) const;

  /** Open a file at the given path and return it.
   *
   * @throws FileSystemException if the given path is absolute, if it is invalid invalid
   * or if it does not denote a file
   */
  std::shared_ptr<File> openFile(const Path& path) const;

protected:
  virtual Path doMakeAbsolute(const Path& path) const = 0;
  virtual PathInfo doGetPathInfo(const Path& path) const = 0;
  virtual std::vector<Path> doGetDirectoryContents(const Path& path) const = 0;
  virtual std::shared_ptr<File> doOpenFile(const Path& path) const = 0;
};

class WritableFileSystem : public virtual FileSystem
{
public:
  ~WritableFileSystem() override;

  void createFileAtomic(const Path& path, const std::string& contents);
  void createFile(const Path& path, const std::string& contents);
  void createDirectory(const Path& path);
  void deleteFile(const Path& path);
  void copyFile(const Path& sourcePath, const Path& destPath, bool overwrite);
  void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite);

private:
  virtual void doCreateFile(const Path& path, const std::string& contents) = 0;
  virtual void doCreateDirectory(const Path& path) = 0;
  virtual void doDeleteFile(const Path& path) = 0;
  virtual void doCopyFile(
    const Path& sourcePath, const Path& destPath, bool overwrite) = 0;
  virtual void doMoveFile(
    const Path& sourcePath, const Path& destPath, bool overwrite) = 0;
};

} // namespace TrenchBroom::IO
