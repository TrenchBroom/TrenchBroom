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

#include "Exceptions.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/IOUtils.h"
#include "IO/PathQt.h"

#include <kdl/string_compare.h>

#include <fstream>
#include <string>

#include <QDir>
#include <QFileInfo>

namespace TrenchBroom
{
namespace IO
{
namespace Disk
{
bool doCheckCaseSensitive();
Path findCaseSensitivePath(const std::vector<Path>& list, const Path& path);
Path fixCase(const Path& path);

bool doCheckCaseSensitive()
{
  const QDir cwd = QDir::current();
  assert(cwd.exists());

  const QDir upper = QDir(cwd.path().toUpper());
  const QDir lower = QDir(cwd.path().toLower());
  return !upper.exists() || !lower.exists();
}

bool isCaseSensitive()
{
  static const bool caseSensitive = doCheckCaseSensitive();
  return caseSensitive;
}

Path findCaseSensitivePath(const std::vector<Path>& list, const Path& path)
{
  for (const Path& entry : list)
  {
    if (kdl::ci::str_is_equal(entry.asString(), path.asString()))
      return entry;
  }
  return Path("");
}

Path fixCase(const Path& path)
{
  try
  {
    if (!path.isAbsolute())
      throw FileSystemException(
        "Cannot fix case of relative path: '" + path.asString() + "'");

    if (path.isEmpty() || !isCaseSensitive())
      return path;
    if (QFileInfo::exists(pathAsQString(path)))
      return path;

    Path result(path.firstComponent());
    Path remainder(path.deleteFirstComponent());
    if (remainder.isEmpty())
      return result;

    while (!remainder.isEmpty())
    {
      const QString nextPathStr = pathAsQString(result + remainder.firstComponent());
      if (!QFileInfo::exists(nextPathStr))
      {
        const std::vector<Path> content = getDirectoryContents(result);
        const Path part = findCaseSensitivePath(content, remainder.firstComponent());
        if (part.isEmpty())
          return path;
        result = result + part;
      }
      else
      {
        result = result + remainder.firstComponent();
      }
      remainder = remainder.deleteFirstComponent();
    }
    return result;
  }
  catch (const PathException& e)
  {
    throw FileSystemException("Cannot fix case of path: '" + path.asString() + "'", e);
  }
}

Path fixPath(const Path& path)
{
  try
  {
    if (!path.isAbsolute())
      throw FileSystemException("Cannot fix relative path: '" + path.asString() + "'");
    return fixCase(path.makeCanonical());
  }
  catch (const PathException& e)
  {
    throw FileSystemException("Cannot fix path: '" + path.asString() + "'", e);
  }
}

bool directoryExists(const Path& path)
{
  const Path fixedPath = fixPath(path);
  return QDir(pathAsQString(fixedPath)).exists();
}

bool fileExists(const Path& path)
{
  const Path fixedPath = fixPath(path);
  QFileInfo fileInfo = QFileInfo(pathAsQString(fixedPath));
  return fileInfo.exists() && fileInfo.isFile();
}

std::vector<Path> getDirectoryContents(const Path& path)
{
  const Path fixedPath = fixPath(path);
  QDir dir(pathAsQString(fixedPath));
  if (!dir.exists())
  {
    throw FileSystemException("Cannot open directory: '" + fixedPath.asString() + "'");
  }

  dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

  std::vector<Path> result;
  for (QString& entry : dir.entryList())
  {
    result.push_back(pathFromQString(entry));
  }
  return result;
}

std::shared_ptr<File> openFile(const Path& path)
{
  const Path fixedPath = fixPath(path);
  if (!fileExists(fixedPath))
  {
    throw FileNotFoundException(fixedPath.asString());
  }

  return std::make_shared<CFile>(fixedPath);
}

std::string readTextFile(const Path& path)
{
  const Path fixedPath = fixPath(path);

  std::ifstream stream = openPathAsInputStream(fixedPath);
  if (!stream.is_open())
  {
    throw FileSystemException("Cannot open file: " + fixedPath.asString());
  }

  return std::string(
    (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

Path getCurrentWorkingDir()
{
  return pathFromQString(QDir::currentPath());
}

std::vector<Path> findItems(const Path& path)
{
  return findItems(path, FileTypeMatcher());
}

std::vector<Path> findItemsRecursively(const Path& path)
{
  return findItemsRecursively(path, FileTypeMatcher());
}

void createFile(const Path& path, const std::string& contents)
{
  const Path fixedPath = fixPath(path);
  if (fileExists(fixedPath))
  {
    deleteFile(fixedPath);
  }
  else
  {
    const Path directory = fixedPath.deleteLastComponent();
    if (!directoryExists(directory))
      createDirectory(directory);
  }

  std::ofstream stream = openPathAsOutputStream(fixedPath);
  stream << contents;
}

bool createDirectoryHelper(const Path& path);

void createDirectory(const Path& path)
{
  const Path fixedPath = fixPath(path);
  if (fileExists(fixedPath))
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString()
      + "': A file already exists at that path.");
  if (directoryExists(fixedPath))
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString()
      + "': A directory already exists at that path.");
  if (!createDirectoryHelper(fixedPath))
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString() + "'");
}

bool createDirectoryHelper(const Path& path)
{
  if (path.isEmpty())
    return false;
  const IO::Path parent = path.deleteLastComponent();
  if (!QDir(pathAsQString(parent)).exists() && !createDirectoryHelper(parent))
    return false;
  return QDir().mkdir(pathAsQString(path));
}

void ensureDirectoryExists(const Path& path)
{
  const Path fixedPath = fixPath(path);
  if (fileExists(fixedPath))
    throw FileSystemException(
      "Could not create directory '" + fixedPath.asString()
      + "': A file already exists at that path.");
  if (!directoryExists(fixedPath))
    createDirectoryHelper(fixedPath);
}

void deleteFile(const Path& path)
{
  const Path fixedPath = fixPath(path);
  if (!fileExists(fixedPath))
    throw FileSystemException(
      "Could not delete file '" + fixedPath.asString() + "': File does not exist.");
  if (!QFile::remove(pathAsQString(fixedPath)))
    throw FileSystemException("Could not delete file '" + path.asString() + "'");
}

void copyFile(const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  const Path fixedSourcePath = fixPath(sourcePath);
  Path fixedDestPath = fixPath(destPath);
  if (directoryExists(fixedDestPath))
  {
    fixedDestPath = fixedDestPath + sourcePath.lastComponent();
  }
  const bool exists = fileExists(fixedDestPath);
  if (!overwrite && exists)
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "': file already exists");
  if (overwrite && exists)
  {
    if (!QFile::remove(pathAsQString(fixedDestPath)))
    {
      throw FileSystemException(
        "Could not copy file '" + fixedSourcePath.asString() + "' to '"
        + fixedDestPath.asString() + "': couldn't remove destination");
    }
  }
  // NOTE: QFile::copy will not overwrite the dest
  if (!QFile::copy(pathAsQString(fixedSourcePath), pathAsQString(fixedDestPath)))
    throw FileSystemException(
      "Could not copy file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "'");
}

void moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  const Path fixedSourcePath = fixPath(sourcePath);
  Path fixedDestPath = fixPath(destPath);
  const bool exists = fileExists(fixedDestPath);
  if (!overwrite && exists)
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "': file already exists");
  if (overwrite && exists)
  {
    if (!QFile::remove(pathAsQString(fixedDestPath)))
    {
      throw FileSystemException(
        "Could not move file '" + fixedSourcePath.asString() + "' to '"
        + fixedDestPath.asString() + "': couldn't remove destination");
    }
  }
  if (directoryExists(fixedDestPath))
    fixedDestPath = fixedDestPath + sourcePath.lastComponent();
  if (!QFile::rename(pathAsQString(fixedSourcePath), pathAsQString(fixedDestPath)))
    throw FileSystemException(
      "Could not move file '" + fixedSourcePath.asString() + "' to '"
      + fixedDestPath.asString() + "'");
}

IO::Path resolvePath(const std::vector<Path>& searchPaths, const Path& path)
{
  if (path.isAbsolute())
  {
    if (fileExists(path) || directoryExists(path))
      return path;
  }
  else
  {
    for (const Path& searchPath : searchPaths)
    {
      if (searchPath.isAbsolute())
      {
        try
        {
          const Path fullPath = searchPath + path;
          if (fileExists(fullPath) || directoryExists(fullPath))
            return fullPath;
        }
        catch (const Exception&)
        {
        }
      }
    }
  }
  return Path("");
}
} // namespace Disk
} // namespace IO
} // namespace TrenchBroom
