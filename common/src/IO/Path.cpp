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
#include <ostream>
#include <string>

namespace TrenchBroom::IO
{
static constexpr std::string_view separators()
{
  return std::string_view("/\\");
}

Path::Path(bool absolute, std::vector<std::string> components)
  : m_components{std::move(components)}
  , m_absolute{absolute}
{
}

Path::Path(const std::string& path)
{
  const auto trimmed = kdl::str_trim(path);
  m_components = kdl::str_split(trimmed, separators());
#ifdef _WIN32
  m_absolute =
    (hasDriveSpec(m_components) || (!trimmed.empty() && trimmed[0] == '/')
     || (!trimmed.empty() && trimmed[0] == '\\'));
#else
  m_absolute = !trimmed.empty() && kdl::cs::str_is_prefix(trimmed, separator());
#endif
}

Path Path::operator+(const Path& rhs) const
{
  if (rhs.isAbsolute())
  {
    throw PathException{"Cannot concatenate absolute path"};
  }
  auto components = m_components;
  components.insert(
    std::end(components), std::begin(rhs.m_components), std::end(rhs.m_components));
  return Path{m_absolute, std::move(components)};
}

int Path::compare(const Path& rhs, const bool caseSensitive) const
{
  if (!isAbsolute() && rhs.isAbsolute())
  {
    return -1;
  }
  else if (isAbsolute() && !rhs.isAbsolute())
  {
    return 1;
  }

  const auto& rcomps = rhs.m_components;

  size_t i = 0;
  const auto max =
    m_components.size() < rcomps.size() ? m_components.size() : rcomps.size();
  while (i < max)
  {
    const auto& mcomp = m_components[i];
    const auto& rcomp = rcomps[i];
    const auto result = caseSensitive ? kdl::cs::str_compare(mcomp, rcomp)
                                      : kdl::ci::str_compare(mcomp, rcomp);
    if (result < 0)
    {
      return -1;
    }
    else if (result > 0)
    {
      return 1;
    }
    ++i;
  }
  if (m_components.size() < rcomps.size())
  {
    return -1;
  }
  else if (m_components.size() > rcomps.size())
  {
    return 1;
  }
  else
  {
    return 0;
  }
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

std::string Path::asString(const std::string_view separator) const
{
  if (m_absolute)
  {
#ifdef _WIN32
    if (hasDriveSpec(m_components))
    {
      return kdl::str_join(m_components, separator);
    }
    else
    {
      return std::string(separator) + kdl::str_join(m_components, separator);
    }
#else
    return std::string(separator) + kdl::str_join(m_components, separator);
#endif
  }
  return kdl::str_join(m_components, separator);
}

std::vector<std::string> Path::asStrings(
  const std::vector<Path>& paths, const std::string_view separator)
{
  return kdl::vec_transform(
    paths, [&](const auto& path) { return path.asString(separator); });
}

std::vector<Path> Path::asPaths(const std::vector<std::string>& strs)
{
  return kdl::vec_transform(strs, [](const auto& str) { return Path{str}; });
}

size_t Path::length() const
{
  return m_components.size();
}

bool Path::isEmpty() const
{
  return !m_absolute && m_components.empty();
}

Path Path::firstComponent() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot return first component of empty path"};
  }

  if (!m_absolute)
  {
    return Path{m_components.front()};
  }

#ifdef _WIN32
  if (hasDriveSpec(m_components))
  {
    return Path{m_components.front()};
  }

  return Path{"\\"};
#else
  return Path{"/"};
#endif
}

Path Path::deleteFirstComponent() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot delete first component of empty path"};
  }
  if (!m_absolute)
  {
    auto components = std::vector<std::string>();
    components.reserve(m_components.size() - 1);
    components.insert(
      std::begin(components), std::begin(m_components) + 1, std::end(m_components));
    return Path{false, std::move(components)};
  }
#ifdef _WIN32
  if (!m_components.empty() && hasDriveSpec(m_components[0]))
  {
    std::vector<std::string> components;
    components.reserve(m_components.size() - 1);
    components.insert(
      std::begin(components), std::begin(m_components) + 1, std::end(m_components));
    return Path{false, std::move(components)};
  }
  return Path{false, m_components};
#else
  return Path{false, m_components};
#endif
}

Path Path::lastComponent() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot return last component of empty path"};
  }

  if (!m_components.empty())
  {
    return Path{m_components.back()};
  }
  else
  {
    return Path{};
  }
}

Path Path::deleteLastComponent() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot delete last component of empty path"};
  }

  if (!m_components.empty())
  {
    auto components = std::vector<std::string>();
    components.reserve(m_components.size() - 1);
    components.insert(
      std::begin(components), std::begin(m_components), std::end(m_components) - 1);
    return Path{m_absolute, std::move(components)};
  }
  else
  {
    return Path{m_absolute, m_components};
  }
}

Path Path::prefix(const size_t count) const
{
  return subPath(0, count);
}

Path Path::suffix(const size_t count) const
{
  return subPath(m_components.size() - count, count);
}

Path Path::subPath(const size_t index, const size_t count) const
{
  if (index + count > m_components.size())
  {
    throw PathException{"Sub path out of bounds"};
  }

  if (count == 0)
  {
    return Path{};
  }

  auto newComponents = std::vector<std::string>();
  newComponents.reserve(count);
  for (size_t i = 0u; i < count; ++i)
  {
    newComponents.push_back(m_components[index + i]);
  }
  return Path{m_absolute && index == 0, std::move(newComponents)};
}

