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

#include "kdl/reflection_impl.h"
#include "kdl/string_compare.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <cassert>
#include <string>

namespace tb::mdl
{

kdl_reflect_impl(EntityDefinitionFileSpec);

std::ostream& operator<<(std::ostream& lhs, const EntityDefinitionFileSpec::Type rhs)
{
  switch (rhs)
  {
  case EntityDefinitionFileSpec::Type::Builtin:
    lhs << "builtin";
    break;
  case EntityDefinitionFileSpec::Type::External:
    lhs << "external";
    break;
  }
  return lhs;
}

std::optional<EntityDefinitionFileSpec> EntityDefinitionFileSpec::parse(
  const std::string& str)
{
  if (kdl::cs::str_is_prefix(str, "external:"))
  {
    return EntityDefinitionFileSpec::external(str.substr(9));
  }

  if (kdl::cs::str_is_prefix(str, "builtin:"))
  {
    return EntityDefinitionFileSpec::builtin(str.substr(8));
  }

  return std::nullopt;
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
  return builtin() ? fmt::format("builtin:{}", m_path)
                   : fmt::format("external:{}", m_path);
}

EntityDefinitionFileSpec::EntityDefinitionFileSpec(
  const Type type, std::filesystem::path path)
  : m_type{type}
  , m_path{std::move(path)}
{
  assert(!m_path.empty());
}

} // namespace tb::mdl
