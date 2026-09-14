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

#include "ql/GroupNodeLazyMap.h"

#include "el/LazyMap.h"
#include "mdl/GroupNode.h"
#include "ql/NodeQueryValues.h"


namespace tb::ql
{

namespace
{

struct LazyGroupNode
{
  const mdl::GroupNode& node;
};

const el::LazyMapFields<LazyGroupNode>& groupNodeLazyMapFields()
{
  static const auto fields = el::LazyMapFields<LazyGroupNode>{
    {"type", {el::ValueType::String, [](const auto&) { return el::Value{"group"}; }}},
    {"name",
     {el::ValueType::String,
      [](const auto& b) { return el::Value{b.node.group().name()}; }}},
    {"bounds",
     {el::ValueType::BBox,
      [](const auto& b) { return el::Value{b.node.logicalBounds()}; }}},
    {"center",
     {el::ValueType::Vec3,
      [](const auto& b) { return el::Value{b.node.logicalBounds().center()}; }}},
    {"visible",
     {el::ValueType::Boolean, [](const auto& b) { return el::Value{b.node.visible()}; }}},
    {"locked",
     {el::ValueType::Boolean, [](const auto& b) { return el::Value{b.node.locked()}; }}},
    {"layerName",
     {el::ValueType::String, [](const auto& b) { return layerNameValue(b.node); }}},
    {"groupName",
     {el::ValueType::String, [](const auto& b) { return groupNameValue(b.node); }}},
  };
  return fields;
}

} // namespace

el::LazyMap makeGroupNodeLazyMap(const mdl::GroupNode& node)
{
  return el::makeLazyMapOf(LazyGroupNode{node}, groupNodeLazyMapFields());
}

const kdl::flat_map<std::string, el::ValueType>& groupNodeFieldTypes()
{
  static const auto fieldTypes = el::lazyMapFieldTypes(groupNodeLazyMapFields());
  return fieldTypes;
}

} // namespace tb::ql
