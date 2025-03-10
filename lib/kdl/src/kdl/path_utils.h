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

#include <algorithm>
#include <filesystem>
#include <string>

namespace kdl
{

template <typename char_type>
std::filesystem::path parse_path(
  std::basic_string<char_type> str, const bool convert_separators = true)
{
  if (convert_separators)
  {
    constexpr auto pref_sep = char_type(std::filesystem::path::preferred_separator);
    const auto win_sep = char_type('\\');
    const auto x_sep = char_type('/');
    std::ranges::replace_if(
      str, [&](const auto c) { return c == win_sep || c == x_sep; }, pref_sep);
  }
  return std::filesystem::path{std::move(str)};
}

size_t path_length(const std::filesystem::path& path);

bool path_has_prefix(
  const std::filesystem::path& path, const std::filesystem::path& prefix);

std::filesystem::path path_front(const std::filesystem::path& path);

std::filesystem::path path_to_lower(const std::filesystem::path& path);

std::filesystem::path path_clip(
  const std::filesystem::path& path, size_t index, size_t length);

std::filesystem::path path_clip(const std::filesystem::path& path, size_t index);

std::filesystem::path path_pop_front(const std::filesystem::path& path);

bool path_has_extension(
  const std::filesystem::path& path, std::filesystem::path extension);

bool path_has_extension(
  const std::filesystem::path& path, std::filesystem::path extension);

std::filesystem::path path_add_extension(
  std::filesystem::path path, const std::filesystem::path& extension);

std::filesystem::path path_remove_extension(std::filesystem::path path);

std::filesystem::path path_replace_extension(
  std::filesystem::path path, const std::filesystem::path& extension);

} // namespace kdl
