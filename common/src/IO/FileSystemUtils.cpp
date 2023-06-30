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

#include "FileSystemUtils.h"

#include "Exceptions.h"
#include "IO/PathInfo.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom::IO
{

std::optional<std::filesystem::path> safeMakeAbsolute(
  const std::filesystem::path& path, const MakeAbsolute& makeAbsolute)
{
  try
  {
    return makeAbsolute(path);
  }
  catch (FileSystemException&)
  {
    return std::nullopt;
  }
}

namespace
{
std::vector<std::filesystem::path> doFind(
  const std::filesystem::path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher,
  const bool recursive)
{
  if (getPathInfo(path) != PathInfo::Directory)
  {
    throw FileSystemException{"Directory not found: '" + path.string() + "'"};
  }

  auto result = kdl::vec_transform(
    getDirectoryContents(path), [&](const auto& p) { return path / p; });

  if (recursive)
  {
    size_t i = 0;
    while (i < result.size())
    {
      const auto& currentPath = result[i];
      if (getPathInfo(currentPath) == PathInfo::Directory)
      {
        result = kdl::vec_concat(
          std::move(result),
          kdl::vec_transform(getDirectoryContents(currentPath), [&](const auto& p) {
            return currentPath / p;
          }));
      }
      ++i;
    }
  }

  return kdl::vec_filter(
    std::move(result), [&](const auto& p) { return pathMatcher(p, getPathInfo); });
}
} // namespace

std::vector<std::filesystem::path> find(
  const std::filesystem::path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher)
{
  return doFind(path, getDirectoryContents, getPathInfo, pathMatcher, false);
}

std::vector<std::filesystem::path> findRecursively(
  const std::filesystem::path& path,
  const GetDirectoryContents& getDirectoryContents,
  const GetPathInfo& getPathInfo,
  const PathMatcher& pathMatcher)
{
  return doFind(path, getDirectoryContents, getPathInfo, pathMatcher, true);
}

} // namespace TrenchBroom::IO
