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

#include "mdl/BrushFaceBoundValue.h"

#include "el/BoundValue.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/NodeQueryValues.h"

namespace tb::mdl
{

namespace
{

struct BrushFaceBinding
{
  const Map& map;
  BrushFaceHandle handle;
};

const el::BoundValueFields<BrushFaceBinding>& brushFaceBoundValueFields()
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
       return ownerEntityValue(b.handle.node()->entity());
     }},
    {"layerName",
     [](const BrushFaceBinding& b) { return layerNameValue(*b.handle.node()); }},
    {"groupName",
     [](const BrushFaceBinding& b) { return groupNameValue(*b.handle.node()); }},
    {"tags", [](const BrushFaceBinding& b) { return tagsValue(b.map, b.handle.face()); }},
    {"visible",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.node()->visible()}; }},
    {"locked",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.node()->locked()}; }},
    {"selected",
     [](const BrushFaceBinding& b) { return el::Value{b.handle.face().selected()}; }},
  };
  return fields;
}

} // namespace

el::BoundValue makeBrushFaceBoundValue(const Map& map, const BrushFaceHandle& handle)
{
  return el::makeBoundValueOf(BrushFaceBinding{map, handle}, brushFaceBoundValueFields());
}

} // namespace tb::mdl
