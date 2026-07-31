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

#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("OrthographicCamera")
{
  auto camera = OrthographicCamera{
    1.0f,
    100.0f,
    Camera::Viewport{0, 0, 100, 50},
    vm::vec3f{0, 0, 0},
    vm::vec3f{1, 0, 0},
    vm::vec3f{0, 0, 1}};

  SECTION("constructor")
  {
    CHECK(camera.position() == vm::vec3f{0, 0, 0});
    CHECK(camera.zoomedViewport() == camera.viewport());
    CHECK(camera.orthographicProjection());
  }

  SECTION("zoomedViewport tracks the zoom factor")
  {
    CHECK(camera.zoomedViewport() == Camera::Viewport{0, 0, 100, 50});

    camera.setZoom(2.0f);
    CHECK(camera.zoomedViewport() == Camera::Viewport{0, 0, 50, 25});
  }

  SECTION("viewportVertices")
  {
    const auto vertices = camera.viewportVertices();

    REQUIRE(vertices.size() == 4);
    CHECK(vertices[0] == vm::vec3d{0, 50, 25});
    CHECK(vertices[1] == vm::vec3d{0, -50, 25});
    CHECK(vertices[2] == vm::vec3d{0, -50, -25});
    CHECK(vertices[3] == vm::vec3d{0, 50, -25});
  }

  SECTION("frustumPlanes")
  {
    auto topPlane = vm::plane3f{};
    auto rightPlane = vm::plane3f{};
    auto bottomPlane = vm::plane3f{};
    auto leftPlane = vm::plane3f{};
    camera.frustumPlanes(topPlane, rightPlane, bottomPlane, leftPlane);

    CHECK(topPlane.normal == vm::vec3f{0, 0, 1});
    CHECK(topPlane.point_distance(camera.position()) == Catch::Approx(-25.0f));

    CHECK(rightPlane.normal == vm::vec3f{0, -1, 0});
    CHECK(rightPlane.point_distance(camera.position()) == Catch::Approx(-50.0f));

    CHECK(bottomPlane.normal == vm::vec3f{0, 0, -1});
    CHECK(bottomPlane.point_distance(camera.position()) == Catch::Approx(-25.0f));

    CHECK(leftPlane.normal == vm::vec3f{0, 1, 0});
    CHECK(leftPlane.point_distance(camera.position()) == Catch::Approx(-50.0f));
  }

  SECTION("pickRay")
  {
    // the ray's origin is the given point projected onto the plane through the
    // camera position that is perpendicular to the view direction
    const auto ray = camera.pickRay(vm::vec3f{10, 5, 3});

    CHECK(ray.origin == vm::vec3f{0, 5, 3});
    CHECK(ray.direction == camera.direction());
  }

  SECTION("perspectiveScalingFactor is the inverse of the zoom factor")
  {
    CHECK(camera.perspectiveScalingFactor(vm::vec3f{0, 0, 0}) == Catch::Approx(1.0f));

    camera.setZoom(2.0f);
    CHECK(camera.perspectiveScalingFactor(vm::vec3f{0, 0, 0}) == Catch::Approx(0.5f));
  }

  SECTION("pickFrustum is not supported for an orthographic camera")
  {
    const auto ray = vm::ray3f{vm::vec3f{0, 0, 0}, vm::vec3f{1, 0, 0}};
    CHECK(vm::is_nan(camera.pickFrustum(10.0f, ray)));
  }
}

} // namespace tb::gl
