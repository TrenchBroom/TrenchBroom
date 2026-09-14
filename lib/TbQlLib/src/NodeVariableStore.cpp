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
#include "ql/BrushNodeBinding.h"
#include "ql/EntityNodeBinding.h"
#include "ql/GroupNodeBinding.h"
#include "ql/LayerNodeBinding.h"
#include "ql/PatchNodeBinding.h"
#include "ql/WorldNodeBinding.h"

#include "kd/overload.h"

namespace tb::ql
{

el::BoundValueStore makeNodeVariableStore(const mdl::Map& map, const mdl::Node& node)
{
  return el::BoundValueStore{node.accept(kdl::overload(
    [&](const mdl::WorldNode& worldNode) { return makeWorldNodeBinding(map, worldNode); },
    [&](const mdl::LayerNode& layerNode) { return makeLayerNodeBinding(map, layerNode); },
    [&](const mdl::GroupNode& groupNode) { return makeGroupNodeBinding(map, groupNode); },
    [&](const mdl::EntityNode& entityNode) {
      return makeEntityNodeBinding(map, entityNode);
    },
    [&](const mdl::BrushNode& brushNode) { return makeBrushNodeBinding(map, brushNode); },
    [&](const mdl::PatchNode& patchNode) {
      return makePatchNodeBinding(map, patchNode);
    }))};
}

} // namespace tb::ql
