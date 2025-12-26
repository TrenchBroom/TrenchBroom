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
class LayerNode;
class Map;

void setCurrentLayer(Map& map, LayerNode* currentLayer);
bool canSetCurrentLayer(const Map& map, LayerNode* currentLayer);

void renameLayer(Map& map, LayerNode* layer, const std::string& name);

void moveLayer(Map& map, LayerNode* layer, int offset);
bool canMoveLayer(const Map& map, LayerNode* layer, int offset);

void moveSelectedNodesToLayer(Map& map, LayerNode* layer);
bool canMoveSelectedNodesToLayer(const Map& map, LayerNode* layer);

void hideLayers(Map& map, const std::vector<LayerNode*>& layers);
bool canHideLayers(const std::vector<LayerNode*>& layers);

void isolateLayers(Map& map, const std::vector<LayerNode*>& layers);
bool canIsolateLayers(const Map& map, const std::vector<LayerNode*>& layers);

void setOmitLayerFromExport(Map& map, LayerNode* layerNode, bool omitFromExport);

} // namespace tb::mdl
