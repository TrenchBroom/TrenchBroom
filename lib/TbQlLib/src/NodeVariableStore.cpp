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

#include "ql/NodeVariableStore.h"

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ql/BrushNodeLazyMap.h"
#include "ql/EntityNodeLazyMap.h"
#include "ql/GroupNodeLazyMap.h"
#include "ql/LayerNodeLazyMap.h"
#include "ql/PatchNodeLazyMap.h"
#include "ql/WorldNodeLazyMap.h"

#include "kd/overload.h"

namespace tb::ql
{

el::LazyMapVariableStore makeNodeVariableStore(const mdl::Map& map, const mdl::Node& node)
{
  return el::LazyMapVariableStore{node.accept(kdl::overload(
    [&](const mdl::WorldNode& worldNode) { return makeWorldNodeLazyMap(map, worldNode); },
    [&](const mdl::LayerNode& layerNode) { return makeLayerNodeLazyMap(layerNode); },
    [&](const mdl::GroupNode& groupNode) { return makeGroupNodeLazyMap(groupNode); },
    [&](const mdl::EntityNode& entityNode) {
      return makeEntityNodeLazyMap(map, entityNode);
    },
    [&](const mdl::BrushNode& brushNode) { return makeBrushNodeLazyMap(map, brushNode); },
    [&](const mdl::PatchNode& patchNode) {
      return makePatchNodeLazyMap(map, patchNode);
    }))};
}

} // namespace tb::ql
