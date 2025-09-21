/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/Map_Selection.h"

#include "Logger.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/SelectionCommand.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kdl/range_to_vector.h"
#include "kdl/result_fold.h"

#include <ranges>
#include <unordered_set>
#include <vector>

namespace tb::mdl
{

void selectAllNodes(Map& map)
{
  map.executeAndStore(SelectionCommand::selectAllNodes());
}

void selectNodes(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SelectionCommand::select(nodes));
}

void selectSiblingNodes(Map& map)
{
  const auto& nodes = map.selection().nodes;
  if (nodes.empty())
  {
    return;
  }

  auto visited = std::unordered_set<Node*>{};
  auto nodesToSelect = std::vector<Node*>{};

  for (auto* node : nodes)
  {
    auto* parent = node->parent();
    if (visited.insert(parent).second)
    {
      nodesToSelect = kdl::vec_concat(
        std::move(nodesToSelect),
        collectSelectableNodes(parent->children(), map.editorContext()));
    }
  }

  auto transaction = Transaction{map, "Select Siblings"};
  deselectAll(map);
  selectNodes(map, nodesToSelect);
  transaction.commit();
}

void selectTouchingNodes(Map& map, const bool del)
{
  auto nodes = collectTouchingNodes({map.world()}, map.selection().brushes)
               | std::views::filter(
                 [&](const auto* node) { return map.editorContext().selectable(*node); })
               | kdl::to_vector;

  auto transaction = Transaction{map, "Select Touching"};
  if (del)
  {
    removeSelectedNodes(map);
  }
  else
  {
    deselectAll(map);
  }
  selectNodes(map, nodes);
  transaction.commit();
}

void selectTouchingNodes(Map& map, const vm::axis::type cameraAxis, const bool del)
{
  const auto cameraAbsDirection = vm::vec3d::axis(cameraAxis);
  // we can't make a brush that is exactly as large as worldBounds
  const auto tallBounds = map.worldBounds().expand(-1.0);

  const auto min = vm::dot(tallBounds.min, cameraAbsDirection);
  const auto max = vm::dot(tallBounds.max, cameraAbsDirection);

  const auto minPlane = vm::plane3d{min, cameraAbsDirection};
  const auto maxPlane = vm::plane3d{max, cameraAbsDirection};

  const auto& selectionBrushNodes = map.selection().brushes;
  assert(!selectionBrushNodes.empty());

  const auto brushBuilder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

  kdl::vec_transform(
    selectionBrushNodes,
    [&](const auto* selectionBrushNode) {
      const auto& selectionBrush = selectionBrushNode->brush();

      auto tallVertices = std::vector<vm::vec3d>{};
      tallVertices.reserve(2 * selectionBrush.vertexCount());

      for (const auto* vertex : selectionBrush.vertices())
      {
        tallVertices.push_back(minPlane.project_point(vertex->position()));
        tallVertices.push_back(maxPlane.project_point(vertex->position()));
      }

      return brushBuilder.createBrush(tallVertices, BrushFaceAttributes::NoMaterialName)
             | kdl::transform(
               [](auto brush) { return std::make_unique<BrushNode>(std::move(brush)); });
    })
    | kdl::fold | kdl::transform([&](const auto& tallBrushes) {
        // delete the original selection brushes before searching for the objects to
        // select
        auto transaction = Transaction{map, "Select Tall"};
        if (del)
        {
          removeSelectedNodes(map);
        }
        else
        {
          deselectAll(map);
        }

        const auto nodesToSelect =
          collectContainedNodes(
            {map.world()}, tallBrushes | std::views::transform([](const auto& b) {
                             return b.get();
                           }) | kdl::to_vector)
          | std::views::filter(
            [&](const auto* node) { return map.editorContext().selectable(*node); })
          | kdl::to_vector;
        selectNodes(map, nodesToSelect);

        transaction.commit();
      })
    | kdl::transform_error([&](auto e) {
        map.logger().error() << "Could not create selection brush: " << e.msg;
      });
}

void selectContainedNodes(Map& map, const bool del)
{
  auto nodes = collectContainedNodes({map.world()}, map.selection().brushes)
               | std::views::filter(
                 [&](const auto* node) { return map.editorContext().selectable(*node); })
               | kdl::to_vector;

  auto transaction = Transaction{map, "Select Inside"};
  if (del)
  {
    removeSelectedNodes(map);
  }
  else
  {
    deselectAll(map);
  }
  selectNodes(map, nodes);
  transaction.commit();
}

