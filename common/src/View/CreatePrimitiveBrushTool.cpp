/*
 Copyright (C) 2010-2023 Kristian Duske, Nathan "jitspoe" Wulf

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

#include "CreatePrimitiveBrushTool.h"

#include "CreatePrimitiveBrushToolPage.h"
#include "Error.h"
#include "Exceptions.h"
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Game.h"
#include "Model/WorldNode.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/result.h>

namespace TrenchBroom
{
namespace View
{
CreatePrimitiveBrushTool::CreatePrimitiveBrushTool(std::weak_ptr<MapDocument> document)
  : CreateBrushToolBase(false, document)
{
}

void CreatePrimitiveBrushTool::update(const vm::bbox3& bounds)
{
  auto document = kdl::mem_lock(m_document);
  const auto game = document->game();
  const Model::BrushBuilder builder(
    document->world()->mapFormat(), document->worldBounds(), game->defaultFaceAttribs());

  m_previousBounds = bounds;
  std::vector<vm::vec3> positions;

  vm::vec3 size;
  vm::vec3 position = vm::vec3(0.0, 0.0, 0.0);

  position = bounds.center();
  size = bounds.max - bounds.min;
  position[2] = bounds.min[2];

  int numSides = m_primitiveBrushData.numSides;
  int snapType = m_primitiveBrushData.snapType;
  FloatType snap = 0.0;
  if (snapType == PrimitiveBrushData::SNAP_TYPE_INTEGER)
  {
    snap = 1.0f;
  }
  else if (snapType == PrimitiveBrushData::SNAP_TYPE_GRID)
  {
    snap = document->grid().actualSize();
  }

  positions.reserve(((unsigned int)numSides) * 2);

  // Create cylinder or cone verts
  for (int i = 0; i < 2; ++i)
  {
    for (int j = 0; j < numSides; ++j)
    {
      vm::vec3 v;

      // If it's a cone, we only need a single point, so create that and bail.
      if (m_primitiveBrushData.shapeType == PrimitiveBrushData::SHAPE_TYPE_CONE && i == 1)
      {
        v = position;
        v[2] += size[2];
        positions.push_back(v);
        break;
      }

      if (m_primitiveBrushData.radiusMode == PrimitiveBrushData::RADIUS_MODE_EDGE)
      {
        FloatType angle =
          FloatType(j + 0.5) * vm::Cf::two_pi() / FloatType(numSides) - vm::Cf::half_pi();
        FloatType a = vm::Cf::pi() / FloatType(numSides); // Half angle
        FloatType ca = std::cos(a);
        v[0] = std::cos(angle) * 0.5 * size[0] / ca;
        v[1] = std::sin(angle) * 0.5 * size[1] / ca;
      }
      else
      {
        FloatType angle =
          FloatType(j) * vm::Cf::two_pi() / FloatType(numSides) - vm::Cf::half_pi();
        v[0] = std::cos(angle) * 0.5 * size[0];
        v[1] = std::sin(angle) * 0.5 * size[1];
      }

      v[2] = i * size[2];
      v = v + position;

      if (snap > 0.0f)
      {
        v = round(v / snap) * snap;
      }

      positions.push_back(v);
    }
  }

  builder.createBrush(positions, document->currentTextureName())
    .transform([&](auto b) { updateBrush(new Model::BrushNode(std::move(b))); })
    .transform_error([&](auto e) {
      updateBrush(nullptr);
      document->error() << "Could not update brush: " << e.msg;
    });
}

void CreatePrimitiveBrushTool::update()
{
  update(m_previousBounds);
}

QWidget* CreatePrimitiveBrushTool::doCreatePage(QWidget* parent)
{
  return new CreatePrimitiveBrushToolPage(m_document, *this, parent);
}

} // namespace View
} // namespace TrenchBroom
