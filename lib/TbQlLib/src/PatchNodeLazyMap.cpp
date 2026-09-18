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

#include "kd/ranges/to.h"

#include <ranges>

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
    {"type", [](const auto&) { return el::Value{"patch"}; }},
    {"entity", [](const auto& p) { return ownerEntityValue(p.node.entity()); }},
    {"materials",
     [](const auto& p) {
       return el::Value{el::ArrayType{el::Value{p.node.patch().materialName()}}};
     }},
    {"tags", [](const auto& p) { return tagsValue(p.map, p.node); }},
    {"bounds", [](const auto& p) { return el::Value{p.node.logicalBounds()}; }},
    {"center", [](const auto& p) { return el::Value{p.node.logicalBounds().center()}; }},
    {"visible", [](const auto& p) { return el::Value{p.node.visible()}; }},
    {"locked", [](const auto& p) { return el::Value{p.node.locked()}; }},
    {"selected", [](const auto& p) { return el::Value{p.node.selected()}; }},
    {"layerName", [](const auto& p) { return layerNameValue(p.node); }},
    {"groupName", [](const auto& p) { return groupNameValue(p.node); }},
  };
  return fields;
}

} // namespace

el::LazyMap makePatchNodeLazyMap(const mdl::Map& map, const mdl::PatchNode& node)
{
  return el::makeLazyMapOf(LazyPatchNode{map, node}, patchNodeLazyMapFields());
}

std::vector<std::string> patchNodeFieldNames()
{
  return patchNodeLazyMapFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
