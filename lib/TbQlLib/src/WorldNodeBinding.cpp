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

#include "ql/WorldNodeBinding.h"

#include "el/BoundValue.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ql/EntityNodeBinding.h"
#include "ql/NodeQueryValues.h"

#include "kd/ranges/to.h"

#include <ranges>

namespace tb::ql
{

namespace
{

struct WorldNodeBinding
{
  const mdl::Map& map;
  const mdl::WorldNode& node;
};

const el::BoundValueFields<WorldNodeBinding>& worldNodeBindingFields()
{
  static const auto fields = el::BoundValueFields<WorldNodeBinding>{
    {"type", [](const WorldNodeBinding&) { return el::Value{"world"}; }},
    {"classname",
     [](const WorldNodeBinding& b) { return el::Value{b.node.entity().classname()}; }},
    {"properties",
     [](const WorldNodeBinding& b) {
       return makeEntityPropertiesBinding(b.node.entity());
     }},
    {"tags", [](const WorldNodeBinding& b) { return tagsValue(b.map, b.node); }},
    {"visible", [](const WorldNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const WorldNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const WorldNodeBinding& b) { return el::Value{b.node.selected()}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makeWorldNodeBinding(const mdl::Map& map, const mdl::WorldNode& node)
{
  return el::makeBoundValueOf(WorldNodeBinding{map, node}, worldNodeBindingFields());
}

std::vector<std::string> worldNodeFieldNames()
{
  return worldNodeBindingFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
