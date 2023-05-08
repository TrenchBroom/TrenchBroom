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

#include "Exceptions.h"
#include "IO/File.h"
#include "IO/FileSystemUtils.h"
#include "IO/IOUtils.h"
#include "IO/PathInfo.h"
#include "IO/PathQt.h"

#include <kdl/string_compare.h>

#include <fstream>
#include <string>

namespace TrenchBroom::IO
{
namespace Disk
{

namespace
{

std::vector<Path> doGetDirectoryContents(const Path& fixedPath)
{
  auto dir = QDir{pathAsQString(fixedPath)};
  if (!dir.exists())
  {
    throw FileSystemException("Cannot open directory: '" + fixedPath.asString() + "'");
  }

  dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

  const auto entries = dir.entryList();
  auto result = std::vector<Path>{};
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

Path fixCase(const Path& path)
{
  if (
    path.isEmpty() || !path.isAbsolute() || !isCaseSensitive()
    || QFileInfo::exists(pathAsQString(path)))
  {
    return path;
  }

  auto result = path.hasDriveSpec() ? path.firstComponent() : Path{"/"};
  auto remainder =
    path.hasDriveSpec() ? path.deleteFirstComponent() : path.makeRelative();

  auto dir = QDir{};
  dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

  while (!remainder.isEmpty())
  {
    dir.setPath(pathAsQString(result));

    const auto curDirStr = pathAsQString(remainder.firstComponent());
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

    result = result + pathFromQString(*entryIt);
    remainder = remainder.deleteFirstComponent();
  }
  return result;
}

} // namespace

bool isCaseSensitive()
{
  static const bool caseSensitive = doCheckCaseSensitive();
  return caseSensitive;
}

Path fixPath(const Path& path)
{
  try
  {
    return fixCase(path.makeCanonical());
  }
  catch (const PathException&)
  {
    return path;
  }
}

PathInfo pathInfo(const Path& path)
{
  const auto fixedPath = pathAsQString(fixPath(path));
  const auto fileInfo = QFileInfo{fixedPath};
  return fileInfo.exists() && fileInfo.isFile() ? PathInfo::File
         : QDir{fixedPath}.exists()             ? PathInfo::Directory
                                                : PathInfo::Unknown;
}

std::vector<Path> find(const Path& path, const PathMatcher& pathMatcher)
{
  return IO::find(path, directoryContents, pathInfo, pathMatcher);
}

std::vector<Path> findRecursively(const Path& path, const PathMatcher& pathMatcher)
{
  return IO::findRecursively(path, directoryContents, pathInfo, pathMatcher);
}

std::vector<Path> directoryContents(const Path& path)
{
  return doGetDirectoryContents(fixPath(path));
}

std::shared_ptr<File> openFile(const Path& path)
{
  const auto fixedPath = fixPath(path);
  if (pathInfo(fixedPath) != PathInfo::File)
  {
    throw FileNotFoundException(fixedPath.asString());
  }

  return std::make_shared<CFile>(fixedPath);
}

std::string readTextFile(const Path& path)
{
  const auto fixedPath = fixPath(path);

  auto stream = openPathAsInputStream(fixedPath);
  if (!stream.is_open())
  {
    throw FileSystemException("Cannot open file: " + fixedPath.asString());
  }

  return std::string{
    (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>()};
}

Path getCurrentWorkingDir()
{
  return pathFromQString(QDir::currentPath());
}

void createFile(const Path& path, const std::string& contents)
{
  const auto fixedPath = fixPath(path);
  if (pathInfo(fixedPath) == PathInfo::File)
  {
    deleteFile(fixedPath);
  }
  else
  {
    const auto directory = fixedPath.deleteLastComponent();
    if (pathInfo(directory) == PathInfo::Unknown)
    {
      createDirectory(directory);
    }
  }

  auto stream = openPathAsOutputStream(fixedPath);
  stream << contents;
}

namespace
{
bool createDirectoryHelper(const Path& path)
{
  if (path.isEmpty())
  {
    return false;
  }

  const auto parent = path.deleteLastComponent();
  if (!QDir{pathAsQString(parent)}.exists() && !createDirectoryHelper(parent))
  {
    return false;
  }

  return QDir{}.mkdir(pathAsQString(path));
}
} // namespace

void createDirectory(const Path& path)
{
  const auto fixedPath = fixPath(path);
  switch (pathInfo(fixedPath))
  {
  case PathInfo::Directory:
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString()
      + "': A directory already exists at that path.");
  case PathInfo::File:
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString()
      + "': A file already exists at that path.");
  case PathInfo::Unknown:
    if (!createDirectoryHelper(fixedPath))
    {
      throw FileSystemException(
        "Could not create directory '" + fixedPath.asString() + "'");
    }
    break;
  }
}

void ensureDirectoryExists(const Path& path)
{
  const auto fixedPath = fixPath(path);
  switch (pathInfo(fixedPath))
  {
  case PathInfo::Directory:
    break;
  case PathInfo::File:
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString()
      + "': A file already exists at that path.");
  case PathInfo::Unknown:
    if (!createDirectoryHelper(fixedPath))
    {
      throw FileSystemException(
        "Could not create directory '" + fixedPath.asString() + "'");
    }
    break;
  }
}

void deleteFile(const Path& path)
{
  const auto fixedPath = fixPath(path);
  if (pathInfo(fixedPath) != PathInfo::File)
  {
    throw FileSystemException(
      "Could not delete file '" + fixedPath.asString() + "': File does not exist.");
  }

  if (!QFile::remove(pathAsQString(fixedPath)))
  {
    throw FileSystemException("Could not delete file '" + path.asString() + "'");
  }
}

void deleteFiles(const Path& sourceDirPath, const PathMatcher& pathMatcher)
{
  for (const auto& filePath : find(sourceDirPath, pathMatcher))
  {
    deleteFile(filePath);
  }
}

void deleteFilesRecursively(const Path& sourceDirPath, const PathMatcher& pathMatcher)
{
  for (const auto& filePath : findRecursively(sourceDirPath, pathMatcher))
  {
    deleteFile(filePath);
  }
}

void copyFile(const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  auto fixedDestPath = fixPath(destPath);

  if (pathInfo(fixedDestPath) == PathInfo::Directory)
  {
    fixedDestPath = fixedDestPath + sourcePath.lastComponent();
  }

  const auto exists = pathInfo(fixedDestPath) == PathInfo::File;
  if (!overwrite && exists)
  {
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "': file already exists");
  }

