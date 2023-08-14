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

namespace TrenchBroom
{
struct Error;
}

namespace TrenchBroom::IO
{
class File;
class FileSystem;
enum class PathInfo;
enum class TraversalMode;

class FileSystem
{
public:
  virtual ~FileSystem();

  /** Return an absolute path for the given relative path.
   *
   * @return an absolute path or an error if the given path is already an absolute path or
   * if the given path is invalid
   */
  virtual kdl::result<std::filesystem::path, Error> makeAbsolute(
    const std::filesystem::path& path) const = 0;

  /** Indicates whether the given path denotes a file, a directory, or is unknown.
   */
  virtual PathInfo pathInfo(const std::filesystem::path& path) const = 0;

  /** Returns a vector of paths listing the contents of the directory  at the given path
   * that satisfy the given path matcher. The returned paths are relative to the root of
   * this file system.
   *
   * @param path the path to the directory to search
   * @param traversalMode the traversal mode
   * @param pathMatcher only return paths that satisfy this path matcher
   */
  kdl::result<std::vector<std::filesystem::path>, Error> find(
    const std::filesystem::path& path,
    TraversalMode traversalMode,
    const PathMatcher& pathMatcher = matchAnyPath) const;

  /** Open a file at the given path and return it.
   */
  kdl::result<std::shared_ptr<File>, Error> openFile(
    const std::filesystem::path& path) const;

protected:
  virtual kdl::result<std::vector<std::filesystem::path>, Error> doFind(
    const std::filesystem::path& path, TraversalMode traversalMode) const = 0;
  virtual kdl::result<std::shared_ptr<File>, Error> doOpenFile(
    const std::filesystem::path& path) const = 0;
};

class WritableFileSystem : public virtual FileSystem
{
public:
  ~WritableFileSystem() override;

  kdl::result<void, Error> createFileAtomic(
    const std::filesystem::path& path, const std::string& contents);
  kdl::result<void, Error> createFile(
    const std::filesystem::path& path, const std::string& contents);
  kdl::result<bool, Error> createDirectory(const std::filesystem::path& path);
  kdl::result<bool, Error> deleteFile(const std::filesystem::path& path);
  kdl::result<void, Error> copyFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);
  kdl::result<void, Error> moveFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);

private:
  virtual kdl::result<void, Error> doCreateFile(
    const std::filesystem::path& path, const std::string& contents) = 0;
  virtual kdl::result<bool, Error> doCreateDirectory(
    const std::filesystem::path& path) = 0;
  virtual kdl::result<bool, Error> doDeleteFile(const std::filesystem::path& path) = 0;
  virtual kdl::result<void, Error> doCopyFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
  virtual kdl::result<void, Error> doMoveFile(
    const std::filesystem::path& sourcePath, const std::filesystem::path& destPath) = 0;
};

} // namespace TrenchBroom::IO