void selectNodesWithFilePosition(Map& map, const std::vector<size_t>& positions)
{
  auto nodesToSelect = std::vector<Node*>{};
  const auto hasFilePosition = [&](const auto* node) {
    return std::any_of(positions.begin(), positions.end(), [&](const auto position) {
      return node->containsLine(position);
    });
  };

  map.world()->accept(kdl::overload(
    [&](
      auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [&](
      auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      if (hasFilePosition(groupNode))
      {
        if (map.editorContext().selectable(*groupNode))
        {
          nodesToSelect.push_back(groupNode);
        }
        else
        {
          groupNode->visitChildren(thisLambda);
        }
      }
    },
    [&](auto&& thisLambda, EntityNode* entityNode) {
      if (hasFilePosition(entityNode))
      {
        if (map.editorContext().selectable(*entityNode))
        {
          nodesToSelect.push_back(entityNode);
        }
        else
        {
          const auto previousCount = nodesToSelect.size();
          entityNode->visitChildren(thisLambda);
          if (previousCount == nodesToSelect.size())
          {
            // no child was selected, select all children
            nodesToSelect = kdl::vec_concat(
              std::move(nodesToSelect),
              collectSelectableNodes(entityNode->children(), map.editorContext()));
          }
        }
      }
    },
    [&](BrushNode* brushNode) {
      if (hasFilePosition(brushNode) && map.editorContext().selectable(*brushNode))
      {
        nodesToSelect.push_back(brushNode);
      }
    },
    [&](PatchNode* patchNode) {
      if (hasFilePosition(patchNode) && map.editorContext().selectable(*patchNode))
      {
        nodesToSelect.push_back(patchNode);
      }
    }));

  auto transaction = Transaction{map, "Select by Line Number"};
  deselectAll(map);
  selectNodes(map, nodesToSelect);
  transaction.commit();
}

void selectBrushesWithMaterial(Map& map, const Material* material)
{
  const auto brushes =
    collectSelectableNodes(std::vector<Node*>{map.world()}, map.editorContext())
    | std::views::filter([&](const auto& node) {
        return std::ranges::any_of(
          collectSelectableBrushFaces({node}, map.editorContext()),
          [&](const auto& h) { return h.face().material() == material; });
      })
    | kdl::to_vector;

  auto transaction = Transaction{map, "Select Brushes with Material"};
  deselectAll(map);
  selectNodes(map, brushes);
  transaction.commit();
}

void invertNodeSelection(Map& map)
{
  // This only selects nodes that have no selected children (or parents).
  // This is because if a brush entity only 1 selected child and 1 unselected,
  // we treat it as partially selected and don't want to try to select the entity if the
  // selection is inverted, which would reselect both children.

  auto nodesToSelect = std::vector<Node*>{};
  const auto collectNode = [&](auto* node) {
    if (
      !node->transitivelySelected() && !node->descendantSelected()
      && map.editorContext().selectable(*node))
    {
      nodesToSelect.push_back(node);
    }
  };

  currentGroupOrWorld(map)->accept(kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      collectNode(groupNode);
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entityNode) {
      collectNode(entityNode);
      entityNode->visitChildren(thisLambda);
    },
    [&](BrushNode* brushNode) { collectNode(brushNode); },
    [&](PatchNode* patchNode) { collectNode(patchNode); }));

  auto transaction = Transaction{map, "Select Inverse"};
  deselectAll(map);
  selectNodes(map, nodesToSelect);
  transaction.commit();
}

void selectAllInLayers(Map& map, const std::vector<LayerNode*>& layers)
{
  const auto nodes =
    collectSelectableNodes(kdl::vec_static_cast<Node*>(layers), map.editorContext());

  deselectAll(map);
  selectNodes(map, nodes);
}

bool canSelectAllInLayers(const Map& map, const std::vector<LayerNode*>&)
{
  return map.editorContext().canChangeSelection();
}

void selectLinkedGroups(Map& map)
{
  if (!canSelectLinkedGroups(map))
  {
    return;
  }

  const auto linkIdsToSelect = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    map.selection().groups, [](const auto* groupNode) { return groupNode->linkId(); }));
  const auto groupNodesToSelect =
    kdl::vec_flatten(kdl::vec_transform(linkIdsToSelect, [&](const auto& linkId) {
      return collectNodesWithLinkId({map.world()}, linkId);
    }));

  auto transaction = Transaction{map, "Select Linked Groups"};
  deselectAll(map);
  selectNodes(map, groupNodesToSelect);
  transaction.commit();
}

bool canSelectLinkedGroups(const Map& map)
{
  if (!map.selection().hasOnlyGroups())
  {
    return false;
  }

  const auto allLinkIds = kdl::vec_sort(kdl::vec_transform(
    collectGroups({map.world()}),
    [](const auto& groupNode) { return groupNode->linkId(); }));

  return kdl::all_of(map.selection().groups, [&](const auto* groupNode) {
    const auto [iBegin, iEnd] =
      std::equal_range(allLinkIds.begin(), allLinkIds.end(), groupNode->linkId());
    return std::distance(iBegin, iEnd) > 1;
  });
}

void selectBrushFaces(Map& map, const std::vector<BrushFaceHandle>& handles)
{
  map.executeAndStore(SelectionCommand::select(handles));
  if (!handles.empty())
  {
    map.setCurrentMaterialName(handles.back().face().attributes().materialName());
  }
}

void selectBrushFacesWithMaterial(Map& map, const Material* material)
{
  const auto faces =
    collectSelectableBrushFaces(std::vector<Node*>{map.world()}, map.editorContext())
    | std::views::filter([&](const auto& h) { return h.face().material() == material; })
    | kdl::to_vector;

  auto transaction = Transaction{map, "Select Faces with Material"};
  deselectAll(map);
  selectBrushFaces(map, faces);
  transaction.commit();
}

void convertToFaceSelection(Map& map)
{
  map.executeAndStore(SelectionCommand::convertToFaces());
}

void deselectAll(Map& map)
{
  if (map.selection().hasAny())
  {
    map.executeAndStore(SelectionCommand::deselectAll());
  }
}

void deselectNodes(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SelectionCommand::deselect(nodes));
}

void deselectBrushFaces(Map& map, const std::vector<BrushFaceHandle>& handles)
{
  map.executeAndStore(SelectionCommand::deselect(handles));
}

} // namespace tb::mdl
