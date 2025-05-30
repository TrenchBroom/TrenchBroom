/*
 Copyright (C) 2010 Kristian Duske

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

#include "Selection.h"

#include "mdl/BrushFace.h" // IWYU pragma: keep
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"

namespace tb::mdl
{

kdl_reflect_impl(Selection);

bool Selection::hasAny() const
{
  return hasNodes() || hasBrushFaces();
}

bool Selection::hasNodes() const
{
  return !nodes.empty();
}

bool Selection::hasGroups() const
{
  return !groups.empty();
}

bool Selection::hasOnlyGroups() const
{
  return hasNodes() && nodes.size() == groups.size();
}

bool Selection::hasEntities() const
{
  return !entities.empty();
}

bool Selection::hasOnlyEntities() const
{
  return hasNodes() && nodes.size() == entities.size();
}

bool Selection::hasBrushes() const
{
  return !brushes.empty();
}

bool Selection::hasOnlyBrushes() const
{
  return hasNodes() && nodes.size() == brushes.size();
}

bool Selection::hasPatches() const
{
  return !patches.empty();
}

bool Selection::hasOnlyPatches() const
{
  return hasNodes() && nodes.size() == patches.size();
}

bool Selection::hasBrushFaces() const
{
  return !brushFaces.empty();
}

Selection computeSelection(WorldNode& rootNode)
{
  auto selection = Selection{};

  rootNode.accept(kdl::overload(
    [&](auto&& thisLambda, mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
      if (groupNode->selected())
      {
        selection.nodes.push_back(groupNode);
        selection.groups.push_back(groupNode);
      }
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* entityNode) {
      if (entityNode->selected())
      {
        selection.nodes.push_back(entityNode);
        selection.entities.push_back(entityNode);
      }
      entityNode->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* brushNode) {
      if (brushNode->selected())
      {
        selection.nodes.push_back(brushNode);
        selection.brushes.push_back(brushNode);
      }

      const auto& faces = brushNode->brush().faces();
      for (size_t i = 0; i < faces.size(); ++i)
      {
        if (faces[i].selected())
        {
          selection.brushFaces.emplace_back(brushNode, i);
        }
      }
    },
    [&](mdl::PatchNode* patchNode) {
      if (patchNode->selected())
      {
        selection.nodes.push_back(patchNode);
        selection.patches.push_back(patchNode);
      }
    }));

  return selection;
}

} // namespace tb::mdl
