/*
 Copyright (C) 2026

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

#include "mdl/Brush.h"

#include "vm/bbox.h"

#include <vector>

namespace tb::mdl
{
class Brush;

struct BrushOptimizationCandidate
{
  std::vector<vm::bbox3d> bounds;
  /** Area of the coincident faces between the candidate's brushes. */
  double internalFaceArea = 0.0;
  /**
   * Prebuilt non-cuboid brushes for generalized optimization candidates. Cuboid
   * candidates continue to use `bounds` so that the rectangular decomposition
   * algorithm remains independent of map format and face attributes.
   */
  std::vector<Brush> brushes;

  size_t brushCount() const { return brushes.empty() ? bounds.size() : brushes.size(); }
};

/**
 * Returns whether the brush is an axis-aligned cuboid.
 */
bool isAxisAlignedCuboid(const Brush& brush);

/**
 * Decomposes the exact union of the given cuboids into alternative sets of cuboids.
 * Candidates never contain more cuboids than the input, and the original decomposition
 * is not returned. Candidates are ordered by brush count and then total surface area.
 */
std::vector<BrushOptimizationCandidate> createBrushOptimizationCandidates(
  const std::vector<vm::bbox3d>& inputBounds);

} // namespace tb::mdl
