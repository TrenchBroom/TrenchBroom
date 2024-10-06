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

#include "io/MapReader.h"

#include <string_view>
#include <vector>

namespace tb::mdl
{
class BrushNode;
class BrushFace;
class EntityProperty;
class LayerNode;
enum class MapFormat;
class Node;
} // namespace tb::mdl

namespace tb::io
{
class ParserStatus;

/**
 * Used for pasting brush faces (i.e. their UVs only)
 */
class BrushFaceReader : public MapReader
{
private:
  std::vector<mdl::BrushFace> m_brushFaces;

public:
  BrushFaceReader(std::string_view str, mdl::MapFormat sourceAndTargetMapFormat);

  std::vector<mdl::BrushFace> read(const vm::bbox3d& worldBounds, ParserStatus& status);

private: // implement MapReader interface
  mdl::Node* onWorldNode(
    std::unique_ptr<mdl::WorldNode> worldNode, ParserStatus& status) override;
  void onLayerNode(std::unique_ptr<mdl::Node> layerNode, ParserStatus& status) override;
  void onNode(
    mdl::Node* parentNode,
    std::unique_ptr<mdl::Node> node,
    ParserStatus& status) override;
  void onBrushFace(mdl::BrushFace face, ParserStatus& status) override;
};

} // namespace tb::io
