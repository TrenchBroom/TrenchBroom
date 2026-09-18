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

#include "ql/WorldNodeLazyMap.h"

#include "el/LazyMap.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ql/EntityNodeLazyMap.h"
#include "ql/NodeQueryValues.h"


namespace tb::ql
{

namespace
{

struct LazyWorldNode
{
  const mdl::Map& map;
  const mdl::WorldNode& node;
};

const el::LazyMapFields<LazyWorldNode>& worldNodeLazyMapFields()
{
  static const auto fields = el::LazyMapFields<LazyWorldNode>{
    {"type", {el::ValueType::String, [](const auto&) { return el::Value{"world"}; }}},
    {"classname",
     {el::ValueType::String,
      [](const auto& b) { return el::Value{b.node.entity().classname()}; }}},
    {"properties",
     {el::ValueType::LazyMap,
      [](const auto& b) { return makeEntityPropertiesLazyMap(b.node.entity()); }}},
    {"tags",
     {el::ValueType::Array, [](const auto& b) { return tagsValue(b.map, b.node); }}},
    {"visible",
     {el::ValueType::Boolean, [](const auto& b) { return el::Value{b.node.visible()}; }}},
    {"locked",
     {el::ValueType::Boolean, [](const auto& b) { return el::Value{b.node.locked()}; }}},
  };
  return fields;
}

} // namespace

el::LazyMap makeWorldNodeLazyMap(const mdl::Map& map, const mdl::WorldNode& node)
{
  return el::makeLazyMapOf(LazyWorldNode{map, node}, worldNodeLazyMapFields());
}

const kdl::flat_map<std::string, el::ValueType>& worldNodeFieldTypes()
{
  static const auto fieldTypes = el::lazyMapFieldTypes(worldNodeLazyMapFields());
  return fieldTypes;
}

} // namespace tb::ql
