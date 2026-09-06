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

#include "mdl/PatchNodeBoundValue.h"

#include "el/BoundValue.h"
#include "mdl/BezierPatch.h"
#include "mdl/Map.h"
#include "mdl/NodeQueryValues.h"
#include "mdl/PatchNode.h"

namespace tb::mdl
{

namespace
{

struct PatchNodeBinding
{
  const Map& map;
  const PatchNode& node;
};

const el::BoundValueFields<PatchNodeBinding>& patchNodeBoundValueFields()
{
  static const auto fields = el::BoundValueFields<PatchNodeBinding>{
    {"type", [](const PatchNodeBinding&) { return el::Value{"patch"}; }},
    {"entity",
     [](const PatchNodeBinding& b) { return ownerEntityValue(b.node.entity()); }},
    {"materials",
     [](const PatchNodeBinding& b) {
       return el::Value{el::ArrayType{el::Value{b.node.patch().materialName()}}};
     }},
    {"tags", [](const PatchNodeBinding& b) { return tagsValue(b.map, b.node); }},
    {"bounds",
     [](const PatchNodeBinding& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center",
     [](const PatchNodeBinding& b) {
       return el::Value{b.node.logicalBounds().center()};
     }},
    {"visible", [](const PatchNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const PatchNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const PatchNodeBinding& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const PatchNodeBinding& b) { return layerNameValue(b.node); }},
    {"groupName", [](const PatchNodeBinding& b) { return groupNameValue(b.node); }},
    {"linked",
     [](const PatchNodeBinding& b) { return el::Value{isLinked(b.map, b.node)}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makePatchNodeBoundValue(const Map& map, const PatchNode& node)
{
  return el::makeBoundValueOf(PatchNodeBinding{map, node}, patchNodeBoundValueFields());
}

} // namespace tb::mdl
