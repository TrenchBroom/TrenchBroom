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

#pragma once

#include "FloatType.h"
#include "View/CreateBrushToolBase.h"

#include <vecmath/bbox.h>

#include <memory>


namespace TrenchBroom
{
namespace View
{
class MapDocument;

struct PrimitiveBrushData
{
  enum SnapType
  {
    SNAP_TYPE_DISABLED = 0,
    SNAP_TYPE_INTEGER = 1,
    SNAP_TYPE_GRID = 2
  };

  enum ShapeType
  {
    SHAPE_TYPE_CYLINDER = 0,
    SHAPE_TYPE_CONE = 1
  };

  enum RadiusMode
  {
    RADIUS_MODE_EDGE = 0,
    RADIUS_MODE_VERTEX = 1
  };

  int numSides = 8;
  SnapType snapType = SNAP_TYPE_INTEGER;
  ShapeType shapeType = SHAPE_TYPE_CYLINDER;
  RadiusMode radiusMode = RADIUS_MODE_EDGE;
  bool uniformAspect = true;
};

class CreatePrimitiveBrushTool : public CreateBrushToolBase
{
public:
  explicit CreatePrimitiveBrushTool(std::weak_ptr<MapDocument> document);
  void update(const vm::bbox3& bounds);
  void update();

  PrimitiveBrushData m_primitiveBrushData;

private:
  vm::bbox3 m_previousBounds;
  QWidget* doCreatePage(QWidget* parent) override;
};
} // namespace View
} // namespace TrenchBroom
