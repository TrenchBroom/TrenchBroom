/*
 Copyright (C) 2010 Kristian Duske

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

#include "EntityDefinitionFileSpec.h"

#include "kdl/string_compare.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <cassert>
#include <string>


namespace tb::mdl
{

EntityDefinitionFileSpec::EntityDefinitionFileSpec() = default;

EntityDefinitionFileSpec EntityDefinitionFileSpec::parse(const std::string& str)
{
  if (kdl::cs::str_is_prefix(str, "external:"))
  {
    return EntityDefinitionFileSpec::external(str.substr(9));
  }

  if (kdl::cs::str_is_prefix(str, "builtin:"))
  {
    return EntityDefinitionFileSpec::builtin(str.substr(8));
  }

  // If the location spec is missing, we assume that an absolute path indicates an
  // external file spec, and a relative path indicates a builtin file spec.
  const auto path = std::filesystem::path{str};
  return path.is_absolute() ? EntityDefinitionFileSpec::external(path)
                            : EntityDefinitionFileSpec::builtin(path);
}

EntityDefinitionFileSpec EntityDefinitionFileSpec::builtin(
  const std::filesystem::path& path)
{
  return {Type::Builtin, path};
}

EntityDefinitionFileSpec EntityDefinitionFileSpec::external(
  const std::filesystem::path& path)
{
  return {Type::External, path};
}

EntityDefinitionFileSpec EntityDefinitionFileSpec::unset()
{
  return {};
}

bool operator<(const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs)
{
  if (lhs.m_type < rhs.m_type)
  {
    return true;
  }

  if (lhs.m_type > rhs.m_type)
  {
    return false;
  }

  return lhs.m_path < rhs.m_path;
}

bool operator==(const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs)
{
  return lhs.m_type == rhs.m_type && lhs.m_path == rhs.m_path;
}

bool operator!=(const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs)
{
  return !(lhs == rhs);
}

bool EntityDefinitionFileSpec::valid() const
{
  return m_type != Type::Unset;
}

bool EntityDefinitionFileSpec::builtin() const
{
  return m_type == Type::Builtin;
}

bool EntityDefinitionFileSpec::external() const
{
  return m_type == Type::External;
}

const std::filesystem::path& EntityDefinitionFileSpec::path() const
{
  return m_path;
}

std::string EntityDefinitionFileSpec::asString() const
{
  return !valid()    ? std::string{}
         : builtin() ? fmt::format("builtin:{}", m_path)
                     : fmt::format("external:{}", m_path);
}

EntityDefinitionFileSpec::EntityDefinitionFileSpec(
  const Type type, std::filesystem::path path)
  : m_type{type}
  , m_path{std::move(path)}
{
  assert(valid());
  assert(!m_path.empty());
}
} // namespace tb::mdl
