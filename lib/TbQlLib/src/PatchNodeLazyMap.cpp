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

#include "ql/PatchNodeLazyMap.h"

#include "el/LazyMap.h"
#include "mdl/Map.h"
#include "mdl/PatchNode.h"
#include "ql/NodeQueryValues.h"


namespace tb::ql
{

namespace
{

struct LazyPatchNode
{
  const mdl::Map& map;
  const mdl::PatchNode& node;
};

const el::LazyMapFields<LazyPatchNode>& patchNodeLazyMapFields()
{
  static const auto fields = el::LazyMapFields<LazyPatchNode>{
    {"type", {el::ValueType::String, [](const auto&) { return el::Value{"patch"}; }}},
    {"entity",
     {el::ValueType::LazyMap,
      [](const auto& p) { return ownerEntityValue(p.node.entity()); }}},
    {"materials",
     {el::ValueType::Array,
      [](const auto& p) {
        return el::Value{el::ArrayType{el::Value{p.node.patch().materialName()}}};
      }}},
    {"tags",
     {el::ValueType::Array, [](const auto& p) { return tagsValue(p.map, p.node); }}},
    {"bounds",
     {el::ValueType::BBox,
      [](const auto& p) { return el::Value{p.node.logicalBounds()}; }}},
    {"center",
     {el::ValueType::Vec3,
      [](const auto& p) { return el::Value{p.node.logicalBounds().center()}; }}},
    {"visible",
     {el::ValueType::Boolean, [](const auto& p) { return el::Value{p.node.visible()}; }}},
    {"locked",
     {el::ValueType::Boolean, [](const auto& p) { return el::Value{p.node.locked()}; }}},
    {"layerName",
     {el::ValueType::String, [](const auto& p) { return layerNameValue(p.node); }}},
    {"groupName",
     {el::ValueType::String, [](const auto& p) { return groupNameValue(p.node); }}},
  };
  return fields;
}

} // namespace

el::LazyMap makePatchNodeLazyMap(const mdl::Map& map, const mdl::PatchNode& node)
{
  return el::makeLazyMapOf(LazyPatchNode{map, node}, patchNodeLazyMapFields());
}

const kdl::flat_map<std::string, el::ValueType>& patchNodeFieldTypes()
{
  static const auto fieldTypes = el::lazyMapFieldTypes(patchNodeLazyMapFields());
  return fieldTypes;
}

} // namespace tb::ql
