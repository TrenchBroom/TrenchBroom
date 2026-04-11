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

#include "mdl/BezierPatch.h"

#include <cstddef>
#include <vector>

namespace tb::mdl
{
class BrushFace;

/**
 * Creates one or more Bezier patches from the given brush face, each with the given
 * number of control points per row and column.
 *
 * All control points of every returned patch lie on the boundary plane of the given face.
 * Before clipping, a deterministic first vertex is selected to make patch orientation
 * predictable: first along the camera right axis for the face normal, then (for ties)
 * along the negative camera up axis. The remaining vertices keep their original relative
 * boundary order.
 *
 * - If the face has 3 vertices, one degenerate (triangular) patch is returned.
 * - If the face has 4 vertices, one patch is returned whose boundary exactly matches the
 *   face boundary.
 * - If the face has more than 4 vertices, the polygon is clipped into quads using only
 *   boundary vertices using sliding windows:
 *   (V[0],V[1],V[2],V[3]), (V[2],V[3],V[4],V[5]), ...
 *   If the vertex count is odd, the leftover triangle (V[N-3],V[N-2],V[N-1]) is
 *   represented as a degenerate patch.
 *
 * @param face the brush face to convert
 * @param pointRowCount the number of control point rows (must be odd and > 2)
 * @param pointColumnCount the number of control point columns (must be odd and > 2)
 * @return the created patches
 */
std::vector<BezierPatch> createPatch(
  const BrushFace& face, size_t pointRowCount, size_t pointColumnCount);

} // namespace tb::mdl
