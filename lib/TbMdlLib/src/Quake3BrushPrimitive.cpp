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

#include "mdl/Quake3BrushPrimitive.h"

#include "vm/vec.h"

#include <cmath>

namespace tb::mdl
{

std::tuple<vm::vec3d, vm::vec3d> computeAxisBase(const vm::vec3d& normal)
{
  // This is a direct port of GtkRadiant's ComputeAxisBase() so that brush primitive
  // texture matrices are interpreted exactly as the original editor authored them.
  // https://github.com/TTimo/GtkRadiant/blob/270af88f3c2471f6773bded0b5760a3115b52965/radiant/brush_primit.cpp#L70
  auto n = normal;

  // do some cleaning to avoid degenerate cases
  if (std::abs(n.x()) < 1e-6)
  {
    n[0] = 0.0;
  }
  if (std::abs(n.y()) < 1e-6)
  {
    n[1] = 0.0;
  }
  if (std::abs(n.z()) < 1e-6)
  {
    n[2] = 0.0;
  }

  const auto rotY = -std::atan2(n.z(), std::sqrt(n.y() * n.y() + n.x() * n.x()));
  const auto rotZ = std::atan2(n.y(), n.x());

  const auto texX = vm::vec3d{-std::sin(rotZ), std::cos(rotZ), 0.0};
  const auto texY = vm::vec3d{
    -std::sin(rotY) * std::cos(rotZ), -std::sin(rotY) * std::sin(rotZ), -std::cos(rotY)};

  return {texX, texY};
}

Quake3BrushPrimitiveUVAxes brushPrimitiveMatrixToUVAxes(
  const vm::vec3d& normal,
  const Quake3BrushPrimitiveMatrix& matrix,
  const vm::vec2f& textureSize)
{
  const auto [texX, texY] = computeAxisBase(normal);

  const auto texW = double(textureSize.x());
  const auto texH = double(textureSize.y());

  // The brush primitive matrix yields texture coordinates normalized to texture repeats,
  // whereas TrenchBroom expresses U/V axes in texels (it divides by the texture size when
  // computing UV coordinates). Folding the texture size into the axes and offset lets us
  // store the projection as a parallel UV coordinate system with unit scale.
  return {
    texW * (matrix.row0.x() * texX + matrix.row0.y() * texY),
    texH * (matrix.row1.x() * texX + matrix.row1.y() * texY),
    vm::vec2f{
      float(matrix.row0.z()) * textureSize.x(), float(matrix.row1.z()) * textureSize.y()},
  };
}

Quake3BrushPrimitiveMatrix uvAxesToBrushPrimitiveMatrix(
  const vm::vec3d& normal,
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::vec2f& offset,
  const vm::vec2f& textureSize)
{
  const auto [texX, texY] = computeAxisBase(normal);

  const auto texW = double(textureSize.x());
  const auto texH = double(textureSize.y());

  // Project the world space axes back onto the (orthonormal) axis base and remove the
  // texture size that brushPrimitiveMatrixToUVAxes() folded in.
  return {
    vm::vec3d{
      vm::dot(uAxis, texX) / texW,
      vm::dot(uAxis, texY) / texW,
      double(offset.x()) / texW},
    vm::vec3d{
      vm::dot(vAxis, texX) / texH,
      vm::dot(vAxis, texY) / texH,
      double(offset.y()) / texH},
  };
}

} // namespace tb::mdl
