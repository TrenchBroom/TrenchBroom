/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "DrawShapeTool.h"
#include "Error.h"
#include "FloatType.h"
#include "Model/Brush.h" // IWYU pragma: keep
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Game.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"

#include "kdl/memory_utils.h"
#include "kdl/result.h"

namespace TrenchBroom::View
{

DrawShapeTool::DrawShapeTool(std::weak_ptr<MapDocument> document)
  : CreateBrushToolBase{true, std::move(document)}
{
}

void DrawShapeTool::update(const vm::bbox3& bounds)
{
  auto document = kdl::mem_lock(m_document);
  const auto game = document->game();
  const auto builder = Model::BrushBuilder{
    document->world()->mapFormat(), document->worldBounds(), game->defaultFaceAttribs()};

  builder.createCuboid(bounds, document->currentTextureName())
    .transform(
      [&](auto b) { updateBrush(std::make_unique<Model::BrushNode>(std::move(b))); })
    .transform_error([&](auto e) {
      updateBrush(nullptr);
      document->error() << "Could not update brush: " << e;
    });
}

} // namespace TrenchBroom::View
