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

#include <kdl/path_utils.h>
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

std::string Path::string() const
{
  return m_path.string();
}

std::string Path::generic_string() const
{
  return m_path.generic_string();
}

size_t Path::hidden_length() const
{
  return size_t(std::distance(m_path.begin(), m_path.end()));
}

bool Path::empty() const
{
  return m_path.empty();
}

Path Path::parent_path() const
{
  return Path{m_path.parent_path()};
}

Path Path::hidden_front() const
{
  return Path{kdl::path_front(m_path)};
}

Path Path::hidden_pop_front() const
{
  return Path{kdl::path_pop_front(m_path)};
}

Path Path::hidden_clip(const size_t index, const size_t count) const
{
  return Path{kdl::path_clip(m_path, index, count)};
}

Path Path::filename() const
{
  return Path{m_path.filename()};
}

Path Path::stem() const
{
  return Path{m_path.stem()};
}

Path Path::extension() const
{
  return Path{m_path.extension()};
}

bool Path::hidden_hasPrefix(const Path& prefix) const
{
  return kdl::path_has_prefix(m_path, prefix.m_path);
}

Path Path::hidden_addExtension(const std::string& extension) const
{
  return Path{kdl::path_add_extension(m_path, extension)};
}

Path Path::hidden_removeExtension() const
{
  return Path{kdl::path_remove_extension(m_path)};
}

Path Path::hidden_replaceExtension(const std::string& extension) const
{
  return Path{kdl::path_replace_extension(m_path, extension)};
}

bool Path::is_absolute() const
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
  return Path{m_path.lexically_normal()};
}

Path Path::hidden_makeLowerCase() const
{
  return Path{kdl::str_to_lower(m_path.u8string())};
}

std::ostream& operator<<(std::ostream& stream, const Path& path)
{
  stream << path.string();
  return stream;
}
} // namespace TrenchBroom::IO

namespace kdl
{
Path path_front(const Path& path)
{
  return path.hidden_front();
}

Path path_pop_front(const Path& path)
{
  return path.hidden_pop_front();
}


size_t path_length(const Path& path)
{
  return path.hidden_length();
}

bool path_has_prefix(const Path& path, const Path& prefix)
{
  return path.hidden_hasPrefix(prefix);
}

Path path_to_lower(const Path& path)
{
  return path.hidden_makeLowerCase();
}

Path path_clip(const Path& path, size_t index, size_t length)
{
  return path.hidden_clip(index, length);
}

Path path_clip(const Path& path, size_t index)
{
  return path_clip(path, index, path_length(path));
}

Path path_add_extension(const Path& path, const std::string& extension)
{
  return path.hidden_addExtension(extension);
}

Path path_remove_extension(Path path)
{
  return path.hidden_removeExtension();
}

Path path_replace_extension(Path path, const std::string& extension)
{
  return path.hidden_replaceExtension(extension);
}
} // namespace kdl
