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

#include "mdl/BrushFaceHandle.h"

#include "vm/util.h"

#include <string_view>
#include <vector>

namespace tb::mdl
{
class LayerNode;
class Map;
class Node;

void selectAllNodes(Map& map);
void selectNodes(Map& map, const std::vector<Node*>& nodes);

void selectSiblingNodes(Map& map);
void selectTouchingNodes(Map& map, bool del);
void selectTouchingNodes(Map& map, vm::axis::type cameraAxis, bool del);
void selectContainedNodes(Map& map, bool del);
void selectNodesWithFilePosition(Map& map, const std::vector<size_t>& positions);
void selectBrushesWithMaterial(Map& map, std::string_view materialName);
void invertNodeSelection(Map& map);

void selectAllInLayers(Map& map, const std::vector<LayerNode*>& layers);
bool canSelectAllInLayers(const Map& map, const std::vector<LayerNode*>& layers);

void selectLinkedGroups(Map& map);
bool canSelectLinkedGroups(const Map& map);

void selectBrushFaces(Map& map, const std::vector<BrushFaceHandle>& handles);
void selectBrushFacesWithMaterial(Map& map, std::string_view materialName);
void convertToFaceSelection(Map& map);

void deselectAll(Map& map);
void deselectNodes(Map& map, const std::vector<Node*>& nodes);
void deselectBrushFaces(Map& map, const std::vector<BrushFaceHandle>& handles);

} // namespace tb::mdl
