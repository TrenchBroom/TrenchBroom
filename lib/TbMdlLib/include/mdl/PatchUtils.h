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

namespace tb::mdl
{

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
