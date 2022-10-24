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

#pragma once

#include "FloatType.h"
#include "View/ToolController.h"

#include <vecmath/vec.h>

#include <memory>

namespace TrenchBroom
{
namespace View
{
class CreateSimpleBrushTool;
class DragTracker;
class MapDocument;

class CreateSimpleBrushToolController3D : public ToolController
{
private:
  CreateSimpleBrushTool& m_tool;
  std::weak_ptr<MapDocument> m_document;

  vm::vec3 m_initialPoint;

public:
  CreateSimpleBrushToolController3D(
    CreateSimpleBrushTool& tool, std::weak_ptr<MapDocument> document);

private:
  Tool& tool() override;
  const Tool& tool() const override;

  std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

  bool cancel() override;
};
} // namespace View
} // namespace TrenchBroom
