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

#include "gl/Camera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Grid.h"
#include "mdl/Hit.h"
#include "mdl/NodeHandles.h"
#include "mdl/PatchNode.h"

#include "vm/bbox.h"
#include "vm/ray.h"
#include "vm/segment.h"
#include "vm/vec.h"

#include <limits>
#include <ranges>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{
auto testCamera(const vm::vec3f& position)
{
  return gl::PerspectiveCamera{
    90.0f,
    1.0f,
    8192.0f,
    gl::Camera::Viewport{0, 0, 800, 600},
    position,
    vm::vec3f{1.0f, 0.0f, 0.0f},
    vm::vec3f{0.0f, 0.0f, 1.0f}};
}
} // namespace

TEST_CASE("VertexHandle")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  auto brushBuilder = BrushBuilder{MapFormat::Quake3_Legacy, worldBounds};
  auto brushNode = BrushNode{brushBuilder.createCube(32.0, "material").value()};

  SECTION("getHandles")
  {
    CHECK(VertexHandle::getHandles(EntityNode{Entity{}}).empty());

    CHECK_THAT(
      VertexHandle::getHandles(brushNode),
      UnorderedRangeEquals(
        brushNode.brush().vertices() | std::views::transform([](const auto* vertex) {
          return VertexHandle{vertex->position()};
        })));
  }

  SECTION("distance")
  {
    const auto lhs = VertexHandle{{0.0, 0.0, 0.0}};
    const auto rhs = VertexHandle{{3.0, 4.0, 0.0}};

    CHECK(VertexHandle::distance(lhs, rhs) == 5.0);
  }

  SECTION("pick")
  {
    const auto hitType = HitType::freeType();
    const auto camera = testCamera(vm::vec3f{-200.0f, -16.0f, -16.0f});

    const auto handle = VertexHandle{vm::vec3d{-16.0, -16.0, -16.0}};
    const auto pickRay = vm::ray3d{{-200.0, -16.0, -16.0}, {1.0, 0.0, 0.0}};

    const auto hit = handle.pick(hitType, pickRay, camera, 3.0);
    REQUIRE(hit);

    CHECK(hit->type() == hitType);
    CHECK(hit->target<VertexHandle>() == handle);
  }
}

TEST_CASE("EdgeHandle")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  auto brushBuilder = BrushBuilder{MapFormat::Quake3_Legacy, worldBounds};
  auto brushNode = BrushNode{brushBuilder.createCube(32.0, "material").value()};

  SECTION("getHandles")
  {
    CHECK(EdgeHandle::getHandles(EntityNode{Entity{}}).empty());

    CHECK_THAT(
      EdgeHandle::getHandles(brushNode),
      UnorderedRangeEquals(
        brushNode.brush().edges() | std::views::transform([](const auto* edge) {
          return EdgeHandle{edge->segment()};
        })));
  }

  SECTION("distance")
  {
    const auto lhs = EdgeHandle{{{0.0, 0.0, 0.0}, {10.0, 0.0, 0.0}}};
    const auto rhs = EdgeHandle{{{3.0, 4.0, 0.0}, {10.0, 0.0, 12.0}}};

    CHECK(EdgeHandle::distance(lhs, rhs) == 12.0);
  }

  SECTION("pick")
  {
    const auto hitType = HitType::freeType();
    const auto camera = testCamera(vm::vec3f{-200.0f, -16.0f, -16.0f});

    SECTION("center-point overload")
    {
      const auto handle =
        EdgeHandle{vm::segment3d{{-16.0, -16.0, -16.0}, {16.0, -16.0, -16.0}}};
      const auto pickRay = vm::ray3d{{-200.0, -16.0, -16.0}, {1.0, 0.0, 0.0}};

      const auto hit = handle.pick(hitType, pickRay, camera, 3.0);
      REQUIRE(hit);

      CHECK(hit->type() == hitType);
      CHECK(hit->target<EdgeHandle>() == handle);
    }

    SECTION("grid-snapped overload")
    {
      const auto handle = EdgeHandle{vm::segment3d{{0.0, -1.0, 0.0}, {0.0, 1.0, 0.0}}};
      const auto pickRay = vm::ray3d{{-10.0, 0.2, 0.0}, {1.0, 0.0, 0.0}};
      const auto grid = Grid{0};

      const auto hit = handle.pick(hitType, pickRay, camera, 1.0, grid);
      REQUIRE(hit);

      CHECK(hit->type() == hitType);
      const auto [edgeHandle, pointHandle] = hit->target<EdgeHandle::GridHandleHitData>();
      CHECK(edgeHandle == handle);
      CHECK(pointHandle == vm::vec3d{0.0, 0.0, 0.0});
    }
  }
}

