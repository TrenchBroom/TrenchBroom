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

#include "mdl/Map_Layers.h"

#include "Contracts.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/EditorContext.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeQueries.h"
#include "mdl/SetCurrentLayerCommand.h"
#include "mdl/Transaction.h"

#include "kd/range_utils.h"

#include <algorithm>

namespace tb::mdl
{
namespace
{

enum class MoveDirection
{
  Up,
  Down
};

bool moveLayerByOne(Map& map, LayerNode* layerNode, const MoveDirection direction)
{
  const auto sorted = map.world()->customLayersUserSorted();

  const auto maybeIndex = kdl::index_of(sorted, layerNode);
  if (!maybeIndex.has_value())
  {
    return false;
  }

  const auto newIndex =
    static_cast<int>(*maybeIndex) + (direction == MoveDirection::Down ? 1 : -1);
  if (newIndex < 0 || newIndex >= static_cast<int>(sorted.size()))
  {
    return false;
  }

  auto* neighbourNode = sorted.at(static_cast<size_t>(newIndex));
  auto layer = layerNode->layer();
  auto neighbourLayer = neighbourNode->layer();

  const auto layerSortIndex = layer.sortIndex();
  const auto neighbourSortIndex = neighbourLayer.sortIndex();

  // Swap the sort indices of `layer` and `neighbour`
  layer.setSortIndex(neighbourSortIndex);
  neighbourLayer.setSortIndex(layerSortIndex);

  updateNodeContents(
    map,
    "Swap Layer Positions",
    {{layerNode, NodeContents(std::move(layer))},
     {neighbourNode, NodeContents(std::move(neighbourLayer))}},
    {});

  return true;
}

} // namespace

void setCurrentLayer(Map& map, LayerNode* layerNode)
{
  contract_pre(layerNode != nullptr);

  auto* currentLayer = map.editorContext().currentLayer();
  contract_assert(currentLayer != nullptr);

  auto transaction = Transaction{map, "Set Current Layer"};

  while (map.editorContext().currentGroup())
  {
    closeGroup(map);
  }

  const auto descendants = collectDescendants({currentLayer});
  downgradeShownToInherit(map, descendants);
  downgradeUnlockedToInherit(map, descendants);

  map.executeAndStore(SetCurrentLayerCommand::set(layerNode));
  transaction.commit();
}

bool canSetCurrentLayer(const Map& map, LayerNode* layerNode)
{
  return map.editorContext().currentLayer() != layerNode;
}

void renameLayer(Map& map, LayerNode* layerNode, const std::string& name)
{
  applyAndSwap(
    map,
    "Rename Layer",
    std::vector<Node*>{layerNode},
    {},
    kdl::overload(
      [&](Layer& layer) {
        layer.setName(name);
        return true;
      },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

void moveLayer(Map& map, LayerNode* layer, const int offset)
{
  contract_pre(layer != map.world()->defaultLayer());

  auto transaction = Transaction{map, "Move Layer"};

  const auto direction = (offset > 0) ? MoveDirection::Down : MoveDirection::Up;
  for (int i = 0; i < std::abs(offset); ++i)
  {
    if (!moveLayerByOne(map, layer, direction))
    {
      break;
    }
  }

  transaction.commit();
}

bool canMoveLayer(const Map& map, LayerNode* layerNode, const int offset)
{
  contract_pre(layerNode != nullptr);

  auto* worldNode = map.world();
  if (layerNode == worldNode->defaultLayer())
  {
    return false;
  }

  const auto sorted = worldNode->customLayersUserSorted();
  const auto maybeIndex = kdl::index_of(sorted, layerNode);
  if (!maybeIndex.has_value())
  {
    return false;
  }

  const auto newIndex = static_cast<int>(*maybeIndex) + offset;
  return (newIndex >= 0 && newIndex < static_cast<int>(sorted.size()));
}

void moveSelectedNodesToLayer(Map& map, LayerNode* layerNode)
{
  const auto& selectedNodes = map.selection().nodes;

  auto nodesToMove = std::vector<Node*>{};
  auto nodesToSelect = std::vector<Node*>{};

  const auto addBrushOrPatchNode = [&](auto* node) {
    assert(node->selected());

    if (!node->containedInGroup())
    {
      auto* entityNode = node->entity();
      if (entityNode == map.world())
      {
        nodesToMove.push_back(node);
        nodesToSelect.push_back(node);
      }
      else
      {
        if (!kdl::vec_contains(nodesToMove, entityNode))
        {
          nodesToMove.push_back(entityNode);
          nodesToSelect =
            kdl::vec_concat(std::move(nodesToSelect), entityNode->children());
        }
      }
    }
  };

  for (auto* node : selectedNodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* groupNode) {
        assert(groupNode->selected());

        if (!groupNode->containedInGroup())
        {
          nodesToMove.push_back(groupNode);
          nodesToSelect.push_back(groupNode);
        }
      },
      [&](EntityNode* entityNode) {
        assert(entityNode->selected());

        if (!entityNode->containedInGroup())
        {
          nodesToMove.push_back(entityNode);
          nodesToSelect.push_back(entityNode);
        }
      },
      [&](BrushNode* brushNode) { addBrushOrPatchNode(brushNode); },
      [&](PatchNode* patchNode) { addBrushOrPatchNode(patchNode); }));
  }

  if (!nodesToMove.empty())
  {
    auto transaction = Transaction{map, "Move Nodes to " + layerNode->name()};
    deselectAll(map);
    if (!reparentNodes(map, {{layerNode, nodesToMove}}))
    {
      transaction.cancel();
      return;
    }
    if (!layerNode->hidden() && !layerNode->locked())
    {
      selectNodes(map, nodesToSelect);
    }
    transaction.commit();
  }
}

bool canMoveSelectedNodesToLayer(const Map& map, LayerNode* layerNode)
{
  contract_pre(layerNode != nullptr);

  const auto& nodes = map.selection().nodes;

  const auto isAnyNodeInGroup = std::ranges::any_of(
    nodes, [&](auto* node) { return findContainingGroup(node) != nullptr; });
  const auto isAnyNodeInOtherLayer = std::ranges::any_of(
    nodes, [&](auto* node) { return findContainingLayer(node) != layerNode; });

  return !nodes.empty() && !isAnyNodeInGroup && isAnyNodeInOtherLayer;
}

void hideLayers(Map& map, const std::vector<LayerNode*>& layers)
{
  auto transaction = Transaction{map, "Hide Layers"};
  hideNodes(map, kdl::vec_static_cast<Node*>(layers));
  transaction.commit();
}

bool canHideLayers(const std::vector<LayerNode*>& layers)
{
  return std::ranges::any_of(layers, [](const auto* layer) { return layer->visible(); });
}

void isolateLayers(Map& map, const std::vector<LayerNode*>& layers)
{
  const auto allLayers = map.world()->allLayers();

  auto transaction = Transaction{map, "Isolate Layers"};
  hideNodes(map, kdl::vec_static_cast<Node*>(allLayers));
  showNodes(map, kdl::vec_static_cast<Node*>(layers));
  transaction.commit();
}

bool canIsolateLayers(const Map& map, const std::vector<LayerNode*>& layers)
{
  const auto allLayers = map.world()->allLayers();
  return std::ranges::any_of(allLayers, [&](const auto* layer) {
    return kdl::vec_contains(layers, layer) != layer->visible();
  });
}

void setOmitLayerFromExport(Map& map, LayerNode* layerNode, const bool omitFromExport)
{
  const auto commandName =
    omitFromExport ? "Omit Layer from Export" : "Include Layer in Export";

  auto layer = layerNode->layer();
  layer.setOmitFromExport(omitFromExport);
  updateNodeContents(map, commandName, {{layerNode, NodeContents(std::move(layer))}}, {});
}

} // namespace tb::mdl
