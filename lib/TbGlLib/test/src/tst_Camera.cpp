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

// Camera is abstract, so it is exercised through OrthographicCamera, which does
// not override isValidZoom, keeping the base class's implementation reachable.
#include "gl/OrthographicCamera.h"

#include "vm/ray.h"
#include "vm/segment.h"
#include "vm/vec.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{
namespace
{

bool matrixIsNan(const vm::mat4x4f& m)
{
  return vm::is_nan(m[0]) || vm::is_nan(m[1]) || vm::is_nan(m[2]) || vm::is_nan(m[3]);
}

} // namespace

TEST_CASE("Camera")
{
  auto camera = OrthographicCamera{
    1.0f,
    100.0f,
    Camera::Viewport{0, 0, 100, 50},
    vm::vec3f{0, 0, 0},
    vm::vec3f{1, 0, 0},
    vm::vec3f{0, 0, 1}};

  SECTION("orthographicProjection and perspectiveProjection")
  {
    CHECK(camera.orthographicProjection());
    CHECK(!camera.perspectiveProjection());
  }

  SECTION("nearPlane and setNearPlane")
  {
    CHECK(camera.nearPlane() == 1.0f);
    camera.setNearPlane(2.0f);
    CHECK(camera.nearPlane() == 2.0f);

    // setting the same value is a no-op
    camera.setNearPlane(2.0f);
    CHECK(camera.nearPlane() == 2.0f);
  }

  SECTION("farPlane and setFarPlane")
  {
    CHECK(camera.farPlane() == 100.0f);
    camera.setFarPlane(200.0f);
    CHECK(camera.farPlane() == 200.0f);

    // setting the same value is a no-op
    camera.setFarPlane(200.0f);
    CHECK(camera.farPlane() == 200.0f);
  }

  SECTION("viewport and setViewport")
  {
    CHECK(camera.viewport() == Camera::Viewport{0, 0, 100, 50});
    CHECK(camera.setViewport(Camera::Viewport{0, 0, 200, 100}));
    CHECK(camera.viewport() == Camera::Viewport{0, 0, 200, 100});

    // setting the same viewport again is a no-op
    CHECK(!camera.setViewport(Camera::Viewport{0, 0, 200, 100}));
  }

  SECTION("Viewport::contains and minDimension")
  {
    const auto viewport = Camera::Viewport{0, 0, 100, 50};
    CHECK(viewport.minDimension() == 50);
    CHECK(Camera::Viewport{0, 0, 50, 100}.minDimension() == 50);

    CHECK(viewport.contains(10, 10));
    CHECK(viewport.contains(0, 0));
    CHECK(viewport.contains(100, 50));
    CHECK(!viewport.contains(-10, 10));
    CHECK(!viewport.contains(10, -10));

    CHECK(viewport.contains(-10, -10, 20, 20));
    CHECK(!viewport.contains(-100, -100, 5, 5));
    CHECK(!viewport.contains(200, 200, 5, 5));
  }

  SECTION("zoom, setZoom, zoom(factor) and isValidZoom")
  {
    CHECK(camera.zoom() == 1.0f);

    camera.setZoom(2.0f);
    CHECK(camera.zoom() == 2.0f);

    camera.zoom(2.0f);
    CHECK(camera.zoom() == 4.0f);

    // setting the same value is a no-op
    camera.setZoom(4.0f);
    CHECK(camera.zoom() == 4.0f);

    // values outside [0.02, 100] are rejected
    camera.setZoom(0.01f);
    CHECK(camera.zoom() == 4.0f);
    camera.setZoom(200.0f);
    CHECK(camera.zoom() == 4.0f);

    // boundary values are valid
    camera.setZoom(0.02f);
    CHECK(camera.zoom() == 0.02f);
    camera.setZoom(100.0f);
    CHECK(camera.zoom() == 100.0f);
  }

  SECTION("position, direction, up, right")
  {
    CHECK(camera.position() == vm::vec3f{0, 0, 0});
    CHECK(camera.direction() == vm::vec3f{1, 0, 0});
    CHECK(camera.up() == vm::vec3f{0, 0, 1});
    CHECK(camera.right() == vm::vec3f{0, -1, 0});
  }

  SECTION("moveTo")
  {
    camera.moveTo(vm::vec3f{10, 20, 30});
    CHECK(camera.position() == vm::vec3f{10, 20, 30});
  }

  SECTION("moveBy")
  {
    camera.moveBy(vm::vec3f{1, 2, 3});
    CHECK(camera.position() == vm::vec3f{1, 2, 3});

    // a zero delta is a no-op
    camera.moveBy(vm::vec3f{0, 0, 0});
    CHECK(camera.position() == vm::vec3f{1, 2, 3});
  }

  SECTION("lookAt")
  {
    camera.lookAt(vm::vec3f{10, 0, 0}, vm::vec3f{0, 0, 1});
    CHECK(camera.direction() == vm::vec3f{1, 0, 0});
  }

  SECTION("setDirection with a direction and up vector that are colinear")
  {
    // this takes the fallback branch that derives an arbitrary right vector
    camera.setDirection(vm::vec3f{0, 0, 1}, vm::vec3f{0, 0, 1});

    CHECK(!vm::is_nan(camera.right()));
    CHECK(!vm::is_nan(camera.up()));
    CHECK(vm::is_unit(camera.right(), vm::Cf::almost_zero()));
    CHECK(vm::is_unit(camera.up(), vm::Cf::almost_zero()));
  }

  SECTION("rotate")
  {
    SECTION("no-op and a simple yaw rotation")
    {
      camera.rotate(0.0f, 0.0f);
      CHECK(camera.direction() == vm::vec3f{1, 0, 0});

      camera.rotate(vm::constants<float>::pi() / 2.0f, 0.0f);
      CHECK(camera.direction().x() == Catch::Approx(0.0f).margin(0.0001));
      CHECK(camera.direction().y() == Catch::Approx(1.0f).margin(0.0001));
    }

    SECTION("clamps the up vector so it does not rotate below the horizon")
    {
      camera.rotate(0.0f, -vm::constants<float>::pi() / 2.0f - 0.5f);
      CHECK(camera.up().z() >= -0.0001f);
      CHECK(!vm::is_nan(camera.up()));
    }

    SECTION("avoids producing NaN when the clamped up vector points straight down")
    {
      // rotating 180 degrees around `right` sends `up` to exactly (0, 0, -1), which
      // makes the horizontal projection used for clamping zero
      camera.rotate(0.0f, vm::constants<float>::pi());

      CHECK(!vm::is_nan(camera.up()));
      CHECK(!vm::is_nan(camera.direction()));
      CHECK(camera.up().z() >= -0.0001f);
    }

    SECTION("clamps in the other direction when the new direction points upward")
    {
      // a 135 degree pitch leaves the new direction pointing above the horizon
      // while the new up vector dips below it, exercising the other correction
      // angle sign
      camera.rotate(0.0f, 3.0f * vm::constants<float>::pi() / 4.0f);

      CHECK(camera.direction().z() > 0.0f);
      CHECK(camera.up().z() >= -0.0001f);
      CHECK(!vm::is_nan(camera.up()));
    }
  }

  SECTION("orbit")
  {
    camera.orbit(vm::vec3f{0, 0, 0}, 0.0f, 0.0f);
    CHECK(camera.position() == vm::vec3f{0, 0, 0});

    auto c = OrthographicCamera{
      1.0f,
      100.0f,
      Camera::Viewport{0, 0, 100, 50},
      vm::vec3f{10, 0, 0},
      vm::vec3f{-1, 0, 0},
      vm::vec3f{0, 0, 1}};
    c.orbit(vm::vec3f{0, 0, 0}, vm::constants<float>::pi(), 0.0f);

    CHECK(c.position().x() == Catch::Approx(-10.0f).margin(0.001));
    CHECK(c.position().y() == Catch::Approx(0.0f).margin(0.001));
  }

  SECTION("projectionMatrix and viewMatrix are validated lazily and then cached")
  {
    // each accessor independently validates on first use if the cache is stale, so
    // call them in both orders to cover each one's own validation branch
    const auto projectionMatrix = camera.projectionMatrix();
    const auto viewMatrix = camera.viewMatrix();

    CHECK(!matrixIsNan(projectionMatrix));
    CHECK(!matrixIsNan(viewMatrix));

    // subsequent calls return the same, cached matrices
    CHECK(camera.projectionMatrix() == projectionMatrix);
    CHECK(camera.viewMatrix() == viewMatrix);

    auto c = OrthographicCamera{};
    const auto viewMatrix2 = c.viewMatrix();
    const auto projectionMatrix2 = c.projectionMatrix();

    CHECK(!matrixIsNan(projectionMatrix2));
    CHECK(!matrixIsNan(viewMatrix2));
  }

  SECTION("orthogonalBillboardMatrix")
  {
    CHECK(!matrixIsNan(camera.orthogonalBillboardMatrix()));
  }

  SECTION("verticalBillboardMatrix")
  {
    SECTION("regular case")
    {
      CHECK(!matrixIsNan(camera.verticalBillboardMatrix()));
    }

    SECTION("falls back to the up vector when looking straight down")
    {
      // -direction projected onto the XY plane is zero here, which must fall back
      // to using -up instead
      auto c = OrthographicCamera{
        1.0f,
        100.0f,
        Camera::Viewport{0, 0, 100, 50},
        vm::vec3f{0, 0, 0},
        vm::vec3f{0, 0, -1},
        vm::vec3f{1, 0, 0}};

      CHECK(!matrixIsNan(c.verticalBillboardMatrix()));
    }
  }

  SECTION("viewRay")
  {
    const auto ray = camera.viewRay();
    CHECK(ray.origin == camera.position());
    CHECK(ray.direction == camera.direction());
  }

  SECTION("pickRay(x, y)")
  {
    const auto ray = camera.pickRay(50.0f, 25.0f);
    CHECK(!vm::is_nan(ray.direction));
  }

  SECTION("distanceTo and squaredDistanceTo")
  {
    CHECK(camera.distanceTo(vm::vec3f{5, 0, 0}) == Catch::Approx(5.0f));
    CHECK(camera.squaredDistanceTo(vm::vec3f{5, 0, 0}) == Catch::Approx(25.0f));
  }

  SECTION("perpendicularDistanceTo")
  {
    CHECK(camera.perpendicularDistanceTo(vm::vec3f{5, 3, 3}) == Catch::Approx(5.0f));
  }

  SECTION("defaultPoint(distance)")
  {
    CHECK(camera.defaultPoint(10.0f) == vm::vec3f{10, 0, 0});
    CHECK(camera.defaultPoint() == camera.defaultPoint(Camera::DefaultPointDistance));
  }

  SECTION("defaultPoint(x, y)")
  {
    // defined as the point at the default distance along the pick ray for the given
    // screen coordinates
    const auto ray = camera.pickRay(50.0f, 25.0f);
    const auto expected = ray.origin + Camera::DefaultPointDistance * ray.direction;

    CHECK(camera.defaultPoint(50.0f, 25.0f) == expected);
  }

  SECTION("project")
  {
    // right = (0, -1, 0) and up = (0, 0, 1), so world Y maps to screen X (inverted)
    // and world Z maps to screen Y; depth is remapped from [nearPlane, farPlane] to
    // [0, 1]
    const auto projected = camera.project(vm::vec3f{50.5f, 2.0f, -10.0f});

    CHECK(projected.x() == Catch::Approx(48.0f).margin(0.01f));
    CHECK(projected.y() == Catch::Approx(15.0f).margin(0.01f));
    CHECK(projected.z() == Catch::Approx(0.5f).margin(0.01f));

    // a second call reuses the already-validated matrices
    CHECK(camera.project(vm::vec3f{50.5f, 2.0f, -10.0f}) == projected);
  }

  SECTION("unproject")
  {
    SECTION("unproject(x, y, depth)")
    {
      // the center of the viewport at depth 0.5 (i.e. halfway between the near and
      // far planes) unprojects to the point on the view axis halfway between the
      // planes
      const auto point = camera.unproject(50.0f, 25.0f, 0.5f);

      CHECK(point.x() == Catch::Approx(50.5f).margin(0.01f));
      CHECK(point.y() == Catch::Approx(0.0f).margin(0.01f));
      CHECK(point.z() == Catch::Approx(0.0f).margin(0.01f));
    }

    SECTION("unproject(point) delegates to unproject(x, y, depth)")
    {
      CHECK(
        camera.unproject(vm::vec3f{50.0f, 25.0f, 0.5f})
        == camera.unproject(50.0f, 25.0f, 0.5f));
    }
  }

  SECTION("pickPointHandle")
  {
    const auto ray = vm::ray3d{vm::vec3d{-10, 0, 0}, vm::vec3d{1, 0, 0}};

    const auto hit = camera.pickPointHandle(ray, vm::vec3d{0, 0, 0}, 1.0);
    CHECK(hit.has_value());

    const auto miss = camera.pickPointHandle(ray, vm::vec3d{0, 1000, 0}, 1.0);
    CHECK(!miss.has_value());
  }

  SECTION("pickLineSegmentHandle")
  {
    SECTION("hit")
    {
      const auto ray = vm::ray3d{vm::vec3d{-10, 0, 0}, vm::vec3d{1, 0, 0}};
      const auto segment = vm::segment3d{vm::vec3d{0, -5, 0}, vm::vec3d{0, 5, 0}};

      const auto hit = camera.pickLineSegmentHandle(ray, segment, 1.0);
      CHECK(hit.has_value());
    }

    SECTION("returns nullopt for a segment parallel to the ray")
    {
      const auto ray = vm::ray3d{vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}};
      const auto segment = vm::segment3d{vm::vec3d{0, 5, 0}, vm::vec3d{10, 5, 0}};

      const auto hit = camera.pickLineSegmentHandle(ray, segment, 1.0);
      CHECK(!hit.has_value());
    }
  }
}

} // namespace tb::gl
