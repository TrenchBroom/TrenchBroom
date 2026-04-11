/*
 Copyright (C) 2026 Kristian Duske

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

#include "vm/mat.h"
#include "vm/vec.h"

#include <cstddef>
#include <vector>

namespace tb::mdl
{
class Map;

void convertSelectionToPatches(Map& map, size_t pointRowCount, size_t pointColumnCount);

bool resamplePatches(Map& map, size_t pointRowCount, size_t pointColumnCount);

bool transformControlPoints(
  Map& map,
  const std::vector<vm::vec3d>& controlPointPositions,
  const vm::mat4x4d& transform);

} // namespace tb::mdl
