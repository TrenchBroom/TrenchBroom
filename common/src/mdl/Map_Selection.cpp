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

#include "Logger.h"
#include "Map.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
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

const Selection& Map::selection() const
{
  if (!m_cachedSelection)
  {
    m_cachedSelection = m_world ? computeSelection(*m_world.get()) : Selection{};
  }
  return *m_cachedSelection;
}

void Map::selectAllNodes()
{
  executeAndStore(SelectionCommand::selectAllNodes());
}

void Map::selectNodes(const std::vector<Node*>& nodes)
{
  executeAndStore(SelectionCommand::select(nodes));
}

void Map::selectSiblingNodes()
{
  const auto& nodes = selection().nodes;
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
        collectSelectableNodes(parent->children(), editorContext()));
    }
  }

  auto transaction = Transaction{*this, "Select Siblings"};
  deselectAll();
  selectNodes(nodesToSelect);
  transaction.commit();
}

void Map::selectTouchingNodes(const bool del)
{
  auto nodes = collectTouchingNodes({world()}, selection().brushes)
               | std::views::filter(
                 [&](const auto* node) { return editorContext().selectable(*node); })
               | kdl::to_vector;

  auto transaction = Transaction{*this, "Select Touching"};
  if (del)
  {
    removeSelectedNodes(*this);
  }
  else
  {
    deselectAll();
  }
  selectNodes(nodes);
  transaction.commit();
}

void Map::selectTouchingNodes(const vm::axis::type cameraAxis, const bool del)
{
  const auto cameraAbsDirection = vm::vec3d::axis(cameraAxis);
  // we can't make a brush that is exactly as large as worldBounds
  const auto tallBounds = worldBounds().expand(-1.0);

  const auto min = vm::dot(tallBounds.min, cameraAbsDirection);
  const auto max = vm::dot(tallBounds.max, cameraAbsDirection);

  const auto minPlane = vm::plane3d{min, cameraAbsDirection};
  const auto maxPlane = vm::plane3d{max, cameraAbsDirection};

  const auto& selectionBrushNodes = selection().brushes;
  assert(!selectionBrushNodes.empty());

  const auto brushBuilder = BrushBuilder{world()->mapFormat(), worldBounds()};

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
        auto transaction = Transaction{*this, "Select Tall"};
        if (del)
        {
          removeSelectedNodes(*this);
        }
        else
        {
          deselectAll();
        }

        const auto nodesToSelect =
          collectContainedNodes(
            {world()}, tallBrushes | std::views::transform([](const auto& b) {
                         return b.get();
                       }) | kdl::to_vector)
          | std::views::filter(
            [&](const auto* node) { return editorContext().selectable(*node); })
          | kdl::to_vector;
        selectNodes(nodesToSelect);

        transaction.commit();
      })
    | kdl::transform_error(
      [&](auto e) { logger().error() << "Could not create selection brush: " << e.msg; });
}

void Map::selectContainedNodes(const bool del)
{
  auto nodes = collectContainedNodes({world()}, selection().brushes)
               | std::views::filter(
                 [&](const auto* node) { return editorContext().selectable(*node); })
               | kdl::to_vector;

  auto transaction = Transaction{*this, "Select Inside"};
  if (del)
  {
    removeSelectedNodes(*this);
  }
  else
  {
    deselectAll();
  }
  selectNodes(nodes);
  transaction.commit();
}

void Map::selectNodesWithFilePosition(const std::vector<size_t>& positions)
{
  auto nodesToSelect = std::vector<Node*>{};
  const auto hasFilePosition = [&](const auto* node) {
    return std::any_of(positions.begin(), positions.end(), [&](const auto position) {
      return node->containsLine(position);
    });
  };

  world()->accept(kdl::overload(
    [&](
      auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [&](
      auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      if (hasFilePosition(groupNode))
      {
        if (editorContext().selectable(*groupNode))
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
        if (editorContext().selectable(*entityNode))
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
              collectSelectableNodes(entityNode->children(), editorContext()));
          }
        }
      }
    },
    [&](BrushNode* brushNode) {
      if (hasFilePosition(brushNode) && editorContext().selectable(*brushNode))
      {
        nodesToSelect.push_back(brushNode);
      }
    },
    [&](PatchNode* patchNode) {
      if (hasFilePosition(patchNode) && editorContext().selectable(*patchNode))
      {
        nodesToSelect.push_back(patchNode);
      }
    }));

  auto transaction = Transaction{*this, "Select by Line Number"};
  deselectAll();
  selectNodes(nodesToSelect);
  transaction.commit();
}