  if (overwrite && exists && !QFile::remove(pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "': couldn't remove destination");
  }

  // NOTE: QFile::copy will not overwrite the dest
  if (!QFile::copy(pathAsQString(fixedSourcePath), pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "'");
  }
}

void copyFiles(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  const bool overwrite)
{
  for (const auto& sourceFilePath : find(sourceDirPath, pathMatcher))
  {
    copyFile(sourceFilePath, destDirPath, overwrite);
  }
}

void copyFilesRecursively(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  const bool overwrite)
{
  for (const auto& sourceFilePath : findRecursively(sourceDirPath, pathMatcher))
  {
    copyFile(sourceFilePath, destDirPath, overwrite);
  }
}

void moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  auto fixedDestPath = fixPath(destPath);

  const auto exists = pathInfo(fixedDestPath) == PathInfo::File;
  if (!overwrite && exists)
  {
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "': file already exists");
  }

  if (overwrite && exists && !QFile::remove(pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "': couldn't remove destination");
  }

  if (pathInfo(fixedDestPath) == PathInfo::Directory)
  {
    fixedDestPath = fixedDestPath + sourcePath.lastComponent();
  }

  if (!QFile::rename(pathAsQString(fixedSourcePath), pathAsQString(fixedDestPath)))
  {
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "'");
  }
}

void moveFiles(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  const bool overwrite)
{
  for (const auto& sourceFilePath : find(sourceDirPath, pathMatcher))
  {
    moveFile(sourceFilePath, destDirPath, overwrite);
  }
}

void moveFilesRecursively(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  const bool overwrite)
{
  for (const auto& sourceFilePath : findRecursively(sourceDirPath, pathMatcher))
  {
    moveFile(sourceFilePath, destDirPath, overwrite);
  }
}

Path resolvePath(const std::vector<Path>& searchPaths, const Path& path)
{
  if (path.isAbsolute())
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
      if (searchPath.isAbsolute())
      {
        try
        {
          const auto fullPath = searchPath + path;
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
  return Path{};
}

} // namespace Disk
} // namespace TrenchBroom::IO
