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

#include "kd/ranges/to.h"

#include <ranges>

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
    {"type", [](const auto&) { return el::Value{"world"}; }},
    {"classname", [](const auto& b) { return el::Value{b.node.entity().classname()}; }},
    {"properties",
     [](const auto& b) { return makeEntityPropertiesLazyMap(b.node.entity()); }},
    {"tags", [](const auto& b) { return tagsValue(b.map, b.node); }},
    {"visible", [](const auto& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const auto& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const auto& b) { return el::Value{b.node.selected()}; }},
  };
  return fields;
}

} // namespace

el::LazyMap makeWorldNodeLazyMap(const mdl::Map& map, const mdl::WorldNode& node)
{
  return el::makeLazyMapOf(LazyWorldNode{map, node}, worldNodeLazyMapFields());
}

std::vector<std::string> worldNodeFieldNames()
{
  return worldNodeLazyMapFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
