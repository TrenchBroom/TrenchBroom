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

#include "Macros.h"

#include "kd/path_utils.h"
#include "kd/reflection_impl.h"
#include "kd/string_compare.h"
#include "kd/string_utils.h"

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
    return EntityDefinitionFileSpec::makeExternal(kdl::parse_path(str.substr(9)));
  }

  if (kdl::cs::str_is_prefix(str, "builtin:"))
  {
    return EntityDefinitionFileSpec::makeBuiltin(kdl::parse_path(str.substr(8)));
  }

  return std::nullopt;
}

EntityDefinitionFileSpec EntityDefinitionFileSpec::makeBuiltin(
  const std::filesystem::path& path)
{
  return {Type::Builtin, path};
}

EntityDefinitionFileSpec EntityDefinitionFileSpec::makeExternal(
  const std::filesystem::path& path)
{
  return {Type::External, path};
}

std::string EntityDefinitionFileSpec::asString() const
{
  // to avoid backslashes being misinterpreted as escape sequences
  const auto forwardPath = kdl::str_replace_every(path.string(), "\\", "/");

  switch (type)
  {
  case Type::Builtin:
    return fmt::format("builtin:{}", forwardPath);
  case Type::External:
    return fmt::format("external:{}", forwardPath);
    switchDefault();
  }
}

} // namespace tb::mdl
