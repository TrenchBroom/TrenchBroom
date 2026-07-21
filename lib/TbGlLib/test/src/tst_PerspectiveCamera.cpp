/*
 Copyright (C) 2010 Kristian Duske

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

#include "gl/PerspectiveCamera.h"

#include "vm/scalar.h"
#include "vm/vec.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::gl
{

TEST_CASE("PerspectiveCamera")
{
  auto camera = PerspectiveCamera{};

  SECTION("invalid up axis")
  {
    camera.setDirection(vm::vec3f(0, 0, 1), vm::vec3f(0, 0, 1));

    CHECK_FALSE(vm::is_nan(camera.direction()));
    CHECK_FALSE(vm::is_nan(camera.right()));
    CHECK_FALSE(vm::is_nan(camera.up()));
  }

  SECTION("orbit and look down")
  {
    PerspectiveCamera c;
    c.setDirection(vm::vec3f(1, 0, 0), vm::vec3f(0, 0, 1));

    c.orbit(vm::vec3f{0, 0, 0}, 0.0f, vm::constants<float>::pi());

    CHECK_FALSE(vm::is_nan(c.direction()));
    CHECK_FALSE(vm::is_nan(c.right()));
    CHECK_FALSE(vm::is_nan(c.up()));
  }

  SECTION("orbit with inverted view direction")
  {
    PerspectiveCamera c;
    c.setDirection(vm::vec3f(1, 0, 0), vm::vec3f(0, 0, -1));

    c.orbit(vm::vec3f{0, 0, 0}, vm::constants<float>::pi(), 0.0f);

    CHECK_FALSE(vm::is_nan(c.direction()));
    CHECK_FALSE(vm::is_nan(c.right()));
    CHECK_FALSE(vm::is_nan(c.up()));
  }

  SECTION("rotate when looking straight down")
  {
    PerspectiveCamera c;
    c.setDirection(vm::vec3f{0, 0, -1}, vm::vec3f{1, 0, 0});

    c.rotate(0.1f, 0.0f);

    CHECK_FALSE(vm::is_nan(c.direction()));
    CHECK_FALSE(vm::is_nan(c.right()));
    CHECK_FALSE(vm::is_nan(c.up()));
  }

  SECTION("yaw")
  {
    const auto [direction, up, expectedYaw] =
      GENERATE(table<vm::vec3f, vm::vec3f, float>({
        {vm::vec3f{1, 0, 0}, vm::vec3f{0, 0, 1}, 0.0f},
        {vm::vec3f{0, 1, 0}, vm::vec3f{0, 0, 1}, 90.0f},
        {vm::vec3f{-1, 0, 0}, vm::vec3f{0, 0, 1}, 180.0f},
        {vm::vec3f{0, -1, 0}, vm::vec3f{0, 0, 1}, -90.0f},
        {vm::normalize(vm::vec3f{1, 1, 0}), vm::vec3f{0, 0, 1}, 45.0f},
        // yaw ignores the pitch component of the direction
        {vm::normalize(vm::vec3f{0, 1, 1}), vm::vec3f{0, 0, 1}, 90.0f},
      }));

    CAPTURE(direction, up);

    camera.setDirection(direction, up);
    CHECK(vm::to_degrees(camera.yaw()) == Catch::Approx{expectedYaw});
  }

  SECTION("pitch")
  {
    const auto [direction, up, expectedPitch] =
      GENERATE(table<vm::vec3f, vm::vec3f, float>({
        {vm::vec3f{1, 0, 0}, vm::vec3f{0, 0, 1}, 0.0f},
        {vm::normalize(vm::vec3f{1, 0, 1}), vm::vec3f{0, 0, 1}, 45.0f},
        {vm::normalize(vm::vec3f{1, 0, -1}), vm::vec3f{0, 0, 1}, -45.0f},
        {vm::vec3f{0, 0, 1}, vm::vec3f{1, 0, 0}, 90.0f},
        {vm::vec3f{0, 0, -1}, vm::vec3f{1, 0, 0}, -90.0f},
        // pitch ignores the yaw component of the direction
        {vm::normalize(vm::vec3f{0, 1, 1}), vm::vec3f{0, 0, 1}, 45.0f},
      }));

    CAPTURE(direction, up);

    camera.setDirection(direction, up);
    CHECK(vm::to_degrees(camera.pitch()) == Catch::Approx{expectedPitch});
  }
}

} // namespace tb::gl
