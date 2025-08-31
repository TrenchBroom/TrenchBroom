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

#include "Ensure.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/EditorContext.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/NodeQueries.h"
#include "mdl/SetCurrentLayerCommand.h"
#include "mdl/Transaction.h"

#include "kdl/range_utils.h"

#include <algorithm>

namespace tb::mdl
{

LayerNode* Map::currentLayer() const
{
  return editorContext().currentLayer();
}

void Map::setCurrentLayer(LayerNode* layerNode)
{
  ensure(currentLayer() != nullptr, "old currentLayer is not null");
  ensure(layerNode != nullptr, "new currentLayer is not null");

  auto transaction = Transaction{*this, "Set Current Layer"};

  while (editorContext().currentGroup())
  {
    closeGroup(*this);
  }

  const auto descendants = collectDescendants({currentLayer()});
  downgradeShownToInherit(descendants);
  downgradeUnlockedToInherit(descendants);

  executeAndStore(SetCurrentLayerCommand::set(layerNode));
  transaction.commit();
}

bool Map::canSetCurrentLayer(LayerNode* layerNode) const
{
  return currentLayer() != layerNode;
}

void Map::renameLayer(LayerNode* layerNode, const std::string& name)
{
  applyAndSwap(
    *this,
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

enum class Map::MoveDirection
{
  Up,
  Down
};

void Map::moveLayer(LayerNode* layer, const int offset)
{
  ensure(layer != world()->defaultLayer(), "attempted to move default layer");

  auto transaction = Transaction{*this, "Move Layer"};

  const auto direction = (offset > 0) ? Map::MoveDirection::Down : MoveDirection::Up;
  for (int i = 0; i < std::abs(offset); ++i)
  {
    if (!moveLayerByOne(layer, direction))
    {
      break;
    }
  }

  transaction.commit();
}

bool Map::canMoveLayer(LayerNode* layer, const int offset) const
{
  ensure(layer != nullptr, "null layer");

  WorldNode* world = this->world();
  if (layer == world->defaultLayer())
  {
    return false;
  }

  const auto sorted = world->customLayersUserSorted();
  const auto maybeIndex = kdl::index_of(sorted, layer);
  if (!maybeIndex.has_value())
  {
    return false;
  }

  const auto newIndex = static_cast<int>(*maybeIndex) + offset;
  return (newIndex >= 0 && newIndex < static_cast<int>(sorted.size()));
}

void Map::moveSelectedNodesToLayer(LayerNode* layer)
{
  const auto& selectedNodes = this->selection().nodes;

  auto nodesToMove = std::vector<Node*>{};
  auto nodesToSelect = std::vector<Node*>{};

  const auto addBrushOrPatchNode = [&](auto* node) {
    assert(node->selected());

    if (!node->containedInGroup())
    {
      auto* entity = node->entity();
      if (entity == world())
      {
        nodesToMove.push_back(node);
        nodesToSelect.push_back(node);
      }
      else
      {
        if (!kdl::vec_contains(nodesToMove, entity))
        {
          nodesToMove.push_back(entity);
          nodesToSelect = kdl::vec_concat(std::move(nodesToSelect), entity->children());
        }
      }
    }
  };

  for (auto* node : selectedNodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* group) {
        assert(group->selected());

        if (!group->containedInGroup())
        {
          nodesToMove.push_back(group);
          nodesToSelect.push_back(group);
        }
      },
      [&](EntityNode* entity) {
        assert(entity->selected());

        if (!entity->containedInGroup())
        {
          nodesToMove.push_back(entity);
          nodesToSelect.push_back(entity);
        }
      },
      [&](BrushNode* brush) { addBrushOrPatchNode(brush); },
      [&](PatchNode* patch) { addBrushOrPatchNode(patch); }));
  }

  if (!nodesToMove.empty())
  {
    auto transaction = Transaction{*this, "Move Nodes to " + layer->name()};
    deselectAll();
    if (!reparentNodes(*this, {{layer, nodesToMove}}))
    {
      transaction.cancel();
      return;
    }
    if (!layer->hidden() && !layer->locked())
    {
      selectNodes(nodesToSelect);
    }
    transaction.commit();
  }
}

bool Map::canMoveSelectedNodesToLayer(LayerNode* layer) const
{
  ensure(layer != nullptr, "null layer");
  const auto& nodes = selection().nodes;

  const auto isAnyNodeInGroup = std::ranges::any_of(
    nodes, [&](auto* node) { return findContainingGroup(node) != nullptr; });
  const auto isAnyNodeInOtherLayer = std::ranges::any_of(
    nodes, [&](auto* node) { return findContainingLayer(node) != layer; });

  return !nodes.empty() && !isAnyNodeInGroup && isAnyNodeInOtherLayer;
}

void Map::hideLayers(const std::vector<LayerNode*>& layers)
{
  auto transaction = Transaction{*this, "Hide Layers"};
  hideNodes(kdl::vec_static_cast<Node*>(layers));
  transaction.commit();
}

bool Map::canHideLayers(const std::vector<LayerNode*>& layers) const
{
  return std::ranges::any_of(layers, [](const auto* layer) { return layer->visible(); });
}

void Map::isolateLayers(const std::vector<LayerNode*>& layers)
{
  const auto allLayers = world()->allLayers();

  auto transaction = Transaction{*this, "Isolate Layers"};
  hideNodes(kdl::vec_static_cast<Node*>(allLayers));
  showNodes(kdl::vec_static_cast<Node*>(layers));
  transaction.commit();
}

bool Map::canIsolateLayers(const std::vector<LayerNode*>& layers) const
{
  const auto allLayers = world()->allLayers();
  return std::ranges::any_of(allLayers, [&](const auto* layer) {
    return kdl::vec_contains(layers, layer) != layer->visible();
  });
}

void Map::setOmitLayerFromExport(LayerNode* layerNode, const bool omitFromExport)
{
  const auto commandName =
    omitFromExport ? "Omit Layer from Export" : "Include Layer in Export";

  auto layer = layerNode->layer();
  layer.setOmitFromExport(omitFromExport);
  updateNodeContents(commandName, {{layerNode, NodeContents(std::move(layer))}}, {});
}

bool Map::moveLayerByOne(LayerNode* layerNode, const MoveDirection direction)
{
  const auto sorted = world()->customLayersUserSorted();

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
    "Swap Layer Positions",
    {{layerNode, NodeContents(std::move(layer))},
     {neighbourNode, NodeContents(std::move(neighbourLayer))}},
    {});

  return true;
}

} // namespace tb::mdl
