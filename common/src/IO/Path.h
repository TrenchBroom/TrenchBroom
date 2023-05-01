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

#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom::IO
{
class Path
{
public:
  static constexpr std::string_view separator()
  {
#ifdef _WIN32
    return std::string_view("\\");
#else
    return std::string_view("/");
#endif
  }

private:
  std::filesystem::path m_path;

public:
  explicit Path(std::filesystem::path path = {});

  Path operator/(const Path& rhs) const;
  int compare(const Path& rhs) const;
  bool operator==(const Path& rhs) const;
  bool operator!=(const Path& rhs) const;
  bool operator<(const Path& rhs) const;
  bool operator>(const Path& rhs) const;

  std::string string() const;
  std::string generic_string() const;

  size_t hidden_length() const;
  bool empty() const;
  Path parent_path() const;
  Path hidden_front() const;
  Path hidden_pop_front() const;
  Path hidden_clip(size_t index, size_t count) const;

  Path filename() const;
  Path stem() const;
  Path extension() const;

  bool hidden_hasPrefix(const Path& prefix) const;

  Path hidden_addExtension(const std::string& extension) const;
  Path hidden_removeExtension() const;
  Path hidden_replaceExtension(const std::string& extension) const;

  bool is_absolute() const;

  /**
   * Return a relative path if this path is absolute. On Windows, this means that the
   * returned path has no drive specification (i.e. 'C:\'), and on other systems, this
   * means that the returned path will not have a leading forward slash anymore.
   *
   * @return the relative path
   */
  Path relative_path() const;
  Path lexically_relative(const Path& basePath) const;
  Path makeCanonical() const;
  Path hidden_makeLowerCase() const;
};

std::ostream& operator<<(std::ostream& stream, const Path& path);
} // namespace TrenchBroom::IO

namespace kdl
{
using TrenchBroom::IO::Path;

Path path_front(const Path& path);
Path path_pop_front(const Path& path);

size_t path_length(const Path& path);
bool path_has_prefix(const Path& path, const Path& prefix);
Path path_to_lower(const Path& path);
Path path_clip(const Path& path, size_t index, size_t length);
Path path_clip(const Path& path, size_t index);
Path path_add_extension(const Path& path, const std::string& extension);
Path path_remove_extension(Path path);
Path path_replace_extension(Path path, const std::string& extension);
} // namespace kdl
