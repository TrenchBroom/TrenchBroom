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

#include "mdl/WorldNodeBoundValue.h"

#include "el/BoundValue.h"
#include "mdl/Map.h"
#include "mdl/NodeQueryValues.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

namespace
{

struct WorldNodeBinding
{
  const Map& map;
  const WorldNode& node;
};

const el::BoundValueFields<WorldNodeBinding>& worldNodeBoundValueFields()
{
  static const auto fields = el::BoundValueFields<WorldNodeBinding>{
    {"type", [](const WorldNodeBinding&) { return el::Value{"world"}; }},
    {"bounds",
     [](const WorldNodeBinding& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center",
     [](const WorldNodeBinding& b) {
       return el::Value{b.node.logicalBounds().center()};
     }},
    {"visible", [](const WorldNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const WorldNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const WorldNodeBinding& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const WorldNodeBinding& b) { return layerNameValue(b.node); }},
    {"groupName", [](const WorldNodeBinding& b) { return groupNameValue(b.node); }},
    {"linked",
     [](const WorldNodeBinding& b) { return el::Value{isLinked(b.map, b.node)}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makeWorldNodeBoundValue(const Map& map, const WorldNode& node)
{
  return el::makeBoundValueOf(WorldNodeBinding{map, node}, worldNodeBoundValueFields());
}

} // namespace tb::mdl
