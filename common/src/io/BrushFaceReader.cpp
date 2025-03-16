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

#include "BrushFaceReader.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/WorldNode.h"

namespace tb::io
{

BrushFaceReader::BrushFaceReader(
  const std::string_view str, const mdl::MapFormat sourceAndTargetMapFormat)
  : MapReader{str, sourceAndTargetMapFormat, sourceAndTargetMapFormat, {}}
{
}

std::vector<mdl::BrushFace> BrushFaceReader::read(
  const vm::bbox3d& worldBounds, ParserStatus& status)
{
  readBrushFaces(worldBounds, status);
  return std::move(m_brushFaces);
}

mdl::Node* BrushFaceReader::onWorldNode(std::unique_ptr<mdl::WorldNode>, ParserStatus&)
{
  return nullptr;
}

void BrushFaceReader::onLayerNode(std::unique_ptr<mdl::Node>, ParserStatus&) {}

void BrushFaceReader::onNode(mdl::Node*, std::unique_ptr<mdl::Node>, ParserStatus&) {}

void BrushFaceReader::onBrushFace(mdl::BrushFace face, ParserStatus& /* status */)
{
  m_brushFaces.push_back(std::move(face));
}

} // namespace tb::io
