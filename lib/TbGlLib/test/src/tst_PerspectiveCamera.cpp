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

#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include <limits>

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

    CHECK(!vm::is_nan(camera.direction()));
    CHECK(!vm::is_nan(camera.right()));
    CHECK(!vm::is_nan(camera.up()));
  }

  SECTION("orbit and look down")
  {
    camera.setDirection(vm::vec3f(1, 0, 0), vm::vec3f(0, 0, 1));
    camera.orbit(vm::vec3f{0, 0, 0}, 0.0f, vm::constants<float>::pi());

    CHECK(!vm::is_nan(camera.direction()));
    CHECK(!vm::is_nan(camera.right()));
    CHECK(!vm::is_nan(camera.up()));
  }

  SECTION("orbit with inverted view direction")
  {
    camera.setDirection(vm::vec3f(1, 0, 0), vm::vec3f(0, 0, -1));
    camera.orbit(vm::vec3f{0, 0, 0}, vm::constants<float>::pi(), 0.0f);

    CHECK(!vm::is_nan(camera.direction()));
    CHECK(!vm::is_nan(camera.right()));
    CHECK(!vm::is_nan(camera.up()));
  }

  SECTION("rotate when looking straight down")
  {
    camera.setDirection(vm::vec3f{0, 0, -1}, vm::vec3f{1, 0, 0});
    camera.rotate(0.1f, 0.0f);

    CHECK(!vm::is_nan(camera.direction()));
    CHECK(!vm::is_nan(camera.right()));
    CHECK(!vm::is_nan(camera.up()));
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

  SECTION("constructor")
  {
    const auto c = PerspectiveCamera{
      60.0f,
      1.0f,
      100.0f,
      Camera::Viewport{0, 0, 100, 100},
      vm::vec3f{1, 2, 3},
      vm::vec3f{1, 0, 0},
      vm::vec3f{0, 0, 1}};

    CHECK(c.fov() == 60.0f);
    CHECK(c.nearPlane() == 1.0f);
    CHECK(c.farPlane() == 100.0f);
    CHECK(c.position() == vm::vec3f{1, 2, 3});
  }

  SECTION("fov and setFov")
  {
    CHECK(camera.fov() == 90.0f);

    camera.setFov(60.0f);
    CHECK(camera.fov() == 60.0f);

    // setting the same value is a no-op
    camera.setFov(60.0f);
    CHECK(camera.fov() == 60.0f);
  }

  SECTION("zoomedFov and computeZoomedFov")
  {
    // at zoom == 1, computeZoomedFov is the identity, regardless of fov
    CHECK(camera.zoom() == 1.0f);
    CHECK(camera.zoomedFov() == Catch::Approx(90.0f));

    // zoom < 0.7 uses the square root branch
    camera.setZoom(0.5f);
    CHECK(camera.zoomedFov() == Catch::Approx(90.0f * std::sqrt(0.5f)));

    // 0.7 <= zoom < 1.2 linearly interpolates between the two branches
    camera.setZoom(0.9f);
    CHECK(camera.zoomedFov() == Catch::Approx(83.2289f).margin(0.01f));

    // zoom >= 1.2 uses the negated-inverse branch
    camera.setZoom(2.0f);
    CHECK(camera.zoomedFov() == Catch::Approx(135.0f));
  }

  SECTION("isValidZoom rejects zoom factors whose zoomed fov leaves [1, 150]")
  {
    const auto initialZoom = camera.zoom();

    // far too small: zoomedFov collapses towards 0
    camera.setZoom(0.0001f);
    CHECK(camera.zoom() == initialZoom);

    // far too large: zoomedFov approaches 2 * fov = 180
    camera.setZoom(10.0f);
    CHECK(camera.zoom() == initialZoom);

    // a moderate zoom factor is accepted
    camera.setZoom(2.0f);
    CHECK(camera.zoom() == 2.0f);
  }

  SECTION("frustumPlanes pass through the camera position")
  {
    auto topPlane = vm::plane3f{};
    auto rightPlane = vm::plane3f{};
    auto bottomPlane = vm::plane3f{};
    auto leftPlane = vm::plane3f{};
    camera.frustumPlanes(topPlane, rightPlane, bottomPlane, leftPlane);

    // the frustum is a pyramid whose apex is the camera position, so every side
    // plane passes through it
    CHECK(topPlane.point_status(camera.position()) == vm::plane_status::inside);
    CHECK(rightPlane.point_status(camera.position()) == vm::plane_status::inside);
    CHECK(bottomPlane.point_status(camera.position()) == vm::plane_status::inside);
    CHECK(leftPlane.point_status(camera.position()) == vm::plane_status::inside);

    CHECK(vm::is_unit(topPlane.normal, vm::Cf::almost_zero()));
    CHECK(vm::is_unit(rightPlane.normal, vm::Cf::almost_zero()));
    CHECK(vm::is_unit(bottomPlane.normal, vm::Cf::almost_zero()));
    CHECK(vm::is_unit(leftPlane.normal, vm::Cf::almost_zero()));

    // a point straight ahead lies inside all four planes (on the far side from
    // their normals, since the normals point outwards)
    const auto ahead = camera.position() + camera.direction() * 10.0f;
    CHECK(topPlane.point_status(ahead) == vm::plane_status::below);
    CHECK(rightPlane.point_status(ahead) == vm::plane_status::below);
    CHECK(bottomPlane.point_status(ahead) == vm::plane_status::below);
    CHECK(leftPlane.point_status(ahead) == vm::plane_status::below);
  }

  SECTION("projectionType, projectionMatrix and viewMatrix")
  {
    CHECK(camera.perspectiveProjection());
    CHECK(!camera.orthographicProjection());

    CHECK(!vm::is_nan(camera.projectionMatrix()[0]));
    CHECK(!vm::is_nan(camera.viewMatrix()[0]));
  }

  SECTION("pickRay(point)")
  {
    const auto ray = camera.pickRay(vm::vec3f{10, 0, 0});
    CHECK(ray.origin == camera.position());
    CHECK(ray.direction == vm::vec3f{1, 0, 0});
  }

  SECTION("perspectiveScalingFactor")
  {
    // viewportFrustumDistance is 384 for this camera (fov 90, height 768, zoom 1),
    // so a point 768 units along the view direction scales by 768 / 384 == 2
    CHECK(camera.perspectiveScalingFactor(vm::vec3f{768, 0, 0}) == Catch::Approx(2.0f));
  }

  SECTION("pickFrustum")
  {
    // the top and bottom walls of the frustum depend only on the field of view, not
    // on the viewport's aspect ratio, so this camera's default 1024x768 viewport
    // does not change the numbers below

    SECTION("a ray through the frustum's top wall hits it")
    {
      // for size 10, the top wall is the triangle spanned by the camera position
      // and the top two frustum vertices at (10, +-7.5, 7.5), which lies in the
      // plane z == 0.75x; a ray fired straight down through (5, 0, 10) crosses
      // that plane at (5, 0, 3.75), inside the triangle, after travelling
      // 10 - 3.75 == 6.25 units
      const auto ray =
        vm::ray3f{vm::vec3f{5.0f, 0.0f, 10.0f}, vm::vec3f{0.0f, 0.0f, -1.0f}};

      CHECK(camera.pickFrustum(10.0f, ray) == Catch::Approx(6.25f));
    }

    SECTION("a ray that misses the frustum returns the maximum distance")
    {
      const auto ray =
        vm::ray3f{vm::vec3f{1000.0f, 0.0f, 0.0f}, vm::vec3f{0.0f, 1.0f, 0.0f}};

      CHECK(camera.pickFrustum(10.0f, ray) == std::numeric_limits<float>::max());
    }
  }
}

} // namespace tb::gl
