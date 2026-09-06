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

#include "mdl/BrushNodeBoundValue.h"

#include "el/BoundValue.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/NodeQueryValues.h"

#include "kd/ranges/to.h"

#include <ranges>
#include <set>
#include <string>

namespace tb::mdl
{

namespace
{

struct BrushNodeBinding
{
  const Map& map;
  const BrushNode& node;
};

el::Value materialsValue(const Brush& brush)
{
  auto names = std::set<std::string>{};
  for (const auto& face : brush.faces())
  {
    names.insert(face.materialName());
  }
  return el::Value{
    names | std::views::transform([](const auto& name) { return el::Value{name}; })
    | kdl::ranges::to<std::vector>()};
}

const el::BoundValueFields<BrushNodeBinding>& brushNodeBoundValueFields()
{
  static const auto fields = el::BoundValueFields<BrushNodeBinding>{
    {"type", [](const BrushNodeBinding&) { return el::Value{"brush"}; }},
    {"entity",
     [](const BrushNodeBinding& b) { return ownerEntityValue(b.node.entity()); }},
    {"materials",
     [](const BrushNodeBinding& b) { return materialsValue(b.node.brush()); }},
    {"tags", [](const BrushNodeBinding& b) { return tagsValue(b.map, b.node); }},
    {"bounds",
     [](const BrushNodeBinding& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center",
     [](const BrushNodeBinding& b) {
       return el::Value{b.node.logicalBounds().center()};
     }},
    {"visible", [](const BrushNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const BrushNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const BrushNodeBinding& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const BrushNodeBinding& b) { return layerNameValue(b.node); }},
    {"groupName", [](const BrushNodeBinding& b) { return groupNameValue(b.node); }},
    {"linked",
     [](const BrushNodeBinding& b) { return el::Value{isLinked(b.map, b.node)}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makeBrushNodeBoundValue(const Map& map, const BrushNode& node)
{
  return el::makeBoundValueOf(BrushNodeBinding{map, node}, brushNodeBoundValueFields());
}

} // namespace tb::mdl