void Map::selectBrushesWithMaterial(const Material* material)
{
  const auto brushes =
    collectSelectableNodes(std::vector<Node*>{world()}, editorContext())
    | std::views::filter([&](const auto& node) {
        return std::ranges::any_of(
          collectSelectableBrushFaces({node}, editorContext()),
          [&](const auto& h) { return h.face().material() == material; });
      })
    | kdl::to_vector;

  auto transaction = Transaction{*this, "Select Brushes with Material"};
  deselectAll();
  selectNodes(brushes);
  transaction.commit();
}

void Map::invertNodeSelection()
{
  // This only selects nodes that have no selected children (or parents).
  // This is because if a brush entity only 1 selected child and 1 unselected,
  // we treat it as partially selected and don't want to try to select the entity if the
  // selection is inverted, which would reselect both children.

  auto nodesToSelect = std::vector<Node*>{};
  const auto collectNode = [&](auto* node) {
    if (
      !node->transitivelySelected() && !node->descendantSelected()
      && editorContext().selectable(*node))
    {
      nodesToSelect.push_back(node);
    }
  };

  currentGroupOrWorld()->accept(kdl::overload(
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

  auto transaction = Transaction{*this, "Select Inverse"};
  deselectAll();
  selectNodes(nodesToSelect);
  transaction.commit();
}

void Map::selectAllInLayers(const std::vector<LayerNode*>& layers)
{
  const auto nodes =
    collectSelectableNodes(kdl::vec_static_cast<Node*>(layers), editorContext());

  deselectAll();
  selectNodes(nodes);
}

bool Map::canSelectAllInLayers(const std::vector<LayerNode*>& /* layers */) const
{
  return editorContext().canChangeSelection();
}

bool Map::canSelectLinkedGroups() const
{
  if (!selection().hasOnlyGroups())
  {
    return false;
  }

  const auto allLinkIds = kdl::vec_sort(kdl::vec_transform(
    collectGroups({world()}), [](const auto& groupNode) { return groupNode->linkId(); }));

  return kdl::all_of(selection().groups, [&](const auto* groupNode) {
    const auto [iBegin, iEnd] =
      std::equal_range(allLinkIds.begin(), allLinkIds.end(), groupNode->linkId());
    return std::distance(iBegin, iEnd) > 1;
  });
}

void Map::selectLinkedGroups()
{
  if (!canSelectLinkedGroups())
  {
    return;
  }

  const auto linkIdsToSelect = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    selection().groups, [](const auto* groupNode) { return groupNode->linkId(); }));
  const auto groupNodesToSelect =
    kdl::vec_flatten(kdl::vec_transform(linkIdsToSelect, [&](const auto& linkId) {
      return collectNodesWithLinkId({world()}, linkId);
    }));

  auto transaction = Transaction{*this, "Select Linked Groups"};
  deselectAll();
  selectNodes(groupNodesToSelect);
  transaction.commit();
}

void Map::selectBrushFaces(const std::vector<BrushFaceHandle>& handles)
{
  executeAndStore(SelectionCommand::select(handles));
  if (!handles.empty())
  {
    setCurrentMaterialName(handles.back().face().attributes().materialName());
  }
}

void Map::selectBrushFacesWithMaterial(const Material* material)
{
  const auto faces =
    collectSelectableBrushFaces(std::vector<Node*>{world()}, editorContext())
    | std::views::filter([&](const auto& h) { return h.face().material() == material; })
    | kdl::to_vector;

  auto transaction = Transaction{*this, "Select Faces with Material"};
  deselectAll();
  selectBrushFaces(faces);
  transaction.commit();
}

void Map::convertToFaceSelection()
{
  executeAndStore(SelectionCommand::convertToFaces());
}

void Map::deselectAll()
{
  if (selection().hasAny())
  {
    executeAndStore(SelectionCommand::deselectAll());
  }
}

void Map::deselectNodes(const std::vector<Node*>& nodes)
{
  executeAndStore(SelectionCommand::deselect(nodes));
}

void Map::deselectBrushFaces(const std::vector<BrushFaceHandle>& handles)
{
  executeAndStore(SelectionCommand::deselect(handles));
}

const vm::bbox3d Map::referenceBounds() const
{
  if (const auto bounds = selectionBounds())
  {
    return *bounds;
  }
  if (const auto bounds = lastSelectionBounds())
  {
    return *bounds;
  }
  return vm::bbox3d{16.0};
}

const std::optional<vm::bbox3d>& Map::lastSelectionBounds() const
{
  return m_lastSelectionBounds;
}

const std::optional<vm::bbox3d>& Map::selectionBounds() const
{
  if (!m_cachedSelectionBounds && selection().hasNodes())
  {
    m_cachedSelectionBounds = computeLogicalBounds(selection().nodes);
  }
  return m_cachedSelectionBounds;
}

} // namespace tb::mdl
