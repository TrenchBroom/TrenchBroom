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

#include "FileSystem.h"

#include "Exceptions.h"
#include "IO/FileSystemError.h"
#include "IO/PathInfo.h"
#include "Macros.h"

#include <kdl/path_utils.h>
#include <kdl/result.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <ostream>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

FileSystem::~FileSystem() = default;

std::vector<std::filesystem::path> FileSystem::find(
  const std::filesystem::path& path,
  const TraversalMode traversalMode,
  const PathMatcher& pathMatcher) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  if (pathInfo(path) != PathInfo::Directory)
  {
    throw FileSystemException{
      "Path does not denoty a directory: '" + path.string() + "'"};
  }

  return kdl::vec_filter(doFind(path, traversalMode), [&](const auto& p) {
    return pathMatcher(p, [&](const auto& x) { return pathInfo(x); });
  });
}

kdl::result<std::shared_ptr<File>, FileSystemError> FileSystem::openFile(
  const std::filesystem::path& path) const
{
  if (path.is_absolute())
  {
    return FileSystemError{"Path is absolute: '" + path.string() + "'"};
  }

  if (pathInfo(path) != PathInfo::File)
  {
    return FileSystemError{"File not found: '" + path.string() + "'"};
  }

  return doOpenFile(path);
}

WritableFileSystem::~WritableFileSystem() = default;

kdl::result<void, FileSystemError> WritableFileSystem::createFileAtomic(
  const std::filesystem::path& path, const std::string& contents)
{
  if (path.is_absolute())
  {
    return FileSystemError{"Path is absolute: '" + path.string() + "'"};
  }

  const auto tmpPath = kdl::path_add_extension(path, "tmp");
  return doCreateFile(tmpPath, contents).and_then([&]() {
    return doMoveFile(tmpPath, path);
  });
}

kdl::result<void, FileSystemError> WritableFileSystem::createFile(
  const std::filesystem::path& path, const std::string& contents)
{
  if (path.is_absolute())
  {
    return FileSystemError{"Path is absolute: '" + path.string() + "'"};
  }
  return doCreateFile(path, contents);
}

kdl::result<bool, FileSystemError> WritableFileSystem::createDirectory(
  const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    return FileSystemError{"Path is absolute: '" + path.string() + "'"};
  }
  return doCreateDirectory(path);
}

kdl::result<bool, FileSystemError> WritableFileSystem::deleteFile(
  const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    return FileSystemError{"Path is absolute: '" + path.string() + "'"};
  }
  return doDeleteFile(path);
}

kdl::result<void, FileSystemError> WritableFileSystem::copyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  if (sourcePath.is_absolute())
  {
    return FileSystemError{"Source path is absolute: '" + sourcePath.string() + "'"};
  }
  if (destPath.is_absolute())
  {
    return FileSystemError{"Destination path is absolute: '" + destPath.string() + "'"};
  }
  return doCopyFile(sourcePath, destPath);
}

kdl::result<void, FileSystemError> WritableFileSystem::moveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  if (sourcePath.is_absolute())
  {
    return FileSystemError{"Source path is absolute: '" + sourcePath.string() + "'"};
  }
  if (destPath.is_absolute())
  {
    return FileSystemError{"Destination path is absolute: '" + destPath.string() + "'"};
  }
  return doMoveFile(sourcePath, destPath);
}
} // namespace TrenchBroom::IO
