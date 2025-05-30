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

#include "Ensure.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"

#include <algorithm>

namespace tb::mdl
{

kdl_reflect_impl(Selection);

bool Selection::empty() const
{
  return nodes.empty();
}

bool Selection::hasLayers() const
{
  return !layers.empty();
}

bool Selection::hasOnlyLayers() const
{
  return !empty() && nodes.size() == layers.size();
}

bool Selection::hasGroups() const
{
  return !groups.empty();
}

bool Selection::hasOnlyGroups() const
{
  return !empty() && nodes.size() == groups.size();
}

bool Selection::hasEntities() const
{
  return !entities.empty();
}

bool Selection::hasOnlyEntities() const
{
  return !empty() && nodes.size() == entities.size();
}

bool Selection::hasBrushes() const
{
  return !brushes.empty();
}

bool Selection::hasOnlyBrushes() const
{
  return !empty() && nodes.size() == brushes.size();
}

bool Selection::hasPatches() const
{
  return !patches.empty();
}

bool Selection::hasOnlyPatches() const
{
  return !empty() && nodes.size() == patches.size();
}

void Selection::addNodes(const std::vector<Node*>& nodesToAdd)
{
  for (auto* node : nodesToAdd)
  {
    addNode(node);
  }
}

void Selection::addNode(Node* node)
{
  ensure(node != nullptr, "node is null");
  node->accept(kdl::overload(
    [](WorldNode*) {},
    [&](LayerNode* layer) {
      nodes.push_back(layer);
      layers.push_back(layer);
    },
    [&](GroupNode* group) {
      nodes.push_back(group);
      groups.push_back(group);
    },
    [&](EntityNode* entity) {
      nodes.push_back(entity);
      entities.push_back(entity);
    },
    [&](BrushNode* brush) {
      nodes.push_back(brush);
      brushes.push_back(brush);
    },
    [&](PatchNode* patch) {
      nodes.push_back(patch);
      patches.push_back(patch);
    }));
}

static const auto doRemoveNodes = [](
                                    auto& nodes,
                                    auto& layers,
                                    auto& groups,
                                    auto& entities,
                                    auto& brushes,
                                    auto& patches,
                                    auto cur,
                                    auto end) {
  auto nodeEnd = std::end(nodes);
  auto layerEnd = std::end(layers);
  auto groupEnd = std::end(groups);
  auto entityEnd = std::end(entities);
  auto brushEnd = std::end(brushes);
  auto patchEnd = std::end(patches);

  while (cur != end)
  {
    (*cur)->accept(kdl::overload(
      [](WorldNode*) {},
      [&](LayerNode* layer) {
        nodeEnd = std::remove(std::begin(nodes), nodeEnd, layer);
        layerEnd = std::remove(std::begin(layers), layerEnd, layer);
      },
      [&](GroupNode* group) {
        nodeEnd = std::remove(std::begin(nodes), nodeEnd, group);
        groupEnd = std::remove(std::begin(groups), groupEnd, group);
      },
      [&](EntityNode* entity) {
        nodeEnd = std::remove(std::begin(nodes), nodeEnd, entity);
        entityEnd = std::remove(std::begin(entities), entityEnd, entity);
      },
      [&](BrushNode* brush) {
        nodeEnd = std::remove(std::begin(nodes), nodeEnd, brush);
        brushEnd = std::remove(std::begin(brushes), brushEnd, brush);
      },
      [&](PatchNode* patch) {
        nodeEnd = std::remove(std::begin(nodes), nodeEnd, patch);
        patchEnd = std::remove(std::begin(patches), patchEnd, patch);
      }));
    ++cur;
  }

  nodes.erase(nodeEnd, std::end(nodes));
  layers.erase(layerEnd, std::end(layers));
  groups.erase(groupEnd, std::end(groups));
  entities.erase(entityEnd, std::end(entities));
  brushes.erase(brushEnd, std::end(brushes));
  patches.erase(patchEnd, std::end(patches));
};

void Selection::removeNodes(const std::vector<Node*>& nodesToRemovew)
{
  doRemoveNodes(
    nodes,
    layers,
    groups,
    entities,
    brushes,
    patches,
    std::begin(nodesToRemovew),
    std::end(nodesToRemovew));
}

void Selection::removeNode(Node* node)
{
  ensure(node != nullptr, "node is null");
  doRemoveNodes(
    nodes, layers, groups, entities, brushes, patches, &node, std::next(&node));
}

void Selection::clear()
{
  nodes.clear();
  layers.clear();
  groups.clear();
  entities.clear();
  brushes.clear();
  patches.clear();
}

Selection makeSelection(const std::vector<Node*>& nodes)
{
  auto result = Selection{};
  result.addNodes(nodes);
  return result;
}


} // namespace tb::mdl
