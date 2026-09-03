/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/QueryVariableValues.h"

#include "mdl/Entity.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Object.h"
#include "mdl/Tag.h"

namespace tb::mdl
{

el::MapType propertiesMapValue(const Entity& entity)
{
  auto properties = el::MapType{};
  for (const auto& key : entity.propertyKeys())
  {
    if (const auto* value = entity.property(key))
    {
      properties.emplace(key, el::Value{*value});
    }
  }
  return properties;
}

el::Value entityMapValue(const Entity& entity)
{
  return el::Value{el::MapType{
    {"classname", el::Value{entity.classname()}},
    {"properties", el::Value{propertiesMapValue(entity)}},
  }};
}

el::Value layerNameValue(const Object& object)
{
  const auto* layer = object.containingLayer();
  return layer ? el::Value{layer->name()} : el::Value::Undefined;
}

el::Value groupNameValue(const Object& object)
{
  const auto* group = object.containingGroup();
  return group ? el::Value{group->name()} : el::Value::Undefined;
}

el::Value tagsArrayValue(Map& map, const Taggable& taggable)
{
  auto result = el::ArrayType{};
  for (const auto& tag : map.smartTags())
  {
    if (taggable.hasTag(tag))
    {
      result.emplace_back(el::Value{tag.name()});
    }
  }
  return el::Value{std::move(result)};
}

} // namespace tb::mdl
