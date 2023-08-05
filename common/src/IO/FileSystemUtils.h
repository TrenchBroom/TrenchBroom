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
#include <functional>
#include <optional>
#include <vector>

namespace TrenchBroom::IO
{

enum class PathInfo;

using GetDirectoryContents =
  std::function<std::vector<std::filesystem::path>(const std::filesystem::path&)>;

/** Returns a vector of paths listing the contents of the directory  at the given path
 * that satisfy the given path matcher. The returned paths are relative to the root of
 * this file system.
 *
 * @param path the path to the directory to search
 * @param pathMatcher only return paths that satisfy this path matcher
 */
std::vector<std::filesystem::path> find(
  const std::filesystem::path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher = matchAnyPath);

/** Returns a vector of paths listing the contents of the directory recursively at the
 * given path that satisfy the given path matcher. The returned paths are relative to
 * the root of this file system.
 *
 * @param path the path to the directory to search
 * @param pathMatcher only return paths that satisfy this path matcher
 */
std::vector<std::filesystem::path> findRecursively(
  const std::filesystem::path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher = matchAnyPath);

} // namespace TrenchBroom::IO
