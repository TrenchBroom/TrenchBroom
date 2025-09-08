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

#pragma once

#include "kdl/reflection_decl.h"

#include <filesystem>
#include <optional>
#include <string>

namespace tb::mdl
{

class EntityDefinitionFileSpec
{
private:
  enum class Type
  {
    Builtin,
    External,
  };

  friend std::ostream& operator<<(std::ostream& lhs, Type rhs);

  Type m_type;
  std::filesystem::path m_path;

  kdl_reflect_decl(EntityDefinitionFileSpec, m_type, m_path);

public:
  static std::optional<EntityDefinitionFileSpec> parse(const std::string& str);
  static EntityDefinitionFileSpec builtin(const std::filesystem::path& path);
  static EntityDefinitionFileSpec external(const std::filesystem::path& path);
  static EntityDefinitionFileSpec unset();

  bool builtin() const;
  bool external() const;

  const std::filesystem::path& path() const;

  std::string asString() const;

private:
  EntityDefinitionFileSpec(Type type, std::filesystem::path path);
};

} // namespace tb::mdl
