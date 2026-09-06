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

#include "mdl/NodeVariableStore.h"

#include "mdl/BrushNode.h"
#include "mdl/BrushNodeBoundValue.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBoundValue.h"
#include "mdl/GroupNode.h"
#include "mdl/GroupNodeBoundValue.h"
#include "mdl/LayerNode.h"
#include "mdl/LayerNodeBoundValue.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/PatchNodeBoundValue.h"
#include "mdl/WorldNode.h"
#include "mdl/WorldNodeBoundValue.h"

#include "kd/overload.h"

namespace tb::mdl
{

el::BoundValueStore makeNodeVariableStore(const Map& map, const Node& node)
{
  return el::BoundValueStore{node.accept(kdl::overload(
    [&](const WorldNode& worldNode) { return makeWorldNodeBoundValue(map, worldNode); },
    [&](const LayerNode& layerNode) { return makeLayerNodeBoundValue(map, layerNode); },
    [&](const GroupNode& groupNode) { return makeGroupNodeBoundValue(map, groupNode); },
    [&](const EntityNode& entityNode) {
      return makeEntityNodeBoundValue(map, entityNode);
    },
    [&](const BrushNode& brushNode) { return makeBrushNodeBoundValue(map, brushNode); },
    [&](const PatchNode& patchNode) {
      return makePatchNodeBoundValue(map, patchNode);
    }))};
}

} // namespace tb::mdl
