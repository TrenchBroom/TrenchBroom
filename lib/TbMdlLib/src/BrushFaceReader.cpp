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

#include "mdl/BrushFaceReader.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

BrushFaceReader::BrushFaceReader(
  const std::string_view str, const MapFormat sourceAndTargetMapFormat)
  : MapReader{str, sourceAndTargetMapFormat, sourceAndTargetMapFormat, {}}
{
}

Result<std::vector<BrushFace>> BrushFaceReader::read(
  const vm::bbox3d& worldBounds, ParserStatus& status)
{
  return readBrushFaces(worldBounds, status)
         | kdl::transform([&]() { return std::move(m_brushFaces); });
}

Node* BrushFaceReader::onWorldNode(std::unique_ptr<WorldNode>, ParserStatus&)
{
  return nullptr;
}

void BrushFaceReader::onLayerNode(std::unique_ptr<Node>, ParserStatus&) {}

void BrushFaceReader::onNode(Node*, std::unique_ptr<Node>, ParserStatus&) {}

void BrushFaceReader::onBrushFace(BrushFace face, ParserStatus& /* status */)
{
  m_brushFaces.push_back(std::move(face));
}

} // namespace tb::mdl
