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

#include "Exceptions.h"
#include "IO/FileSystemError.h"
#include "IO/PathMatcher.h"

#include <kdl/result.h>

#include <filesystem>
#include <fstream>
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

template <typename Stream, typename F>
auto withStream(
  const std::filesystem::path& path, const std::ios::openmode mode, const F& function)
  -> kdl::wrap_result_t<decltype(function(std::declval<Stream&>())), FileSystemError>
{
  using FnResultType = decltype(function(std::declval<Stream&>()));
  using ResultType = kdl::wrap_result_t<FnResultType, FileSystemError>;
  try
  {
    auto stream = Stream{path, mode};
    if (!stream)
    {
      return ResultType{
        FileSystemError{"Could not open stream for file '" + path.string() + "'"}};
    }
    if constexpr (kdl::is_result_v<FnResultType>)
    {
      if constexpr (std::is_same_v<typename FnResultType::value_type, void>)
      {
        return function(stream).and_then([]() { return ResultType{}; });
      }
      else
      {
        return function(stream).and_then([](auto x) { return ResultType{std::move(x)}; });
      }
    }
    else if constexpr (std::is_same_v<typename ResultType::value_type, void>)
    {
      function(stream);
      return ResultType{};
    }
    else
    {
      return ResultType{function(stream)};
    }
  }
  catch (const std::filesystem::filesystem_error& e)
  {
    return ResultType{FileSystemError{
      "Could not open stream for file '" + path.string() + "': " + e.what()}};
  }
}

template <typename F>
auto withInputStream(
  const std::filesystem::path& path, const std::ios::openmode mode, const F& function)
{
  return withStream<std::ifstream>(path, mode, function);
}

template <typename F>
auto withInputStream(const std::filesystem::path& path, const F& function)
{
  return withStream<std::ifstream>(path, std::ios_base::in, function);
}

template <typename F>
auto withOutputStream(
  const std::filesystem::path& path, const std::ios::openmode mode, const F& function)
{
  return withStream<std::ofstream>(path, mode, function);
}

template <typename F>
auto withOutputStream(const std::filesystem::path& path, const F& function)
{
  return withStream<std::ofstream>(path, std::ios_base::out, function);
}

kdl::result<bool, FileSystemError> createDirectory(const std::filesystem::path& path);

void deleteFile(const std::filesystem::path& path);

void copyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);

void moveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath);

std::filesystem::path resolvePath(
  const std::vector<std::filesystem::path>& searchPaths,
  const std::filesystem::path& path);

} // namespace Disk
} // namespace TrenchBroom::IO
