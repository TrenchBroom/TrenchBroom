/*
 Copyright (C) 2026 Jackson Palmer

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

#include "vm/bbox.h"

#include <cstdint>
#include <vector>

namespace tb::mdl
{

enum class RockType
{
  Boulder,
  Shelf,
  Strata,
  Crag,
  Crystal,
  Columns,
  Basalt,
  Cluster,
};

using RockFormationPoints = std::vector<std::vector<vm::vec3d>>;

/**
 * Generate and fit point clouds for convex rock brushes. The points lie on the integer
 * grid, except for the top points of Columns and Basalt: their tops are slanted, which
 * moves these points off the grid.
 *
 * `detail` is clamped to [0, 1] and makes the formation simpler or more complex. Each
 * rock type interprets it in its own way, for example as the number of faces or pieces.
 *
 * Larger bounds make the formation more complex at the same detail. Which dimensions of
 * the bounds matter also depends on the rock type: Boulder considers its shortest and
 * longest side, and Crag and Crystal their shortest side. Shelf and Strata gain sides
 * with their width and depth, and Strata gains layers with its height. Columns gains
 * columns along its width and along its depth independently. Basalt and Cluster derive
 * the size of their pieces from the detail instead, so that larger bounds are filled
 * with more pieces.
 */
RockFormationPoints makeRockFormation(
  const vm::bbox3d& bounds, RockType type, double detail, uint32_t seed);

} // namespace tb::mdl
