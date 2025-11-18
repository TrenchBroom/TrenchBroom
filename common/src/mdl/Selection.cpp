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
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Node.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include <ranges>

namespace tb::mdl
{
namespace
{

std::vector<EntityNodeBase*> computeAllEntities(
  const Selection& selection, WorldNode& worldNode)
{
  if (!selection.hasAny())
  {
    return {&worldNode};
  }

  auto result = std::vector<EntityNodeBase*>{};
  for (auto* node : selection.nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](auto&& thisLambda, GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [&](EntityNode* entityNode) { result.push_back(entityNode); },
      [&](BrushNode* brushNode) { result.push_back(brushNode->entity()); },
      [&](PatchNode* patchNode) { result.push_back(patchNode->entity()); }));
  }

  if (result.empty())
  {
    return {&worldNode};
  }

  result = kdl::vec_sort_and_remove_duplicates(std::move(result));
  if (result.size() > 1)
  {
    // filter out worldspawn
    result = result | std::views::filter([](const auto* entityNode) {
               return entityNode->entity().classname()
                      != EntityPropertyValues::WorldspawnClassname;
             })
             | kdl::ranges::to<std::vector>();
  }

  return result;
}

std::vector<BrushNode*> computeAllBrushes(const Selection& selection)
{
  auto result = std::vector<BrushNode*>{};

  for (auto* node : selection.nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](auto&& thisLambda, GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, EntityNode* entityNode) {
        entityNode->visitChildren(thisLambda);
      },
      [&](BrushNode* brushNode) { result.push_back(brushNode); },
      [&](PatchNode*) {}));
  }

  return result;
}

std::vector<BrushFaceHandle> computeAllBrushFaces(
  const Selection& selection, WorldNode& worldNode)
{
  if (selection.hasBrushFaces())
  {
    return selection.brushFaces;
  }

  const auto faces = collectBrushFaces(selection.nodes);
  return faceSelectionWithLinkedGroupConstraints(worldNode, faces).facesToSelect;
}

} // namespace

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

bool Selection::hasAnyBrushFaces() const
{
  return hasBrushFaces() || hasBrushes();
}

const std::vector<EntityNodeBase*>& Selection::allEntities() const
{
  return cachedAllEntities;
}

const std::vector<BrushNode*>& Selection::allBrushes() const
{
  return cachedAllBrushes;
}

const std::vector<BrushFaceHandle>& Selection::allBrushFaces() const
{
  return cachedAllBrushFaces;
}

Selection computeSelection(WorldNode& rootNode)
{
  auto selection = Selection{};

  rootNode.accept(kdl::overload(
    [&](
      auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [&](
      auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      if (groupNode->selected())
      {
        selection.nodes.push_back(groupNode);
        selection.groups.push_back(groupNode);
      }
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entityNode) {
      if (entityNode->selected())
      {
        selection.nodes.push_back(entityNode);
        selection.entities.push_back(entityNode);
      }
      entityNode->visitChildren(thisLambda);
    },
    [&](BrushNode* brushNode) {
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
    [&](PatchNode* patchNode) {
      if (patchNode->selected())
      {
        selection.nodes.push_back(patchNode);
        selection.patches.push_back(patchNode);
      }
    }));

  selection.cachedAllEntities = computeAllEntities(selection, rootNode);
  selection.cachedAllBrushes = computeAllBrushes(selection);
  selection.cachedAllBrushFaces = computeAllBrushFaces(selection, rootNode);

  return selection;
}

} // namespace tb::mdl
