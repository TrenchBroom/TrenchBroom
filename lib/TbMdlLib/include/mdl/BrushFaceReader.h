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

#pragma once

#include "Result.h"
#include "mdl/MapReader.h"

#include <string_view>
#include <vector>

namespace tb
{
class ParserStatus;

namespace mdl
{
class BrushNode;
class BrushFace;
class EntityProperty;
class LayerNode;
enum class MapFormat;
class Node;

/**
 * Used for pasting brush faces (i.e. their UVs only)
 */
class BrushFaceReader : public MapReader
{
private:
  std::vector<BrushFace> m_brushFaces;

public:
  BrushFaceReader(
    const GameConfig& config, std::string_view str, MapFormat sourceAndTargetMapFormat);

  Result<std::vector<BrushFace>> read(
    const vm::bbox3d& worldBounds, ParserStatus& status);

private: // implement MapReader interface
  Node* onWorldNode(std::unique_ptr<WorldNode> worldNode, ParserStatus& status) override;
  void onLayerNode(std::unique_ptr<Node> layerNode, ParserStatus& status) override;
  void onNode(
    Node* parentNode, std::unique_ptr<Node> node, ParserStatus& status) override;
  void onBrushFace(BrushFace face, ParserStatus& status) override;
};

} // namespace mdl
} // namespace tb
