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

#include "DiskIO.h"

#include <QDir>
#include <QFileInfo>

#include "IO/File.h"
#include "IO/FileSystemUtils.h"
#include "IO/IOUtils.h"
#include "IO/PathInfo.h"
#include "IO/PathQt.h"

#include <kdl/path_utils.h>
#include <kdl/string_compare.h>

#include <fstream>
#include <string>

namespace TrenchBroom::IO
{
namespace Disk
{

namespace
{

std::vector<std::filesystem::path> doGetDirectoryContents(
  const std::filesystem::path& fixedPath)
{
  auto dir = QDir{pathAsQString(fixedPath)};
  if (!dir.exists())
  {
    throw FileSystemException("Cannot open directory: '" + fixedPath.string() + "'");
  }

  dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

  const auto entries = dir.entryList();
  auto result = std::vector<std::filesystem::path>{};
  result.reserve(size_t(entries.size()));

  std::transform(
    entries.begin(), entries.end(), std::back_inserter(result), pathFromQString);

  return result;
}

bool doCheckCaseSensitive()
{
  const auto cwd = QDir::current();
  assert(cwd.exists());

  const auto upper = QDir{cwd.path().toUpper()};
  const auto lower = QDir{cwd.path().toLower()};
  return !upper.exists() || !lower.exists();
}

std::filesystem::path fixCase(const std::filesystem::path& path)
{
  if (
    path.empty() || !path.is_absolute() || !isCaseSensitive()
    || QFileInfo::exists(pathAsQString(path)))
  {
    return path;
  }

  auto result = kdl::path_front(path);
  auto remainder = kdl::path_pop_front(path);

  auto dir = QDir{};
  dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

  while (!remainder.empty())
  {
    dir.setPath(pathAsQString(result));

    const auto curDirStr = pathAsQString(kdl::path_front(remainder));
    const auto entries = dir.entryList();
    const auto entryIt = std::find_if(entries.begin(), entries.end(), [&](const auto& s) {
      const auto ss = pathFromQString(s);
      const auto c = s.compare(curDirStr, Qt::CaseInsensitive);
      return c == 0;
    });

    if (entryIt == entries.end())
    {
      return path;
    }

    result = result / pathFromQString(*entryIt);
    remainder = kdl::path_pop_front(remainder);
  }
  return result;
}

} // namespace

bool isCaseSensitive()
{
  static const bool caseSensitive = doCheckCaseSensitive();
  return caseSensitive;
}

std::filesystem::path fixPath(const std::filesystem::path& path)
{
  return fixCase(path.lexically_normal());
}

PathInfo pathInfo(const std::filesystem::path& path)
{
  const auto fixedPath = pathAsQString(fixPath(path));
  const auto fileInfo = QFileInfo{fixedPath};
  return fileInfo.exists() && fileInfo.isFile() ? PathInfo::File
         : QDir{fixedPath}.exists()             ? PathInfo::Directory
                                                : PathInfo::Unknown;
}

std::vector<std::filesystem::path> find(
  const std::filesystem::path& path, const PathMatcher& pathMatcher)
{
  return find(path, directoryContents, pathInfo, pathMatcher);
}

std::vector<std::filesystem::path> findRecursively(
  const std::filesystem::path& path, const PathMatcher& pathMatcher)
{
  return findRecursively(path, directoryContents, pathInfo, pathMatcher);
}

std::vector<std::filesystem::path> directoryContents(const std::filesystem::path& path)
{
  return doGetDirectoryContents(fixPath(path));
}

std::shared_ptr<File> openFile(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  if (pathInfo(fixedPath) != PathInfo::File)
  {
    throw FileNotFoundException(fixedPath.string());
  }

  return std::make_shared<CFile>(fixedPath);
}

namespace
{
bool createDirectoryHelper(const std::filesystem::path& path)
{
  if (path.empty())
  {
    return false;
  }

  const auto parent = path.parent_path();
  if (!QDir{pathAsQString(parent)}.exists() && !createDirectoryHelper(parent))
  {
    return false;
  }

  return QDir{}.mkdir(pathAsQString(path));
}
} // namespace

bool createDirectory(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  switch (pathInfo(fixedPath))
  {
  case PathInfo::Directory:
    break;
  case PathInfo::File:
    throw FileSystemException(
      "Could not create directory '" + fixedPath.string()
      + "': A file already exists at that path.");
  case PathInfo::Unknown:
    if (createDirectoryHelper(fixedPath))
    {
      return true;
    }
    throw FileSystemException("Could not create directory '" + fixedPath.string() + "'");
  }
  return false;
}

void deleteFile(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  if (pathInfo(fixedPath) != PathInfo::File)
  {
    throw FileSystemException(
      "Could not delete file '" + fixedPath.string() + "': File does not exist.");
  }

  if (!QFile::remove(pathAsQString(fixedPath)))
  {
    throw FileSystemException("Could not delete file '" + path.string() + "'");
  }
}

void copyFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfo(fixedSourcePath) != PathInfo::File)
  {
    throw FileSystemException{
      "Could not copy '" + fixedSourcePath.string() + "': not a file"};
  }

  auto fixedDestPath = fixPath(destPath);

  if (pathInfo(fixedDestPath) == PathInfo::Directory)
  {
    fixedDestPath = fixedDestPath / sourcePath.filename();
  }

  const auto exists = pathInfo(fixedDestPath) == PathInfo::File;
  if (!overwrite && exists)
  {
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.string() + "' to '"
      + fixedDestPath.string() + "': file already exists");
  }

  if (overwrite && exists && !QFile::remove(pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.string() + "' to '"
      + fixedDestPath.string() + "': couldn't remove destination");
  }

  // NOTE: QFile::copy will not overwrite the dest
  if (!QFile::copy(pathAsQString(fixedSourcePath), pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.string() + "' to '"
      + fixedDestPath.string() + "'");
  }
}

void moveFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfo(fixedSourcePath) != PathInfo::File)
  {
    throw FileSystemException{
      "Could not move '" + fixedSourcePath.string() + "': not a file"};
  }

  auto fixedDestPath = fixPath(destPath);

  const auto exists = pathInfo(fixedDestPath) == PathInfo::File;
  if (!overwrite && exists)
  {
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.string() + "' to '"
      + fixedDestPath.string() + "': file already exists");
  }

  if (overwrite && exists && !QFile::remove(pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.string() + "' to '"
      + fixedDestPath.string() + "': couldn't remove destination");
  }

  if (pathInfo(fixedDestPath) == PathInfo::Directory)
  {
    fixedDestPath = fixedDestPath / sourcePath.filename();
  }

  if (!QFile::rename(pathAsQString(fixedSourcePath), pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.string() + "' to '"
      + fixedDestPath.string() + "'");
  }
}

std::filesystem::path resolvePath(
  const std::vector<std::filesystem::path>& searchPaths,
  const std::filesystem::path& path)
{
  if (path.is_absolute())
  {
    if (pathInfo(path) != PathInfo::Unknown)
    {
      return path;
    }
  }
  else
  {
    for (const auto& searchPath : searchPaths)
    {
      if (searchPath.is_absolute())
      {
        try
        {
          const auto fullPath = searchPath / path;
          if (pathInfo(fullPath) != PathInfo::Unknown)
          {
            return fullPath;
          }
        }
        catch (const Exception&)
        {
        }
      }
    }
  }
  return {};
}

} // namespace Disk
} // namespace TrenchBroom::IO
