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

#include "mdl/UVUtils.h"

namespace tb::mdl
{

std::tuple<vm::vec3d, vm::vec3d> computeCameraAxesForFaceNormal(const vm::vec3d& normal)
{
  const auto right = vm::abs(vm::dot(vm::vec3d{0, 0, 1}, normal)) < double(1)
                       ? vm::normalize(vm::cross(vm::vec3d{0, 0, 1}, normal))
                       : vm::vec3d{1, 0, 0};
  return {vm::normalize(vm::cross(normal, right)), right};
}

} // namespace tb::mdl
