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

#include "vm/vec.h"

namespace tb::render
{
class RenderBatch;
class RenderContext;
} // namespace tb::render

namespace tb::ui
{

void renderAngleIndicator(
  render::RenderBatch& renderBatch,
  double handleRadius,
  const vm::vec3d& center,
  const vm::vec3d& axis,
  const vm::vec3d& initialHandlePosition,
  double angle);

void renderAngleText(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::vec3d& position,
  double angleInDegrees);

} // namespace tb::ui
