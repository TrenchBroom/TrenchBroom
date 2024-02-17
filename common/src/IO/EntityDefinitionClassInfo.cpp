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

#include "EntityDefinitionClassInfo.h"

#include "Assets/PropertyDefinition.h"
#include "Macros.h"
#include "Model/EntityProperties.h"

#include "kdl/reflection_impl.h"
#include "kdl/vector_utils.h"

#include <vecmath/bbox_io.h>
#include <vecmath/vec_io.h>

#include <iostream>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
std::ostream& operator<<(std::ostream& str, const EntityDefinitionClassType type)
{
  switch (type)
  {
  case EntityDefinitionClassType::BaseClass:
    str << "BaseClass";
    break;
  case EntityDefinitionClassType::PointClass:
    str << "PointClass";
    break;
  case EntityDefinitionClassType::BrushClass:
    str << "BrushClass";
    break;
    switchDefault();
  }
  return str;
}

kdl_reflect_impl(EntityDefinitionClassInfo);

bool addPropertyDefinition(
  std::vector<std::shared_ptr<Assets::PropertyDefinition>>& propertyDefinitions,
  std::shared_ptr<Assets::PropertyDefinition> propertyDefinition)
{
  assert(propertyDefinition != nullptr);
  if (kdl::vec_contains(propertyDefinitions, [&](const auto& a) {
        return a->key() == propertyDefinition->key();
      }))
  {
    return false;
  }

  propertyDefinitions.push_back(std::move(propertyDefinition));
  return true;
}

} // namespace IO
} // namespace TrenchBroom
