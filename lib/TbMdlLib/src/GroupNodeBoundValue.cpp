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

#include "mdl/GroupNodeBoundValue.h"

#include "el/BoundValue.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/NodeQueryValues.h"

namespace tb::mdl
{

namespace
{

struct GroupNodeBinding
{
  const Map& map;
  const GroupNode& node;
};

const el::BoundValueFields<GroupNodeBinding>& groupNodeBoundValueFields()
{
  static const auto fields = el::BoundValueFields<GroupNodeBinding>{
    {"type", [](const GroupNodeBinding&) { return el::Value{"group"}; }},
    {"name", [](const GroupNodeBinding& b) { return el::Value{b.node.group().name()}; }},
    {"bounds",
     [](const GroupNodeBinding& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center",
     [](const GroupNodeBinding& b) {
       return el::Value{b.node.logicalBounds().center()};
     }},
    {"visible", [](const GroupNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const GroupNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const GroupNodeBinding& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const GroupNodeBinding& b) { return layerNameValue(b.node); }},
    {"groupName", [](const GroupNodeBinding& b) { return groupNameValue(b.node); }},
    {"linked",
     [](const GroupNodeBinding& b) { return el::Value{isLinked(b.map, b.node)}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makeGroupNodeBoundValue(const Map& map, const GroupNode& node)
{
  return el::makeBoundValueOf(GroupNodeBinding{map, node}, groupNodeBoundValueFields());
}

} // namespace tb::mdl
