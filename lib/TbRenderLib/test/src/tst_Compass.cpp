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

#include "gl/OrthographicCamera.h"
#include "render/Compass2D.h"

#include "vm/mat.h"
#include "vm/mat_ext.h" // IWYU pragma: keep
#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("Compass")
{
  // Compass is abstract; Compass2D adds no state or behavior relevant here, so it's
  // used just to get a concrete instance to call the (otherwise identical, inherited)
  // cameraRotationMatrix() on
  const auto compass = Compass2D{};

  SECTION("cameraRotationMatrix")
  {
    SECTION("builds a matrix from the camera's basis vectors and inverts it")
    {
      const auto camera = gl::OrthographicCamera{
        1.0f,
        100.0f,
        gl::Camera::Viewport{0, 0, 100, 100},
        vm::vec3f{3, 4, 5},
        vm::vec3f{0, 1, 0},
        vm::vec3f{0, 0, 1}};

      auto expected = vm::mat4x4f{};
      expected[0] = vm::vec4f{camera.right()};
      expected[1] = vm::vec4f{camera.direction()};
      expected[2] = vm::vec4f{camera.up()};
      expected = *vm::invert(expected);

      CHECK(compass.cameraRotationMatrix(camera) == expected);
    }

    SECTION(
      "for an axis-aligned camera, the result is the transpose of the basis "
      "matrix (its inverse, since the basis is orthonormal)")
    {
      const auto camera = gl::OrthographicCamera{
        1.0f,
        100.0f,
        gl::Camera::Viewport{0, 0, 100, 100},
        vm::vec3f{0, 0, 0},
        vm::vec3f{0, 1, 0},
        vm::vec3f{0, 0, 1}};

      auto basis = vm::mat4x4f{};
      basis[0] = vm::vec4f{camera.right()};
      basis[1] = vm::vec4f{camera.direction()};
      basis[2] = vm::vec4f{camera.up()};

      CHECK(compass.cameraRotationMatrix(camera) == vm::transpose(basis));
    }
  }
}

} // namespace tb::render
