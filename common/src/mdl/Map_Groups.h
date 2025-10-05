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

#include <string>
#include <vector>

namespace tb::mdl
{
class GroupNode;
class Map;
class Node;

Node* currentGroupOrWorld(const Map& map);

void openGroup(Map& map, GroupNode& groupNode);
void closeGroup(Map& map);

GroupNode* groupSelectedNodes(Map& map, const std::string& name);
void ungroupSelectedNodes(Map& map);
void mergeSelectedGroupsWithGroup(Map& map, GroupNode* group);

void renameSelectedGroups(Map& map, const std::string& name);

bool canCreateLinkedDuplicate(const Map& map);
GroupNode* createLinkedDuplicate(Map& map);

/**
 * Unlinks the selected linked groups.
 *
 * For every set of selected linked groups that belong to the same link set, the
 * selected groups will be added to a new link set with the effect that these groups
 * will still be linked to each other, but they will no longer be linked to any other
 * member of their original link set that was not selected.
 */
void separateSelectedLinkedGroups(Map& map, bool relinkGroups = true);
bool canSeparateSelectedLinkedGroups(const Map& map);

bool canUpdateLinkedGroups(const std::vector<Node*>& nodes);

void setHasPendingChanges(
  const std::vector<GroupNode*>& groupNodes, bool hasPendingChanges);

} // namespace tb::mdl
