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

#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/WorldNode.h"

namespace tb::IO
{

BrushFaceReader::BrushFaceReader(
  const std::string_view str, const Model::MapFormat sourceAndTargetMapFormat)
  : MapReader{str, sourceAndTargetMapFormat, sourceAndTargetMapFormat, {}}
{
}

std::vector<Model::BrushFace> BrushFaceReader::read(
  const vm::bbox3d& worldBounds, ParserStatus& status)
{
  try
  {
    readBrushFaces(worldBounds, status);
    return std::move(m_brushFaces);
  }
  catch (const ParserException&)
  {
    throw;
  }
}

Model::Node* BrushFaceReader::onWorldNode(
  std::unique_ptr<Model::WorldNode>, ParserStatus&)
{
  return nullptr;
}

void BrushFaceReader::onLayerNode(std::unique_ptr<Model::Node>, ParserStatus&) {}

void BrushFaceReader::onNode(Model::Node*, std::unique_ptr<Model::Node>, ParserStatus&) {}

void BrushFaceReader::onBrushFace(Model::BrushFace face, ParserStatus& /* status */)
{
  m_brushFaces.push_back(std::move(face));
}

} // namespace tb::IO
