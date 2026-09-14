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

#include "ql/LayerNodeBinding.h"

#include "el/BoundValue.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "ql/NodeQueryValues.h"

#include "kd/ranges/to.h"

#include <ranges>

namespace tb::ql
{

namespace
{

struct LayerNodeBinding
{
  const mdl::Map& map;
  const mdl::LayerNode& node;
};

const el::BoundValueFields<LayerNodeBinding>& layerNodeBindingFields()
{
  static const auto fields = el::BoundValueFields<LayerNodeBinding>{
    {"type", [](const LayerNodeBinding&) { return el::Value{"layer"}; }},
    {"name", [](const LayerNodeBinding& b) { return el::Value{b.node.layer().name()}; }},
    {"bounds",
     [](const LayerNodeBinding& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center",
     [](const LayerNodeBinding& b) {
       return el::Value{b.node.logicalBounds().center()};
     }},
    {"visible", [](const LayerNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const LayerNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const LayerNodeBinding& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const LayerNodeBinding& b) { return layerNameValue(b.node); }},
    {"groupName", [](const LayerNodeBinding& b) { return groupNameValue(b.node); }},
    {"linked",
     [](const LayerNodeBinding& b) { return el::Value{isLinked(b.map, b.node)}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makeLayerNodeBinding(const mdl::Map& map, const mdl::LayerNode& node)
{
  return el::makeBoundValueOf(LayerNodeBinding{map, node}, layerNodeBindingFields());
}

std::vector<std::string> layerNodeFieldNames()
{
  return layerNodeBindingFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
