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

#include "TestUtils.h"

#include "gl/TestUtils.h"

#include "vm/abstract_line.h"
#include "vm/vec.h"

namespace tb::render
{

std::vector<SpikeVertex> expectedSpikeVertices(const vm::ray3d& ray, const Color& color)
{
  constexpr auto maxLength = 1024.0;
  constexpr auto mix = 0.5;

  const auto rgba = color.to<RgbaF>();
  const auto fadedRgba = blendColor(rgba, mix);

  return {
    SpikeVertex{vm::vec3f(ray.origin), rgba.toVec()},
    SpikeVertex{vm::vec3f{vm::point_at_distance(ray, maxLength * 0.75)}, rgba.toVec()},
    SpikeVertex{vm::vec3f{vm::point_at_distance(ray, maxLength * 0.75)}, rgba.toVec()},
    SpikeVertex{vm::vec3f{vm::point_at_distance(ray, maxLength)}, fadedRgba.toVec()},
  };
}

} // namespace tb::render
