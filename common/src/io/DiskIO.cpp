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

#include "DiskIO.h"

#include "Exceptions.h"
#include "Macros.h"
#include "io/File.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"

#include "kdl/path_utils.h"
#include "kdl/string_format.h"

namespace tb::io::Disk
{
namespace
{

bool doCheckCaseSensitive()
{
  const auto cwd = std::filesystem::current_path();
  assert(std::filesystem::is_directory(cwd));

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
    return Error{fmt::format(
      "Failed to open {}: path does not denote a directory", fmt::streamed(path))};
  }

  auto error = std::error_code{};
  auto result = std::vector<std::filesystem::path>{};

  auto it = std::filesystem::recursive_directory_iterator{
    fixedPath, std::filesystem::directory_options::follow_directory_symlink, error};
  auto end = std::filesystem::recursive_directory_iterator{};

  while (it != end)
  {
    assert(!traversalMode.depth || it.depth() <= traversalMode.depth);

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
    return Error{
      fmt::format("Failed to open {}: {}", fmt::streamed(path), error.message())};
  }

  return result;
}

Result<std::shared_ptr<CFile>> openFile(const std::filesystem::path& path)
{
  const auto fixedPath = fixPath(path);
  if (pathInfoForFixedPath(fixedPath) != PathInfo::File)
  {
    return Error{
      fmt::format("Failed to open {}: path does not denote a file", fmt::streamed(path))};
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
    return Error{
      fmt::format("Failed to create {}: path denotes a file", fmt::streamed(path))};
  case PathInfo::Unknown: {
    auto error = std::error_code{};
    const auto created = std::filesystem::create_directories(fixedPath, error);
    if (!error)
    {
      return created;
    }
    return Error{
      fmt::format("Failed to create {}: {}", fmt::streamed(path), error.message())};
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
    return Error{
      fmt::format("Failed to delete {}: path denotes a directory", fmt::streamed(path))};
  case PathInfo::File: {
    auto error = std::error_code{};
    if (std::filesystem::remove(fixedPath, error) && !error)
    {
      return true;
    }
    if (error)
    {
      return Error{
        fmt::format("Failed to delete {}: {}", fmt::streamed(path), error.message())};
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
    return Error{fmt::format(
      "Failed to copy {}: path does not denote a file", fmt::streamed(sourcePath))};
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
    return Error{fmt::format(
      "Failed to copy {} to {}: {}",
      fmt::streamed(sourcePath),
      fmt::streamed(destPath),
      error.message())};
  }

  return kdl::void_success;
}

Result<void> moveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfoForFixedPath(fixedSourcePath) != PathInfo::File)
  {
    return Error{fmt::format(
      "Failed to move {}: path does not denote a file", fmt::streamed(sourcePath))};
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
    return Error{fmt::format(
      "Failed to move {} to {}: {}",
      fmt::streamed(sourcePath),
      fmt::streamed(destPath),
      error.message())};
  }

  return kdl::void_success;
}

Result<void> renameDirectory(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  const auto fixedSourcePath = fixPath(sourcePath);
  if (pathInfoForFixedPath(fixedSourcePath) != PathInfo::Directory)
  {
    return Error{fmt::format(
      "Failed to rename {}: path does not denote a directory",
      fmt::streamed(sourcePath))};
  }

  auto fixedDestPath = fixPath(destPath);
  if (pathInfoForFixedPath(fixedDestPath) != PathInfo::Unknown)
  {
    return Error{fmt::format(
      "Failed to rename {} to {}: target path already exists",
      fmt::streamed(sourcePath),
      fmt::streamed(destPath))};
  }

  auto error = std::error_code{};
  std::filesystem::rename(fixedSourcePath, fixedDestPath, error);
  if (error)
  {
    return Error{fmt::format(
      "Failed to rename {} to {}: {}",
      fmt::streamed(sourcePath),
      fmt::streamed(destPath),
      error.message())};
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

} // namespace tb::io::Disk
