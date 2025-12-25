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

#pragma once

#include "mdl/NodeContents.h"

#include <vector>

namespace tb::mdl
{
class GroupNode;
class Map;
class Node;

/**
 * Suggests a parent to use for new nodes.
 *
 * If reference nodes are given, return the parent (either a group, if there is one,
 * otherwise the layer) of the first node in the given vector.
 *
 * Otherwise, returns the current group if one is open, otherwise the current layer.
 */
Node* parentForNodes(const Map& map, const std::vector<Node*>& nodes = {});

std::vector<Node*> addNodes(Map& map, const std::map<Node*, std::vector<Node*>>& nodes);

void duplicateSelectedNodes(Map& map);

bool reparentNodes(Map& map, const std::map<Node*, std::vector<Node*>>& nodesToAdd);

void removeNodes(Map& map, const std::vector<Node*>& nodes);
void removeSelectedNodes(Map& map);

bool updateNodeContents(
  Map& map,
  const std::string& commandName,
  std::vector<std::pair<Node*, NodeContents>> nodesToSwap,
  std::vector<GroupNode*> changedLinkedGroups);
bool updateNodeContents(
  Map& map,
  const std::string& commandName,
  std::vector<std::pair<Node*, NodeContents>> nodesToSwap);

} // namespace tb::mdl
