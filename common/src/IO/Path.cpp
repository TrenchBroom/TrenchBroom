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

#include "Path.h"

#include "Exceptions.h"
#include "Macros.h"

#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <iterator>
#include <numeric>
#include <ostream>
#include <string>

namespace TrenchBroom::IO
{

Path::Path(std::filesystem::path path)
  : m_path{std::move(path.make_preferred())}
{
}

Path Path::operator/(const Path& rhs) const
{
  return Path{m_path / rhs.m_path};
}

int Path::compare(const Path& rhs) const
{
  return m_path.compare(rhs.m_path);
}

bool Path::operator==(const Path& rhs) const
{
  return compare(rhs) == 0;
}

bool Path::operator!=(const Path& rhs) const
{
  return !(*this == rhs);
}

bool Path::operator<(const Path& rhs) const
{
  return compare(rhs) < 0;
}

bool Path::operator>(const Path& rhs) const
{
  return compare(rhs) > 0;
}

std::string Path::asString() const
{
  return m_path.u8string();
}

std::string Path::asGenericString() const
{
  return m_path.generic_u8string();
}

std::vector<std::string> Path::asStrings(const std::vector<Path>& paths)
{
  return kdl::vec_transform(paths, [](const auto& path) { return path.asString(); });
}

std::vector<Path> Path::asPaths(const std::vector<std::string>& strs)
{
  return kdl::vec_transform(strs, [](const auto& str) { return Path{str}; });
}

size_t Path::length() const
{
  return size_t(std::distance(m_path.begin(), m_path.end()));
}

bool Path::isEmpty() const
{
  return m_path.empty();
}

Path Path::firstComponent() const
{
  return isEmpty() ? Path{} : Path{*m_path.begin()};
}

Path Path::deleteFirstComponent() const
{
  return isEmpty() ? *this : subPath(1, length() - 1);
}

Path Path::lastComponent() const
{
  return isEmpty() ? Path{} : Path{*std::prev(m_path.end())};
}

Path Path::deleteLastComponent() const
{
  return isEmpty() ? Path{} : Path{m_path.parent_path()};
}

Path Path::prefix(const size_t count) const
{
  return subPath(0, count);
}

Path Path::suffix(const size_t count) const
{
  return subPath(length() - count, count);
}

Path Path::subPath(const size_t index, size_t count) const
{
  count = std::min(count, length() >= count ? length() - index : 0);

  if (count == 0)
  {
    return Path{};
  }

  return Path{std::accumulate(
    std::next(m_path.begin(), index),
    std::next(m_path.begin(), index + count),
    std::filesystem::path{},
    [](const auto& lhs, const auto& rhs) { return lhs / rhs; })};
}

std::vector<std::string> Path::components() const
{
  auto result = std::vector<std::string>{};
  result.reserve(length());

  std::transform(
    m_path.begin(), m_path.end(), std::back_inserter(result), [](const auto& component) {
      return component.u8string();
    });
  return result;
}

std::string Path::filename() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot get filename of empty path"};
  }

  return m_path.filename().u8string();
}

std::string Path::basename() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot get basename of empty path"};
  }

  return m_path.stem().u8string();
}

std::string Path::extension() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot get extension of empty path"};
  }

  const auto extension = m_path.extension().u8string();
  return extension.empty() ? extension : extension.substr(1);
}

bool Path::hasPrefix(const Path& prefix, bool caseSensitive) const
{
  if (prefix.length() > length())
  {
    return false;
  }

  const auto mPrefix = this->prefix(prefix.length());
  return !caseSensitive ? mPrefix.makeLowerCase() == prefix.makeLowerCase()
                        : mPrefix == prefix;
}

bool Path::hasFilename(const std::string& filename, const bool caseSensitive) const
{
  if (caseSensitive)
  {
    return filename == this->filename();
  }
  else
  {
    return kdl::ci::str_is_equal(filename, this->filename());
  }
}

bool Path::hasFilename(
  const std::vector<std::string>& filenames, const bool caseSensitive) const
{
  for (const auto& filename : filenames)
  {
    if (hasFilename(filename, caseSensitive))
    {
      return true;
    }
  }
  return false;
}

bool Path::hasBasename(const std::string& basename, const bool caseSensitive) const
{
  if (caseSensitive)
  {
    return basename == this->basename();
  }
  else
  {
    return kdl::ci::str_is_equal(basename, this->basename());
  }
}

