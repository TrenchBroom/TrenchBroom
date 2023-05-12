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

#include "IO/PathMatcher.h"

#include <filesystem>
#include <memory>
#include <string>

namespace TrenchBroom::IO
{
class File;
enum class PathInfo;

namespace Disk
{
bool isCaseSensitive();

std::filesystem::path fixPath(const std::filesystem::path& path);

PathInfo pathInfo(const std::filesystem::path& path);

std::vector<std::filesystem::path> find(
  const std::filesystem::path& path, const PathMatcher& pathMatcher = matchAnyPath);
std::vector<std::filesystem::path> findRecursively(
  const std::filesystem::path& path, const PathMatcher& pathMatcher = matchAnyPath);
std::vector<std::filesystem::path> directoryContents(const std::filesystem::path& path);

std::shared_ptr<File> openFile(const std::filesystem::path& path);

std::filesystem::path getCurrentWorkingDir();

void createFile(const std::filesystem::path& path, const std::string& contents);
void createDirectory(const std::filesystem::path& path);
void ensureDirectoryExists(const std::filesystem::path& path);

void deleteFile(const std::filesystem::path& path);
void deleteFiles(
  const std::filesystem::path& sourceDirPath, const PathMatcher& pathMatcher);
void deleteFilesRecursively(
  const std::filesystem::path& sourceDirPath, const PathMatcher& pathMatcher);

void copyFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  bool overwrite);
void copyFiles(
  const std::filesystem::path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const std::filesystem::path& destDirPath,
  bool overwrite);
void copyFilesRecursively(
  const std::filesystem::path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const std::filesystem::path& destDirPath,
  bool overwrite);

void moveFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  bool overwrite);
void moveFiles(
  const std::filesystem::path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const std::filesystem::path& destDirPath,
  bool overwrite);
void moveFilesRecursively(
  const std::filesystem::path& sourceDirPath,
  const PathMatcher& pathMatcher,
  const std::filesystem::path& destDirPath,
  bool overwrite);

std::filesystem::path resolvePath(
  const std::vector<std::filesystem::path>& searchPaths,
  const std::filesystem::path& path);

} // namespace Disk
} // namespace TrenchBroom::IO
