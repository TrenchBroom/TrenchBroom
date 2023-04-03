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

#include "IO/Path.h"
#include "IO/PathMatcher.h"

#include <memory>
#include <string>

namespace TrenchBroom::IO
{
class File;
enum class PathInfo;

namespace Disk
{
bool isCaseSensitive();

Path fixPath(const Path& path);

PathInfo pathInfo(const Path& path);
std::vector<Path> find(
  const Path& path,
  const PathMatcher& pathMatcher = [](const Path&, const GetPathInfo&) { return true; });
std::vector<Path> findRecursively(
  const Path& path,
  const PathMatcher& pathMatcher = [](const Path&, const GetPathInfo&) { return true; });
std::vector<Path> directoryContents(const Path& path);
std::shared_ptr<File> openFile(const Path& path);

std::string readTextFile(const Path& path);
Path getCurrentWorkingDir();

void createFile(const Path& path, const std::string& contents);
void createDirectory(const Path& path);
void ensureDirectoryExists(const Path& path);

void deleteFile(const Path& path);
void deleteFiles(const Path& sourceDirPath, const PathMatcher& pathMatcher);
void deleteFilesRecursively(const Path& sourceDirPath, const PathMatcher& pathMatcher);

void copyFile(const Path& sourcePath, const Path& destPath, bool overwrite);
void copyFiles(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  bool overwrite);
void copyFilesRecursively(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  bool overwrite);

void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite);
void moveFiles(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  bool overwrite);
void moveFilesRecursively(
  const Path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const Path& destDirPath,
  bool overwrite);

Path resolvePath(const std::vector<Path>& searchPaths, const Path& path);

} // namespace Disk
} // namespace TrenchBroom::IO
