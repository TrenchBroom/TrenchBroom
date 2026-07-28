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

#include "mdl/UvUtils.h"

#include "kd/contracts.h"

#include "vm/mat_ext.h"

namespace tb::mdl
{

std::tuple<vm::vec3d, vm::vec3d> computeCameraAxesForFaceNormal(const vm::vec3d& normal)
{
  const auto right = vm::abs(vm::dot(vm::vec3d{0, 0, 1}, normal)) < double(1)
                       ? vm::normalize(vm::cross(vm::vec3d{0, 0, 1}, normal))
                       : vm::vec3d{1, 0, 0};
  return {vm::normalize(vm::cross(normal, right)), right};
}

vm::vec2f computeUvCoords(
  const vm::vec3d& point,
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::vec2f& scale)
{
  return vm::vec2f{
    float(vm::dot(point, safeScaleAxis(uAxis, scale.x()))),
    float(vm::dot(point, safeScaleAxis(vAxis, scale.y())))};
}

vm::mat4x4d computeWorldToUvMatrix(
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::vec3d& normal,
  const vm::vec2f& o,
  const vm::vec2f& s)
{
  const vm::vec3d u = safeScaleAxis(uAxis, s.x());
  const vm::vec3d v = safeScaleAxis(vAxis, s.y());
  const vm::vec3d n = normal;

  return vm::mat4x4d{
    u[0],
    u[1],
    u[2],
    o[0],
    v[0],
    v[1],
    v[2],
    o[1],
    n[0],
    n[1],
    n[2],
    0.0,
    0.0,
    0.0,
    0.0,
    1.0};
}

vm::mat4x4d computeUvToWorldMatrix(
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::vec3d& normal,
  const vm::vec2f& offset,
  const vm::vec2f& scale)
{
  const auto result = invert(computeWorldToUvMatrix(uAxis, vAxis, normal, offset, scale));
  contract_assert(result);

  return *result;
}

} // namespace tb::mdl
