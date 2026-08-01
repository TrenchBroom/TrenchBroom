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
#include "render/SelectionBoundsRenderer.h"

#include "vm/bbox.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::render
{

namespace
{
using range = vm::bbox3d::range;

gl::OrthographicCamera makeCamera(
  const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up)
{
  return gl::OrthographicCamera{
    1.0f, 100.0f, gl::Camera::Viewport{0, 0, 100, 100}, position, direction, up};
}

// places a camera so that bounds.relative_position(camera.position()) reports the
// given range on each axis: bounds spans [-10, 10] on every axis, so -20 is "less", 0
// is "within" and 20 is "greater"
vm::vec3f camPos(const range x, const range y, const range z = range::within)
{
  const auto component = [](const range r) {
    switch (r)
    {
    case range::less:
      return -20.0f;
    case range::within:
      return 0.0f;
    case range::greater:
      return 20.0f;
    }
    return 0.0f;
  };
  return {component(x), component(y), component(z)};
}

// mirrors TextAnchor3D::offset() combined with SizeTextAnchor2D/3D::extraOffsets(), so
// tests don't have to hand-compute the rounded pixel shift for a given alignment
vm::vec3f expectedOffset(
  const vm::vec3f& base, const TextAlignment::Type alignment, const vm::vec2f& size)
{
  auto factors = vm::vec2f{};
  if (alignment & TextAlignment::Left)
  {
    factors[0] = 0.5f;
  }
  else if (alignment & TextAlignment::Right)
  {
    factors[0] = -0.5f;
  }
  if (alignment & TextAlignment::Top)
  {
    factors[1] = -0.5f;
  }
  else if (alignment & TextAlignment::Bottom)
  {
    factors[1] = 0.5f;
  }

  auto extra = vm::vec2f{};
  if (alignment & TextAlignment::Top)
  {
    extra[1] -= 8.0f;
  }
  if (alignment & TextAlignment::Bottom)
  {
    extra[1] += 8.0f;
  }
  if (alignment & TextAlignment::Left)
  {
    extra[0] += 8.0f;
  }
  if (alignment & TextAlignment::Right)
  {
    extra[0] -= 8.0f;
  }

  const auto halfSize = size / 2.0f;
  return base + vm::vec3f{vm::round(factors * size - halfSize + extra), 0.0f};
}
} // namespace

TEST_CASE("SizeTextAnchor2D")
{
  const auto bounds = vm::bbox3d{{-10, -10, -10}, {10, 10, 10}};
  const auto camera =
    makeCamera(camPos(range::within, range::within), {0, 0, -1}, {0, 1, 0});

  SECTION("position is the midpoint of the given axis, min on the other two")
  {
    CHECK(
      SizeTextAnchor2D{bounds, vm::axis::x, camera}.position(camera)
      == vm::vec3f{0, -10, -10});
    CHECK(
      SizeTextAnchor2D{bounds, vm::axis::y, camera}.position(camera)
      == vm::vec3f{-10, 0, -10});
    CHECK(
      SizeTextAnchor2D{bounds, vm::axis::z, camera}.position(camera)
      == vm::vec3f{-10, -10, 0});
  }

  SECTION("offset")
  {
    const auto size = vm::vec2f{40, 20};

    SECTION("axis x is always aligned Top")
    {
      const auto anchor = SizeTextAnchor2D{bounds, vm::axis::x, camera};
      const auto base = camera.project(anchor.position(camera));
      CHECK(
        anchor.offset(camera, size) == expectedOffset(base, TextAlignment::Top, size));
    }

    SECTION(
      "axis y with a camera direction that has a nonzero x component is aligned Top")
    {
      const auto tiltedCamera = makeCamera(
        camPos(range::within, range::within),
        vm::normalize(vm::vec3f{1, 0, -1}),
        {0, 1, 0});
      const auto anchor = SizeTextAnchor2D{bounds, vm::axis::y, tiltedCamera};
      const auto base = tiltedCamera.project(anchor.position(tiltedCamera));
      CHECK(
        anchor.offset(tiltedCamera, size)
        == expectedOffset(base, TextAlignment::Top, size));
    }

    SECTION("axis y with a camera direction with no x component is aligned Right")
    {
      const auto straightCamera =
        makeCamera(camPos(range::within, range::within), {0, 0, -1}, {0, 1, 0});
      const auto anchor = SizeTextAnchor2D{bounds, vm::axis::y, straightCamera};
      const auto base = straightCamera.project(anchor.position(straightCamera));
      CHECK(
        anchor.offset(straightCamera, size)
        == expectedOffset(base, TextAlignment::Right, size));
    }

    SECTION("axis z is always aligned Right")
    {
      const auto anchor = SizeTextAnchor2D{bounds, vm::axis::z, camera};
      const auto base = camera.project(anchor.position(camera));
      CHECK(
        anchor.offset(camera, size) == expectedOffset(base, TextAlignment::Right, size));
    }
  }
}

TEST_CASE("SizeTextAnchor3D")
{
  const auto bounds = vm::bbox3d{{-10, -10, -10}, {10, 10, 10}};
  static constexpr auto minX = -10.0f, maxX = 10.0f;
  static constexpr auto minY = -10.0f, maxY = 10.0f;
  static constexpr auto minZ = -10.0f, maxZ = 10.0f;

  SECTION("position")
  {
    SECTION("axis z: pos.z is always the midpoint, pos.xy follows the camera quadrant")
    {
      // clang-format off
      const auto
      [cameraPos,                              cameraDir,                           expected] = GENERATE(table<vm::vec3f, vm::vec3f, vm::vec3f>({
      {camPos(range::less, range::less),       {0, 0, -1},                          {minX, maxY, 0}},
      {camPos(range::less, range::greater),    {0, 0, -1},                          {maxX, maxY, 0}},
      {camPos(range::greater, range::greater), {0, 0, -1},                          {maxX, minY, 0}},
      {camPos(range::within, range::less),     {0, 0, -1},                          {minX, minY, 0}},
      {camPos(range::within, range::within),   vm::normalize(vm::vec3f{1, 1, 0}),   {maxX, maxY, 0}},
      {camPos(range::within, range::within),   vm::normalize(vm::vec3f{-1, -1, 0}), {minX, minY, 0}},
      }));
      // clang-format on

      CAPTURE(cameraPos, cameraDir);

      const auto camera = makeCamera(cameraPos, cameraDir, {0, 0, 1});
      const auto anchor = SizeTextAnchor3D{bounds, vm::axis::z, camera};
      CHECK(anchor.position(camera) == expected);
    }

    SECTION(
      "axis x: pos.x is always the midpoint, pos.y follows the camera quadrant, "
      "pos.z follows whether the camera is below the bounds")
    {
      static constexpr auto within = range::within;

      // clang-format off
      const auto
      [cameraPos,                                      cameraDir,  expected] = GENERATE(table<vm::vec3f, vm::vec3f, vm::vec3f>({
      {camPos(range::less, range::less, within),       {0, -1, 0}, {0, minY, maxZ}},
      {camPos(range::less, within, within),            {0, -1, 0}, {0, maxY, maxZ}},
      {camPos(range::less, range::greater, within),    {0, -1, 0}, {0, maxY, maxZ}},
      {camPos(within, range::less, within),            {0, -1, 0}, {0, minY, maxZ}},
      {camPos(within, within, within),                 {0, 1, 0},  {0, maxY, maxZ}},
      {camPos(within, within, within),                 {0, -1, 0}, {0, minY, maxZ}},
      {camPos(within, range::greater, within),         {0, -1, 0}, {0, maxY, maxZ}},
      {camPos(range::greater, range::less, within),    {0, -1, 0}, {0, minY, maxZ}},
      {camPos(range::greater, within, within),         {0, -1, 0}, {0, minY, maxZ}},
      {camPos(range::greater, range::greater, within), {0, -1, 0}, {0, maxY, maxZ}},
      // camPos[2] == less flips both the inner ternary and the pos.z outcome
      {camPos(range::less, range::less, range::less),  {0, -1, 0}, {0, maxY, minZ}},
      }));
      // clang-format on

      CAPTURE(cameraPos, cameraDir);

      const auto camera = makeCamera(cameraPos, cameraDir, {0, 0, 1});
      const auto anchor = SizeTextAnchor3D{bounds, vm::axis::x, camera};
      CHECK(anchor.position(camera) == expected);
    }

    SECTION(
      "axis y: pos.y is always the midpoint, pos.x follows the camera quadrant, "
      "pos.z follows whether the camera is below the bounds")
    {
      static constexpr auto within = range::within;

      // clang-format off
      const auto
      [cameraPos,                                      cameraDir,  expected] = GENERATE(table<vm::vec3f, vm::vec3f, vm::vec3f>({
      {camPos(range::less, range::less, within),       {-1, 0, 0}, {minX, 0, maxZ}},
      {camPos(range::less, within, within),            {-1, 0, 0}, {minX, 0, maxZ}},
      {camPos(range::less, range::greater, within),    {-1, 0, 0}, {minX, 0, maxZ}},
      {camPos(within, range::less, within),            {-1, 0, 0}, {minX, 0, maxZ}},
      {camPos(within, within, within),                 {1, 0, 0},  {maxX, 0, maxZ}},
      {camPos(within, within, within),                 {-1, 0, 0}, {minX, 0, maxZ}},
      {camPos(within, range::greater, within),         {-1, 0, 0}, {maxX, 0, maxZ}},
      {camPos(range::greater, range::less, within),    {-1, 0, 0}, {maxX, 0, maxZ}},
      {camPos(range::greater, within, within),         {-1, 0, 0}, {maxX, 0, maxZ}},
      {camPos(range::greater, range::greater, within), {-1, 0, 0}, {maxX, 0, maxZ}},
      // camPos[2] == less flips both the inner ternary and the pos.z outcome
      {camPos(range::less, range::less, range::less),  {-1, 0, 0}, {maxX, 0, minZ}},
      }));
      // clang-format on

      CAPTURE(cameraPos, cameraDir);

      const auto camera = makeCamera(cameraPos, cameraDir, {0, 0, 1});
      const auto anchor = SizeTextAnchor3D{bounds, vm::axis::y, camera};
      CHECK(anchor.position(camera) == expected);
    }
  }

  SECTION("offset")
  {
    const auto size = vm::vec2f{40, 20};

    SECTION("axis z is always aligned Right")
    {
      const auto camera =
        makeCamera(camPos(range::within, range::within), {0, 0, -1}, {0, 1, 0});
      const auto anchor = SizeTextAnchor3D{bounds, vm::axis::z, camera};
      const auto base = camera.project(anchor.position(camera));
      CHECK(
        anchor.offset(camera, size) == expectedOffset(base, TextAlignment::Right, size));
    }

    SECTION("axis x is aligned Top when the camera is below the bounds")
    {
      const auto camera = makeCamera(
        camPos(range::within, range::within, range::less), {0, -1, 0}, {0, 0, 1});
      const auto anchor = SizeTextAnchor3D{bounds, vm::axis::x, camera};
      const auto base = camera.project(anchor.position(camera));
      CHECK(
        anchor.offset(camera, size) == expectedOffset(base, TextAlignment::Top, size));
    }

    SECTION("axis x is aligned Bottom when the camera is not below the bounds")
    {
      const auto camera = makeCamera(
        camPos(range::within, range::within, range::greater), {0, -1, 0}, {0, 0, 1});
      const auto anchor = SizeTextAnchor3D{bounds, vm::axis::x, camera};
      const auto base = camera.project(anchor.position(camera));
      CHECK(
        anchor.offset(camera, size) == expectedOffset(base, TextAlignment::Bottom, size));
    }
  }
}

} // namespace tb::render