bool Path::hasBasename(
  const std::vector<std::string>& basenames, const bool caseSensitive) const
{
  for (const auto& basename : basenames)
  {
    if (hasBasename(basename, caseSensitive))
    {
      return true;
    }
  }
  return false;
}

bool Path::hasExtension(const std::string& extension, const bool caseSensitive) const
{
  if (caseSensitive)
  {
    return extension == this->extension();
  }
  else
  {
    return kdl::ci::str_is_equal(extension, this->extension());
  }
}

bool Path::hasExtension(
  const std::vector<std::string>& extensions, const bool caseSensitive) const
{
  for (const auto& extension : extensions)
  {
    if (hasExtension(extension, caseSensitive))
    {
      return true;
    }
  }
  return false;
}

bool Path::hasDriveSpec() const
{
  return m_path.has_root_name();
}

Path Path::deleteExtension() const
{
  if (isEmpty())
  {
    return *this;
  }
  return deleteLastComponent() / Path{basename()};
}

Path Path::addExtension(const std::string& extension) const
{
  if (isEmpty())
  {
    throw PathException{"Cannot add extension to empty path"};
  }

  return Path{m_path.parent_path() / m_path.filename() += "." + extension};
}

Path Path::replaceExtension(const std::string& extension) const
{
  return deleteExtension().addExtension(extension);
}

Path Path::replaceBasename(const std::string& basename) const
{
  if (isEmpty())
  {
    throw PathException{"Cannot replace the base name of an empty path."};
  }
  return deleteLastComponent() / Path{basename}.addExtension(extension());
}

bool Path::isAbsolute() const
{
  return m_path.is_absolute();
}

bool Path::canMakeRelative(const Path& absolutePath) const
{
  return (
    !isEmpty() && !absolutePath.isEmpty() && isAbsolute() && absolutePath.isAbsolute()
#ifdef _WIN32
    && !m_components.empty() && !absolutePath.m_components.empty()
    && m_components[0] == absolutePath.m_components[0]
#endif
  );
}

Path Path::makeAbsolute(const Path& relativePath) const
{
  if (!isAbsolute())
  {
    throw PathException{"Cannot make absolute path from relative path"};
  }

  if (relativePath.isAbsolute())
  {
    throw PathException{"Cannot make absolute path with absolute sub path"};
  }

  return relativePath.isEmpty() ? *this : *this / relativePath;
}

Path Path::makeRelative() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot make relative path from an empty reference path"};
  }

  if (!isAbsolute())
  {
    throw PathException{"Cannot make relative path from relative reference path"};
  }

  return Path{m_path.relative_path()};
}

Path Path::makeRelative(const Path& absolutePath) const
{
  if (isEmpty())
  {
    throw PathException{"Cannot make relative path from an empty reference path"};
  }

  if (absolutePath.isEmpty())
  {
    throw PathException{"Cannot make relative path with empty sub path"};
  }

  if (!isAbsolute())
  {
    throw PathException{"Cannot make relative path from relative reference path"};
  }

  if (!absolutePath.isAbsolute())
  {
    throw PathException{"Cannot make relative path with relative sub path"};
  }

  return Path{
    absolutePath.m_path.lexically_normal().lexically_relative(m_path.lexically_normal())};
}

Path Path::makeCanonical() const
{
  auto normalPath = m_path.lexically_normal();
  if (std::any_of(normalPath.begin(), normalPath.end(), [](const auto& component) {
        return component == "..";
      }))
  {
    throw PathException{"Cannot make path canonical"};
  }
  return Path{std::move(normalPath)};
}

Path Path::makeLowerCase() const
{
  return Path{kdl::str_to_lower(m_path.u8string())};
}

std::vector<Path> Path::makeAbsoluteAndCanonical(
  const std::vector<Path>& paths, const Path& relativePath)
{
  auto result = std::vector<Path>();
  result.reserve(paths.size());
  for (const auto& path : paths)
  {
    result.push_back(path.makeAbsolute(relativePath).makeCanonical());
  }
  return result;
}

std::ostream& operator<<(std::ostream& stream, const Path& path)
{
  stream << path.asString();
  return stream;
}
} // namespace TrenchBroom::IO
