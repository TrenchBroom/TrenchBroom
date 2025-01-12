/*
 Copyright (C) 2010 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "kdl/string_format.h"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <string>

namespace kdl
{

inline std::filesystem::path parse_path(
  std::string str, const bool replace_backslashes = true)
{
  if (replace_backslashes)
  {
    std::ranges::replace_if(str, [](char c) { return c == '\\'; }, '/');
  }
  return std::filesystem::path{std::move(str)};
}

inline size_t path_length(const std::filesystem::path& path)
{
  return size_t(std::distance(path.begin(), path.end()));
}

inline bool path_has_prefix(
  const std::filesystem::path& path, const std::filesystem::path& prefix)
{
  const auto [i_path, i_prefix] =
    std::mismatch(path.begin(), path.end(), prefix.begin(), prefix.end());
  return i_prefix == prefix.end();
}

inline std::filesystem::path path_front(const std::filesystem::path& path)
{
  return path.begin() == path.end() ? std::filesystem::path{} : *path.begin();
}

inline std::filesystem::path path_to_lower(const std::filesystem::path& path)
{
  return std::filesystem::path{str_to_lower(path.string())};
}

inline std::filesystem::path path_clip(
  const std::filesystem::path& path, const size_t index, size_t length)
{
  using difftype =
    typename std::iterator_traits<std::filesystem::path::const_iterator>::difference_type;

  if (index >= path_length(path))
  {
    return {};
  }

  length = std::min(length, path_length(path) - index);

  return std::accumulate(
    std::next(path.begin(), difftype(index)),
    std::next(path.begin(), difftype(index + length)),
    std::filesystem::path{},
    [](auto result, const auto& component) { return result /= component; });
}

inline std::filesystem::path path_clip(
  const std::filesystem::path& path, const size_t index)
{
  return path_clip(path, index, path_length(path));
}

inline std::filesystem::path path_pop_front(const std::filesystem::path& path)
{
  return path_clip(path, 1, path_length(path));
}

inline std::filesystem::path path_add_extension(
  std::filesystem::path path, const std::filesystem::path& extension)
{
  return path += extension;
}

inline std::filesystem::path path_remove_extension(std::filesystem::path path)
{
  return path.replace_extension();
}

inline std::filesystem::path path_replace_extension(
  std::filesystem::path path, const std::filesystem::path& extension)
{
  return path.replace_extension(extension);
}

} // namespace kdl
