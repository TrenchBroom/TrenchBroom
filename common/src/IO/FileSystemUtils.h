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

#include <functional>
#include <optional>
#include <vector>

namespace TrenchBroom::IO
{

class Path;
enum class PathInfo;

using GetDirectoryContents = std::function<std::vector<Path>(const Path&)>;
using MakeAbsolute = std::function<Path(const Path&)>;

std::optional<Path> safeMakeAbsolute(const Path& path, const MakeAbsolute& makeAbsolute);

/** Returns a vector of paths listing the contents of the directory  at the given path
 * that satisfy the given path matcher. The returned paths are relative to the root of
 * this file system.
 *
 * @param path the path to the directory to search
 * @param pathMatcher only return paths that satisfy this path matcher
 */
std::vector<Path> find(
  const Path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher = [](const Path&, const GetPathInfo&) { return true; });

/** Returns a vector of paths listing the contents of the directory recursively at the
 * given path that satisfy the given path matcher. The returned paths are relative to
 * the root of this file system.
 *
 * @param path the path to the directory to search
 * @param pathMatcher only return paths that satisfy this path matcher
 */
std::vector<Path> findRecursively(
  const Path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher = [](const Path&, const GetPathInfo&) { return true; });

} // namespace TrenchBroom::IO
