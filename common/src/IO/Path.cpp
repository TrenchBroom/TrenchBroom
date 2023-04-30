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

std::string Path::filename() const
{
  return m_path.filename().u8string();
}

std::string Path::basename() const
{
  return m_path.stem().u8string();
}

std::string Path::extension() const
{
  return m_path.extension().u8string();
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

Path Path::deleteExtension() const
{
  return isEmpty() ? *this : deleteLastComponent() / Path{basename()};
}

Path Path::addExtension(const std::string& extension) const
{
  return isEmpty() ? Path{extension}
                   : Path{m_path.parent_path() / m_path.filename() += extension};
}

bool Path::isAbsolute() const
{
  return m_path.is_absolute();
}

Path Path::makeRelative() const
{
  return Path{m_path.relative_path()};
}

Path Path::makeRelative(const Path& absolutePath) const
{
  return Path{absolutePath.m_path.lexically_relative(m_path)};
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

std::ostream& operator<<(std::ostream& stream, const Path& path)
{
  stream << path.asString();
  return stream;
}
} // namespace TrenchBroom::IO
