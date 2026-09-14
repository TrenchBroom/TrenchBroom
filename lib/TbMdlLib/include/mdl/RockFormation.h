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

// Generate and fit integer-grid point clouds for convex rock brushes.
RockFormationPoints makeRockFormation(
  const vm::bbox3d& bounds,
  RockType type,
  size_t resolution,
  double baseFlattening,
  double form,
  uint32_t seed);

} // namespace tb::mdl
