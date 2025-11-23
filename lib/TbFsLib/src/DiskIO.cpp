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

#include "fs/DiskIO.h"

#include "Macros.h"
#include "fs/File.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"

#include "kd/contracts.h"
#include "kd/path_utils.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

namespace tb::fs::Disk
{
namespace
{

bool doCheckCaseSensitive()
{
  const auto cwd = std::filesystem::current_path();
  contract_assert(std::filesystem::is_directory(cwd));

  return !std::filesystem::exists(kdl::path_to_lower(cwd))
         || !std::filesystem::exists(kdl::str_to_upper(cwd.string()));
}

std::filesystem::path fixCase(const std::filesystem::path& path)
{
  try
  {
    if (
      path.empty() || !path.is_absolute() || !isCaseSensitive()
      || std::filesystem::exists(path))
    {
      return path;
    }

    auto result = kdl::path_front(kdl::path_to_lower(path));
    auto remainder = kdl::path_pop_front(kdl::path_to_lower(path));

    while (!remainder.empty())
    {
      const auto nameToFind = kdl::path_front(remainder);
      const auto entryIt = std::find_if(
        std::filesystem::directory_iterator{result},
        std::filesystem::directory_iterator{},
        [&](const auto& entry) {
          return nameToFind == kdl::path_to_lower(entry.path().filename());
        });

      if (entryIt == std::filesystem::directory_iterator{})
      {
        return path;
      }

      result = result / entryIt->path().filename();
      remainder = kdl::path_pop_front(remainder);
    }
    return result;
  }
  catch (const std::filesystem::filesystem_error&)
  {
    return path;
  }
}

PathInfo pathInfoForFixedPath(const std::filesystem::path& fixedPath)
{
  auto error = std::error_code{};
  return std::filesystem::is_directory(fixedPath, error) && !error ? PathInfo::Directory
         : std::filesystem::is_regular_file(fixedPath, error) && !error
           ? PathInfo::File
           : PathInfo::Unknown;
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
  return pathInfoForFixedPath(fixPath(path));
}

Result<std::vector<std::filesystem::path>> find(
  const std::filesystem::path& path,
  const TraversalMode& traversalMode,
  const PathMatcher& pathMatcher)
{
  const auto fixedPath = fixPath(path);
  if (pathInfoForFixedPath(fixedPath) != PathInfo::Directory)
  {
    return Error{
      fmt::format("Failed to open {}: path does not denote a directory", path)};
  }

  auto error = std::error_code{};
  auto result = std::vector<std::filesystem::path>{};

  auto it = std::filesystem::recursive_directory_iterator{
    fixedPath, std::filesystem::directory_options::follow_directory_symlink, error};
  auto end = std::filesystem::recursive_directory_iterator{};

  while (it != end)
  {
    contract_assert(!traversalMode.depth || it.depth() <= traversalMode.depth);

    const auto& entryPath = it->path();
    if (pathMatcher(entryPath, pathInfo))
    {
      result.push_back(entryPath);
    }

    if (traversalMode.depth && it.depth() == int(*traversalMode.depth))
    {
      it.disable_recursion_pending();
    }

    ++it;
  }

  if (error)
  {
    return Error{fmt::format("Failed to open {}: {}", path, error.message())};
  }

  return result;
}

Result<std::shared_ptr<CFile>> openFile(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  if (pathInfoForFixedPath(fixedPath) != PathInfo::File)
  {
    return Error{fmt::format("Failed to open {}: path does not denote a file", path)};
  }

  return createCFile(fixedPath);
}

Result<bool> createDirectory(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  switch (pathInfoForFixedPath(fixedPath))
  {
  case PathInfo::Directory:
    return false;
  case PathInfo::File:
    return Error{fmt::format("Failed to create {}: path denotes a file", path)};
  case PathInfo::Unknown: {
    auto error = std::error_code{};
    const auto created = std::filesystem::create_directories(fixedPath, error);
    if (!error)
    {
      return created;
    }
    return Error{fmt::format("Failed to create {}: {}", path, error.message())};
  }
    switchDefault();
  }
}

Result<bool> deleteFile(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  switch (pathInfoForFixedPath(fixedPath))
  {
  case PathInfo::Directory:
    return Error{fmt::format("Failed to delete {}: path denotes a directory", path)};
  case PathInfo::File: {
    auto error = std::error_code{};
    if (std::filesystem::remove(fixedPath, error) && !error)
    {
      return true;
    }
    if (error)
    {
      return Error{fmt::format("Failed to delete {}: {}", path, error.message())};
    }
    return false;
  }
  case PathInfo::Unknown:
    return false;
    switchDefault();
  }
}

Result<void> copyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfoForFixedPath(fixedSourcePath) != PathInfo::File)
  {
    return Error{
      fmt::format("Failed to copy {}: path does not denote a file", sourcePath)};
  }

  auto fixedDestPath = fixPath(destPath);
  if (pathInfoForFixedPath(fixedDestPath) == PathInfo::Directory)
  {
    fixedDestPath = fixedDestPath / sourcePath.filename();
  }

  auto error = std::error_code{};
  if (
    !std::filesystem::copy_file(
      fixedSourcePath,
      fixedDestPath,
      std::filesystem::copy_options::overwrite_existing,
      error)
    || error)
  {
    return Error{
      fmt::format("Failed to copy {} to {}: {}", sourcePath, destPath, error.message())};
  }

  return kdl::void_success;
}

Result<void> moveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfoForFixedPath(fixedSourcePath) != PathInfo::File)
  {
    return Error{
      fmt::format("Failed to move {}: path does not denote a file", sourcePath)};
  }

  auto fixedDestPath = fixPath(destPath);
  if (pathInfoForFixedPath(fixedDestPath) == PathInfo::Directory)
  {
    fixedDestPath = fixedDestPath / sourcePath.filename();
  }

  auto error = std::error_code{};
  std::filesystem::rename(fixedSourcePath, fixedDestPath, error);
  if (error)
  {
    return Error{
      fmt::format("Failed to move {} to {}: {}", sourcePath, destPath, error.message())};
  }

  return kdl::void_success;
}

Result<void> renameDirectory(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfoForFixedPath(fixedSourcePath) != PathInfo::Directory)
  {
    return Error{
      fmt::format("Failed to rename {}: path does not denote a directory", sourcePath)};
  }

  auto fixedDestPath = fixPath(destPath);
  if (pathInfoForFixedPath(fixedDestPath) != PathInfo::Unknown)
  {
    return Error{fmt::format(
      "Failed to rename {} to {}: target path already exists", sourcePath, destPath)};
  }

  auto error = std::error_code{};
  std::filesystem::rename(fixedSourcePath, fixedDestPath, error);
  if (error)
  {
    return Error{fmt::format(
      "Failed to rename {} to {}: {}", sourcePath, destPath, error.message())};
  }

  return kdl::void_success;
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
        const auto fullPath = searchPath / path;
        if (pathInfo(fullPath) != PathInfo::Unknown)
        {
          return fullPath;
        }
      }
    }
  }
  return {};
}

Result<std::filesystem::path> makeUniqueFilename(
  const std::filesystem::path& directoryPath)
{
  auto filename = kdl::str_make_random(32);

  auto ec = std::error_code{};
  while (std::filesystem::exists(directoryPath / filename, ec) && !ec)
  {
    filename = kdl::str_make_random(32);
  }

  if (ec)
  {
    return Error{fmt::format(
      "Failed to generate a unique filename at '{}': {}", directoryPath, ec.message())};
  }

  return filename;
}

} // namespace tb::fs::Disk