TEST_CASE("FaceHandle")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  auto brushBuilder = BrushBuilder{MapFormat::Quake3_Legacy, worldBounds};
  auto brushNode = BrushNode{brushBuilder.createCube(32.0, "material").value()};

  SECTION("getHandles")
  {
    CHECK(EdgeHandle::getHandles(EntityNode{Entity{}}).empty());

    CHECK_THAT(
      FaceHandle::getHandles(brushNode),
      UnorderedRangeEquals(
        brushNode.brush().faces() | std::views::transform([](const auto& face) {
          return FaceHandle{face.polygon()};
        })));
  }

  SECTION("distance")
  {
    const auto lhs = FaceHandle{vm::polygon3d{{
      {0.0, 0.0, 0.0},
      {2.0, 0.0, 0.0},
      {0.0, 2.0, 0.0},
    }}};
    const auto rhs = FaceHandle{vm::polygon3d{{
      {1.0, 0.0, 0.0},
      {3.0, 0.0, 0.0},
      {1.0, 2.0, 0.0},
    }}};
    const auto differentVertexCount = FaceHandle{vm::polygon3d{{
      {0.0, 0.0, 0.0},
      {2.0, 0.0, 0.0},
      {2.0, 2.0, 0.0},
      {0.0, 2.0, 0.0},
    }}};

    CHECK(FaceHandle::distance(lhs, rhs) == 1.0);
    CHECK(
      FaceHandle::distance(lhs, differentVertexCount)
      == std::numeric_limits<double>::max());
  }

  SECTION("pick")
  {
    const auto hitType = HitType::freeType();
    const auto camera = testCamera(vm::vec3f{-200.0f, -16.0f, 0.0f});

    SECTION("center-point overload")
    {
      const auto handle = FaceHandle{vm::polygon3d{{
        {-16.0, -16.0, -16.0},
        {-16.0, -16.0, 16.0},
        {16.0, -16.0, 16.0},
        {16.0, -16.0, -16.0},
      }}};

      const auto pickRay = vm::ray3d{{-200.0, -16.0, 0.0}, {1.0, 0.0, 0.0}};

      const auto hit = handle.pick(hitType, pickRay, camera, 3.0);
      REQUIRE(hit);

      CHECK(hit->type() == hitType);
      CHECK(hit->target<FaceHandle>() == handle);
    }

    SECTION("grid-snapped overload")
    {
      const auto handle = FaceHandle{vm::polygon3d{{
        {0.0, -2.0, -2.0},
        {0.0, -2.0, 2.0},
        {0.0, 2.0, 2.0},
        {0.0, 2.0, -2.0},
      }}};

      const auto pickRay = vm::ray3d{{-10.0, 0.2, 0.2}, {1.0, 0.0, 0.0}};
      const auto grid = Grid{0};

      const auto hit = handle.pick(hitType, pickRay, camera, 1.0, grid);
      REQUIRE(hit);

      CHECK(hit->type() == hitType);
      const auto [faceHandle, pointHandle] = hit->target<FaceHandle::GridHandleHitData>();
      CHECK(faceHandle == handle);
      CHECK(pointHandle == vm::vec3d{0.0, 0.0, 0.0});
      CHECK(hit->distance() > 0.0);
    }
  }
}

TEST_CASE("ControlPointHandle")
{
  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {-16, -16, -16, 0, 0}, {  0, -16, -16, 0, 0}, { 16, -16, -16, 0, 0},
    {-16,   0, -16, 0, 0}, {  0,   0, -16, 0, 0}, { 16,   0, -16, 0, 0},
    {-16,  16, -16, 0, 0}, {  0,  16, -16, 0, 0}, { 16,  16, -16, 0, 0},
  }, "material"}};
  // clang-format on

  SECTION("getHandles")
  {
    CHECK(ControlPointHandle::getHandles(EntityNode{Entity{}}).empty());

    CHECK_THAT(
      ControlPointHandle::getHandles(patchNode),
      UnorderedRangeEquals(
        patchNode.patch().controlPoints() | std::views::transform([](const auto& point) {
          return ControlPointHandle{point.xyz()};
        })));
  }

  SECTION("distance")
  {
    const auto lhs = ControlPointHandle{{0, 0, 0}};
    const auto rhs = ControlPointHandle{{3, 4, 0}};

    CHECK(ControlPointHandle::distance(lhs, rhs) == 5.0);
  }

  SECTION("pick")
  {
    const auto hitType = HitType::freeType();
    const auto camera = testCamera(vm::vec3f{-200, -16, -16});

    const auto handle = ControlPointHandle{vm::vec3d{-16, -16, -16}};
    const auto pickRay = vm::ray3d{{-200, -16, -16}, {1, 0, 0}};

    const auto hit = handle.pick(hitType, pickRay, camera, 3.0);
    REQUIRE(hit);

    CHECK(hit->type() == hitType);
    CHECK(hit->target<ControlPointHandle>() == handle);
  }
}

} // namespace tb::mdl
