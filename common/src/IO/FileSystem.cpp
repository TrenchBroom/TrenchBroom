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
#include "IO/FileSystemUtils.h"
#include "IO/PathInfo.h"
#include "Macros.h"

#include <kdl/path_utils.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <ostream>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

FileSystem::~FileSystem() = default;

std::filesystem::path FileSystem::makeAbsolute(const std::filesystem::path& path) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  return doMakeAbsolute(path);
}

PathInfo FileSystem::pathInfo(const std::filesystem::path& path) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  return doGetPathInfo(path);
}

std::vector<std::filesystem::path> FileSystem::find(
  const std::filesystem::path& path, const PathMatcher& pathMatcher) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  return IO::find(
    path,
    [&](const std::filesystem::path& p) { return doGetDirectoryContents(p); },
    [&](const std::filesystem::path& p) { return doGetPathInfo(p); },
    pathMatcher);
}

std::vector<std::filesystem::path> FileSystem::findRecursively(
  const std::filesystem::path& path, const PathMatcher& pathMatcher) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  return IO::findRecursively(
    path,
    [&](const std::filesystem::path& p) { return doGetDirectoryContents(p); },
    [&](const std::filesystem::path& p) { return doGetPathInfo(p); },
    pathMatcher);
}

std::vector<std::filesystem::path> FileSystem::directoryContents(
  const std::filesystem::path& path) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  if (doGetPathInfo(path) != PathInfo::Directory)
  {
    throw FileSystemException{"Directory not found: '" + path.string() + "'"};
  }

  return doGetDirectoryContents(path);
}

std::shared_ptr<File> FileSystem::openFile(const std::filesystem::path& path) const
{
  if (path.is_absolute())
  {
    throw FileSystemException{"Path is absolute: '" + path.string() + "'"};
  }

  if (doGetPathInfo(path) != PathInfo::File)
  {
    throw FileSystemException{"File not found: '" + path.string() + "'"};
  }

  return doOpenFile(path);
}

WritableFileSystem::~WritableFileSystem() = default;

void WritableFileSystem::createFileAtomic(
  const std::filesystem::path& path, const std::string& contents)
{
  if (path.is_absolute())
  {
    throw FileSystemException("Path is absolute: '" + path.string() + "'");
  }

  const auto tmpPath = kdl::path_add_extension(path, "tmp");
  doCreateFile(tmpPath, contents);
  doMoveFile(tmpPath, path, true);
}

void WritableFileSystem::createFile(
  const std::filesystem::path& path, const std::string& contents)
{
  if (path.is_absolute())
  {
    throw FileSystemException("Path is absolute: '" + path.string() + "'");
  }
  doCreateFile(path, contents);
}

void WritableFileSystem::createDirectory(const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    throw FileSystemException("Path is absolute: '" + path.string() + "'");
  }
  doCreateDirectory(path);
}

void WritableFileSystem::deleteFile(const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    throw FileSystemException("Path is absolute: '" + path.string() + "'");
  }
  doDeleteFile(path);
}

void WritableFileSystem::copyFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  if (sourcePath.is_absolute())
  {
    throw FileSystemException("Source path is absolute: '" + sourcePath.string() + "'");
  }
  if (destPath.is_absolute())
  {
    throw FileSystemException(
      "Destination path is absolute: '" + destPath.string() + "'");
  }
  doCopyFile(sourcePath, destPath, overwrite);
}

void WritableFileSystem::moveFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  if (sourcePath.is_absolute())
  {
    throw FileSystemException("Source path is absolute: '" + sourcePath.string() + "'");
  }
  if (destPath.is_absolute())
  {
    throw FileSystemException(
      "Destination path is absolute: '" + destPath.string() + "'");
  }
  doMoveFile(sourcePath, destPath, overwrite);
}
} // namespace TrenchBroom::IO
