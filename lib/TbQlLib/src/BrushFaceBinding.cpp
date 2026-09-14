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

#include "ql/BrushFaceBinding.h"

#include "el/BoundValue.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "ql/NodeQueryValues.h"

#include "kd/ranges/to.h"

#include <ranges>

namespace tb::ql
{

namespace
{

struct BrushFaceBinding
{
  const mdl::Map& map;
  mdl::BrushFaceHandle handle;
};

const el::BoundValueFields<BrushFaceBinding>& brushFaceBindingFields()
{
  static const auto fields = el::BoundValueFields<BrushFaceBinding>{
    {"type", [](const BrushFaceBinding&) { return el::Value{"face"}; }},
    {"material",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.face().materialName()}; }},
    {"normal",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.face().normal()}; }},
    {"bounds",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.face().bounds()}; }},
    {"center",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.face().center()}; }},
    {"entity",
     [](const BrushFaceBinding& b) {
       return ownerEntityValue(b.handle.node().entity());
     }},
    {"layerName",
     [](const BrushFaceBinding& b) { return layerNameValue(b.handle.node()); }},
    {"groupName",
     [](const BrushFaceBinding& b) { return groupNameValue(b.handle.node()); }},
    {"tags", [](const BrushFaceBinding& b) { return tagsValue(b.map, b.handle.face()); }},
    {"visible",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.node().visible()}; }},
    {"locked",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.node().locked()}; }},
    {"selected",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.face().selected()}; }},
    {"linked",
     [](const BrushFaceBinding& b) {
       return el::Value{isLinked(b.map, b.handle.node())};
     }},
  };
  return fields;
}

} // namespace

el::BoundValue makeBrushFaceBinding(
  const mdl::Map& map, const mdl::BrushFaceHandle& handle)
{
  return el::makeBoundValueOf(BrushFaceBinding{map, handle}, brushFaceBindingFields());
}

std::vector<std::string> brushFaceFieldNames()
{
  return brushFaceBindingFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
