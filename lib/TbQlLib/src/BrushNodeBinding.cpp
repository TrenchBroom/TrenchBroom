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

#include "ql/BrushNodeBinding.h"

#include "el/BoundValue.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "ql/NodeQueryValues.h"

#include "kd/ranges/to.h"
#include "kd/vector_utils.h"

#include <ranges>
#include <string>

namespace tb::ql
{

namespace
{

struct BrushNodeBinding
{
  const mdl::Map& map;
  const mdl::BrushNode& node;
};

el::Value materialsValue(const mdl::Brush& brush)
{
  auto names =
    brush.faces()
    | std::views::transform([](const auto& face) { return face.materialName(); })
    | kdl::ranges::to<std::vector>();
  return el::Value{
    kdl::vec_sort_and_remove_duplicates(std::move(names))
    | std::views::transform([](const auto& name) { return el::Value{name}; })
    | kdl::ranges::to<std::vector>()};
}

const el::BoundValueFields<BrushNodeBinding>& brushNodeBindingFields()
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

el::BoundValue makeBrushNodeBinding(const mdl::Map& map, const mdl::BrushNode& node)
{
  return el::makeBoundValueOf(BrushNodeBinding{map, node}, brushNodeBindingFields());
}

std::vector<std::string> brushNodeFieldNames()
{
  return brushNodeBindingFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
