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

  std::string asString() const;
  std::string asGenericString() const;

  size_t length() const;
  bool isEmpty() const;
  Path firstComponent() const;
  Path deleteFirstComponent() const;
  Path lastComponent() const;
  Path deleteLastComponent() const;
  Path prefix(size_t count) const;
  Path suffix(size_t count) const;
  Path subPath(size_t index, size_t count) const;

  Path filename() const;
  Path basename() const;
  Path extension() const;

  bool hasPrefix(const Path& prefix, bool caseSensitive) const;

  Path deleteExtension() const;
  Path addExtension(const std::string& extension) const;

  bool isAbsolute() const;

  /**
   * Return a relative path if this path is absolute. On Windows, this means that the
   * returned path has no drive specification (i.e. 'C:\'), and on other systems, this
   * means that the returned path will not have a leading forward slash anymore.
   *
   * @return the relative path
   */
  Path makeRelative() const;
  Path makeRelative(const Path& absolutePath) const;
  Path makeCanonical() const;
  Path makeLowerCase() const;
};

std::ostream& operator<<(std::ostream& stream, const Path& path);
} // namespace TrenchBroom::IO
