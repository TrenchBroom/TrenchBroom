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

#include "Color.h"
#include "FileLocation.h"
#include "mdl/DecalDefinition.h"
#include "mdl/ModelDefinition.h"
#include "mdl/PropertyDefinition.h"

#include "kdl/reflection_decl.h"

#include "vm/bbox.h"

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{
struct PropertyDefinition;
}

namespace tb::io
{
enum class EntityDefinitionClassType
{
  PointClass,
  BrushClass,
  BaseClass
};

std::ostream& operator<<(std::ostream& str, EntityDefinitionClassType type);

struct EntityDefinitionClassInfo
{
  EntityDefinitionClassType type;
  FileLocation location;
  std::string name;

  std::optional<std::string> description;
  std::optional<Color> color;
  std::optional<vm::bbox3d> size;
  std::optional<mdl::ModelDefinition> modelDefinition;
  std::optional<mdl::DecalDefinition> decalDefinition;

  std::vector<mdl::PropertyDefinition> propertyDefinitions;
  std::vector<std::string> superClasses;

  kdl_reflect_decl(
    EntityDefinitionClassInfo,
    type,
    location,
    name,
    description,
    color,
    size,
    modelDefinition,
    decalDefinition,
    propertyDefinitions,
    superClasses);
};

bool addPropertyDefinition(
  std::vector<mdl::PropertyDefinition>& propertyDefinitions,
  mdl::PropertyDefinition propertyDefinition);

} // namespace tb::io
