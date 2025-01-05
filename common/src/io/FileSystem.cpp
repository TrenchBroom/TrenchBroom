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

#include "FileSystem.h"

#include "io/PathInfo.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <string>
#include <vector>

namespace tb::io
{

FileSystem::~FileSystem() = default;

Result<std::vector<std::filesystem::path>> FileSystem::find(
  const std::filesystem::path& path,
  const TraversalMode& traversalMode,
  const PathMatcher& pathMatcher) const
{
  if (path.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(path))};
  }

  if (pathInfo(path) != PathInfo::Directory)
  {
    return Error{fmt::format("Path {} does not denote a directory", fmt::streamed(path))};
  }

  return doFind(path, traversalMode) | kdl::transform([&](auto paths) {
           return kdl::vec_filter(std::move(paths), [&](const auto& p) {
             return pathMatcher(p, [&](const auto& x) { return pathInfo(x); });
           });
         });
}

Result<std::shared_ptr<File>> FileSystem::openFile(
  const std::filesystem::path& path) const
{
  if (path.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(path))};
  }

  if (pathInfo(path) != PathInfo::File)
  {
    return Error{fmt::format("{} not found", fmt::streamed(path))};
  }

  return doOpenFile(path);
}

WritableFileSystem::~WritableFileSystem() = default;

Result<void> WritableFileSystem::createFileAtomic(
  const std::filesystem::path& path, const std::string& contents)
{
  if (path.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(path))};
  }

  const auto tmpPath = kdl::path_add_extension(path, "tmp");
  return doCreateFile(tmpPath, contents)
         | kdl::and_then([&]() { return doMoveFile(tmpPath, path); });
}

Result<void> WritableFileSystem::createFile(
  const std::filesystem::path& path, const std::string& contents)
{
  if (path.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(path))};
  }
  return doCreateFile(path, contents);
}

Result<bool> WritableFileSystem::createDirectory(const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(path))};
  }
  return doCreateDirectory(path);
}

Result<bool> WritableFileSystem::deleteFile(const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(path))};
  }
  return doDeleteFile(path);
}

Result<void> WritableFileSystem::copyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  if (sourcePath.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(sourcePath))};
  }
  if (destPath.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(destPath))};
  }
  return doCopyFile(sourcePath, destPath);
}

Result<void> WritableFileSystem::moveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  if (sourcePath.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(sourcePath))};
  }
  if (destPath.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(destPath))};
  }
  return doMoveFile(sourcePath, destPath);
}

Result<void> WritableFileSystem::renameDirectory(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  if (sourcePath.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(sourcePath))};
  }
  if (destPath.is_absolute())
  {
    return Error{fmt::format("Path {} is absolute", fmt::streamed(destPath))};
  }
  return doRenameDirectory(sourcePath, destPath);
}

} // namespace tb::io
