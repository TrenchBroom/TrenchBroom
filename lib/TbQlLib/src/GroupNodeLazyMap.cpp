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

#include "kd/ranges/to.h"

#include <ranges>

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
    {"type", [](const auto&) { return el::Value{"group"}; }},
    {"name", [](const auto& b) { return el::Value{b.node.group().name()}; }},
    {"bounds", [](const auto& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center", [](const auto& b) { return el::Value{b.node.logicalBounds().center()}; }},
    {"visible", [](const auto& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const auto& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const auto& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const auto& b) { return layerNameValue(b.node); }},
    {"groupName", [](const auto& b) { return groupNameValue(b.node); }},
  };
  return fields;
}

} // namespace

el::LazyMap makeGroupNodeLazyMap(const mdl::GroupNode& node)
{
  return el::makeLazyMapOf(LazyGroupNode{node}, groupNodeLazyMapFields());
}

std::vector<std::string> groupNodeFieldNames()
{
  return groupNodeLazyMapFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
