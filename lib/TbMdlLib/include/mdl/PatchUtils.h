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
 * The boundary vertices are reordered before clipping so that the first patch is as
 * visually balanced (symmetric) as possible. The remaining vertices keep their relative
 * boundary order.
 *
 * - If the face has 3 vertices, one degenerate (triangular) patch is returned.
 * - If the face has 4 vertices, one patch is returned whose boundary exactly matches the
 *   face boundary.
 * - If the face has more than 4 vertices, the polygon is clipped into quads using only
 *   boundary vertices, starting with (V[0],V[1],V[2],V[3]) and then repeatedly extending
 *   outward by one vertex on each side of the previous quad's diagonal (the one side of
 *   the quad that is not a boundary edge of the face):
 *   (V[0],V[1],V[2],V[3]), (V[N-1],V[0],V[3],V[4]), (V[N-2],V[N-1],V[4],V[5]), ...
 *   Each patch shares a diagonal (not a boundary edge) with its neighbor, so the patches
 *   tile the polygon exactly, without overlap or gaps.
 *   If the vertex count is odd, the leftover vertex becomes the apex of a degenerate
 *   (triangular) patch closing the same diagonal as the last quad.
 *
 * @param face the brush face to convert
 * @param pointRowCount the number of control point rows (must be odd and > 2)
 * @param pointColumnCount the number of control point columns (must be odd and > 2)
 * @return the created patches
 */
std::vector<BezierPatch> createPatch(
  const BrushFace& face, size_t pointRowCount, size_t pointColumnCount);

/**
 * Creates a new Bezier patch with the given number of control point rows and columns,
 * approximating the given patch's surface as closely as possible.
 *
 * The result reproduces the original surface exactly (up to floating point precision)
 * when the new control point counts equal the original counts, or refine them by an
 * exact integer multiple in a given direction. Any other combination of counts - fewer
 * points in a direction, or more points by a non-integer multiple - cannot reproduce the
 * original surface exactly, since the result then has fewer (or differently positioned)
 * degrees of freedom than the original; in that case, the result is the closest possible
 * approximation for the requested control point counts.
 *
 * @param patch the patch to resample
 * @param pointRowCount the result's control point row count (must be odd and > 2)
 * @param pointColumnCount the result's control point column count (must be odd and > 2)
 * @return the resampled patch
 */
BezierPatch resamplePatch(
  const BezierPatch& patch, size_t pointRowCount, size_t pointColumnCount);

} // namespace tb::mdl
