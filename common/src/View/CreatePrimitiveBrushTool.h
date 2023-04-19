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
  int numSides = 8;
  int snapType = 1;
  int radiusMode = 0;
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
