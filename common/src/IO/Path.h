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

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom
{
namespace IO
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

  template <typename StringLess>
  class Less
  {
  private:
    StringLess m_less;

  public:
    bool operator()(const Path& lhs, const Path& rhs) const
    {
      return std::lexicographical_compare(
        std::begin(lhs.m_components),
        std::end(lhs.m_components),
        std::begin(rhs.m_components),
        std::end(rhs.m_components),
        m_less);
    }
  };

private:
  std::vector<std::string> m_components;
  bool m_absolute;

  Path(bool absolute, const std::vector<std::string>& components);

public:
  explicit Path(const std::string& path = "");

  Path operator+(const Path& rhs) const;
  int compare(const Path& rhs, bool caseSensitive = true) const;
  bool operator==(const Path& rhs) const;
  bool operator!=(const Path& rhs) const;
  bool operator<(const Path& rhs) const;
  bool operator>(const Path& rhs) const;

  std::string asString(std::string_view sep = separator()) const;
  static std::vector<std::string> asStrings(
    const std::vector<Path>& paths, std::string_view sep = separator());
  static std::vector<Path> asPaths(const std::vector<std::string>& strs);

  size_t length() const;
  bool isEmpty() const;
  Path firstComponent() const;
  Path deleteFirstComponent() const;
  Path lastComponent() const;
  Path deleteLastComponent() const;
  Path prefix(size_t count) const;
  Path suffix(size_t count) const;
  Path subPath(size_t index, size_t count) const;
  const std::vector<std::string>& components() const;

  std::string filename() const;
  std::string basename() const;
  std::string extension() const;

  bool hasPrefix(const Path& prefix, bool caseSensitive) const;
  bool hasFilename(const std::string& filename, bool caseSensitive) const;
  bool hasFilename(const std::vector<std::string>& filenames, bool caseSensitive) const;
  bool hasBasename(const std::string& basename, bool caseSensitive) const;
  bool hasBasename(const std::vector<std::string>& basenames, bool caseSensitive) const;
  bool hasExtension(const std::string& extension, bool caseSensitive) const;
  bool hasExtension(const std::vector<std::string>& extensions, bool caseSensitive) const;
  bool hasDriveSpec() const;

  Path deleteExtension() const;
  Path addExtension(const std::string& extension) const;
  Path replaceExtension(const std::string& extension) const;

  Path replaceBasename(const std::string& basename) const;

  bool isAbsolute() const;
  bool canMakeRelative(const Path& absolutePath) const;
  Path makeAbsolute(const Path& relativePath) const;

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

  static std::vector<Path> makeAbsoluteAndCanonical(
    const std::vector<Path>& paths, const Path& relativePath);

private:
  static bool hasDriveSpec(const std::vector<std::string>& components);
  static bool hasDriveSpec(const std::string& component);
  std::vector<std::string> resolvePath(
    bool absolute, const std::vector<std::string>& components) const;
};

std::ostream& operator<<(std::ostream& stream, const Path& path);
} // namespace IO
} // namespace TrenchBroom
