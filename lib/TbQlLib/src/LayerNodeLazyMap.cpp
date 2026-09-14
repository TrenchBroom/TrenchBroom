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

#include "ql/LayerNodeLazyMap.h"

#include "el/LazyMap.h"
#include "mdl/LayerNode.h"
#include "ql/NodeQueryValues.h"


namespace tb::ql
{

namespace
{

struct LazyLayerNode
{
  const mdl::LayerNode& node;
};

const el::LazyMapFields<LazyLayerNode>& layerNodeLazyMapFields()
{
  static const auto fields = withCommonNodeFields(el::LazyMapFields<LazyLayerNode>{
    {"type", {el::ValueType::String, [](const auto&) { return el::Value{"layer"}; }}},
    {"name",
     {el::ValueType::String,
      [](const auto& b) { return el::Value{b.node.layer().name()}; }}},
  });
  return fields;
}

} // namespace

el::LazyMap makeLayerNodeLazyMap(const mdl::LayerNode& node)
{
  return el::makeLazyMapOf(LazyLayerNode{node}, layerNodeLazyMapFields());
}

const kdl::flat_map<std::string, el::ValueType>& layerNodeFieldTypes()
{
  static const auto fieldTypes = el::lazyMapFieldTypes(layerNodeLazyMapFields());
  return fieldTypes;
}

} // namespace tb::ql
