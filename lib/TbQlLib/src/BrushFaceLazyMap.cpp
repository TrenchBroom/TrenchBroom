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

#include "ql/BrushFaceLazyMap.h"

#include "el/LazyMap.h"
#include "mdl/BrushFace.h" // IWYU pragma: keep
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "ql/NodeQueryValues.h"

#include "kd/ranges/to.h"

#include <ranges>

namespace tb::ql
{

namespace
{

struct LazyBrushFace
{
  const mdl::Map& map;
  mdl::BrushFaceHandle handle;
};

const el::LazyMapFields<LazyBrushFace>& brushFaceLazyMapFields()
{
  static const auto fields = el::LazyMapFields<LazyBrushFace>{
    {"type", [](const auto&) { return el::Value{"face"}; }},
    {"material", [](const auto& b) { return el::Value{b.handle.face().materialName()}; }},
    {"normal", [](const auto& b) { return el::Value{b.handle.face().normal()}; }},
    {"bounds", [](const auto& b) { return el::Value{b.handle.face().bounds()}; }},
    {"center", [](const auto& b) { return el::Value{b.handle.face().center()}; }},
    {"entity", [](const auto& b) { return ownerEntityValue(b.handle.node()->entity()); }},
    {"layerName", [](const auto& b) { return layerNameValue(*b.handle.node()); }},
    {"groupName", [](const auto& b) { return groupNameValue(*b.handle.node()); }},
    {"tags", [](const auto& b) { return tagsValue(b.map, b.handle.face()); }},
    {"visible", [](const auto& b) { return el::Value{b.handle.node()->visible()}; }},
    {"locked", [](const auto& b) { return el::Value{b.handle.node()->locked()}; }},
    {"selected", [](const auto& b) { return el::Value{b.handle.face().selected()}; }},
  };
  return fields;
}

} // namespace

el::LazyMap makeBrushFaceLazyMap(const mdl::Map& map, const mdl::BrushFaceHandle& handle)
{
  return el::makeLazyMapOf(LazyBrushFace{map, handle}, brushFaceLazyMapFields());
}

std::vector<std::string> brushFaceFieldNames()
{
  return brushFaceLazyMapFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
