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

#include <vector>

namespace tb::mdl
{
class Map;
class Node;

void isolateSelectedNodes(Map& map);
void hideSelectedNodes(Map& map);
void hideNodes(Map& map, std::vector<Node*> nodes);
void showAllNodes(Map& map);
void showNodes(Map& map, const std::vector<Node*>& nodes);
void ensureNodesVisible(Map& map, const std::vector<Node*>& nodes);
void resetNodeVisibility(Map& map, const std::vector<Node*>& nodes);
void downgradeShownToInherit(Map& map, const std::vector<Node*>& nodes);

} // namespace tb::mdl
