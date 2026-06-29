/*
 Copyright (C) 2026 Thomas Jones

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

#include <tuple>

namespace tb::mdl
{

/**
 * Quake 3 brush primitive ("brushDef") texture matrix as it appears in a .map file:
 *
 *     ( ( row0.x row0.y row0.z ) ( row1.x row1.y row1.z ) )
 *
 * The matrix maps a point on the face plane (expressed in the face's axis base, see
 * computeAxisBase()) to texture coordinates that are normalized so that 1.0 corresponds
 * to one full texture repeat:
 *
 *     s = row0.x * (texX . p) + row0.y * (texY . p) + row0.z
 *     t = row1.x * (texX . p) + row1.y * (texY . p) + row1.z
 */
struct Quake3BrushPrimitiveMatrix
{
  vm::vec3d row0;
  vm::vec3d row1;

  friend bool operator==(
    const Quake3BrushPrimitiveMatrix& lhs,
    const Quake3BrushPrimitiveMatrix& rhs) = default;
};

/**
 * World space U/V axes plus a texture offset (in texels) that reproduce a brush primitive
 * texture matrix in TrenchBroom's parallel UV coordinate system (xScale = yScale = 1,
 * rotation = 0).
 */
struct Quake3BrushPrimitiveUVAxes
{
  vm::vec3d uAxis;
  vm::vec3d vAxis;
  vm::vec2f offset;
};

/**
 * Computes the Quake 3 brush primitive "axis base" for a face with the given normal: two
 * orthonormal vectors `{ texX, texY }` spanning the face plane. This matches GtkRadiant's
 * ComputeAxisBase() so that brush primitive texture matrices are interpreted identically.
 */
std::tuple<vm::vec3d, vm::vec3d> computeAxisBase(const vm::vec3d& normal);

/**
 * Converts a brush primitive texture matrix into world space U/V axes and a texel offset,
 * given the face normal and the texture's dimensions. The result can be passed to
 * BrushFace::createFromValve() to build a parallel UV coordinate system.
 */
Quake3BrushPrimitiveUVAxes brushPrimitiveMatrixToUVAxes(
  const vm::vec3d& normal,
  const Quake3BrushPrimitiveMatrix& matrix,
  const vm::vec2f& textureSize);

/**
 * Inverse of brushPrimitiveMatrixToUVAxes(): converts the effective world space U/V axes
 * (i.e. the stored axes already divided by their scale) and texel offset of a parallel UV
 * coordinate system back into a brush primitive texture matrix.
 */
Quake3BrushPrimitiveMatrix uvAxesToBrushPrimitiveMatrix(
  const vm::vec3d& normal,
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::vec2f& offset,
  const vm::vec2f& textureSize);

} // namespace tb::mdl