const std::vector<std::string>& Path::components() const
{
  return m_components;
}

std::string Path::filename() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot get filename of empty path"};
  }

  if (m_components.empty())
  {
    return "";
  }
  else
  {
    return m_components.back();
  }
}

std::string Path::basename() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot get basename of empty path"};
  }

  const auto filename = this->filename();
  const auto dotIndex = filename.rfind('.');
  if (dotIndex == std::string::npos)
  {
    return filename;
  }
  else
  {
    return filename.substr(0, dotIndex);
  }
}

std::string Path::extension() const
{
  if (isEmpty())
  {
    throw PathException{"Cannot get extension of empty path"};
  }

  const auto filename = this->filename();
  const auto dotIndex = filename.rfind('.');
  if (dotIndex == std::string::npos)
  {
    return "";
  }
  else
  {
    return filename.substr(dotIndex + 1);
  }
}

bool Path::hasPrefix(const Path& prefix, bool caseSensitive) const
{
  if (prefix.length() > length())
  {
    return false;
  }

  return this->prefix(prefix.length()).compare(prefix, caseSensitive) == 0;
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
  return hasDriveSpec(m_components);
}

Path Path::deleteExtension() const
{
  if (isEmpty())
  {
    return *this;
  }
  return deleteLastComponent() + Path{basename()};
}

Path Path::addExtension(const std::string& extension) const
{
  if (isEmpty())
  {
    throw PathException{"Cannot add extension to empty path"};
  }

  auto components = m_components;
  if (
    components.empty()
#ifdef _WIN32
    || hasDriveSpec(m_components.back())
#endif
  )
  {
    components.push_back("." + extension);
  }
  else
  {
    components.back() += "." + extension;
  }
  return Path{m_absolute, std::move(components)};
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
  return deleteLastComponent() + Path{basename}.addExtension(extension());
}

bool Path::isAbsolute() const
{
  return m_absolute;
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

  return *this + relativePath;
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

#ifdef _WIN32
  if (m_components.empty())
  {
    throw PathException{
      "Cannot make relative path from an reference path with no drive spec"};
  }

  return Path{false, kdl::vec_slice_suffix(m_components, m_components.size() - 1u)};
#else
  return Path{false, m_components};
#endif
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

#ifdef _WIN32
  if (m_components.empty())
  {
    throw PathException{
      "Cannot make relative path from an reference path with no drive spec"};
  }
  if (absolutePath.m_components.empty())
  {
    throw PathException{"Cannot make relative path with sub path with no drive spec"};
  }
  if (m_components[0] != absolutePath.m_components[0])
  {
    throw PathException{
      "Cannot make relative path if reference path has different drive spec"};
  }
#endif

  const auto myResolved = resolvePath(true, m_components);
  const auto theirResolved = resolvePath(true, absolutePath.m_components);

  // cross off all common prefixes
  size_t p = 0;
  const auto max =
    myResolved.size() < theirResolved.size() ? myResolved.size() : theirResolved.size();
  while (p < max)
  {
    if (myResolved[p] != theirResolved[p])
    {
      break;
    }
    ++p;
  }

  auto components = std::vector<std::string>();
  for (size_t i = p; i < myResolved.size(); ++i)
  {
    components.emplace_back("..");
  }
  for (size_t i = p; i < theirResolved.size(); ++i)
  {
    components.push_back(theirResolved[i]);
  }

  return Path{false, std::move(components)};
}

Path Path::makeCanonical() const
{
  return Path{m_absolute, resolvePath(m_absolute, m_components)};
}

Path Path::makeLowerCase() const
{
  auto lcComponents = std::vector<std::string>();
  lcComponents.reserve(m_components.size());
  for (const auto& component : m_components)
  {
    lcComponents.push_back(kdl::str_to_lower(component));
  }
  return Path{m_absolute, std::move(lcComponents)};
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

#ifdef _WIN32
bool Path::hasDriveSpec(const std::vector<std::string>& components)
{
  if (components.empty())
  {
    return false;
  }
  else
  {
    return hasDriveSpec(components[0]);
  }
}
#else
bool Path::hasDriveSpec(const std::vector<std::string>& /* components */)
{
  return false;
}
#endif

#ifdef _WIN32
bool Path::hasDriveSpec(const std::string& component)
{
  if (component.size() <= 1)
  {
    return false;
  }
  else
  {
    return component[1] == ':';
  }
}
#else
bool Path::hasDriveSpec(const std::string& /* component */)
{
  return false;
}
#endif

std::vector<std::string> Path::resolvePath(
  const bool absolute, const std::vector<std::string>& components) const
{
  auto resolved = std::vector<std::string>();
  for (const auto& comp : components)
  {
    if (comp == ".")
    {
      continue;
    }
    if (comp == "..")
    {
      if (resolved.empty())
      {
        throw PathException{"Cannot resolve path"};
      }

#ifdef _WIN32
      if (absolute && hasDriveSpec(resolved[0]) && resolved.size() < 2)
      {
        throw PathException{"Cannot resolve path"};
      }
#else
      unused(absolute);
#endif
      resolved.pop_back();
      continue;
    }
    resolved.push_back(comp);
  }
  return resolved;
}

std::ostream& operator<<(std::ostream& stream, const Path& path)
{
  stream << path.asString();
  return stream;
}
} // namespace TrenchBroom::IO
