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

#include "mdl/CatchConfig.h"
#include "mdl/ParallelUVCoordSystem.h"
#include "mdl/Quake3BrushPrimitive.h"

#include "vm/approx.h"
#include "vm/quat.h"
#include "vm/scalar.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("Quake3BrushPrimitive")
{
  SECTION("computeAxisBase produces an orthonormal in-plane basis")
  {
    const auto normals = std::vector<vm::vec3d>{
      {0, 0, 1},
      {0, 0, -1},
      {1, 0, 0},
      {-1, 0, 0},
      {0, 1, 0},
      {0, -1, 0},
      vm::normalize(vm::vec3d{1, 2, 3}),
      vm::normalize(vm::vec3d{-3, 1, -2}),
    };

    for (const auto& normal : normals)
    {
      const auto [texX, texY] = computeAxisBase(normal);

      CHECK(vm::length(texX) == vm::approx{1.0, 0.0001});
      CHECK(vm::length(texY) == vm::approx{1.0, 0.0001});
      CHECK(vm::dot(texX, texY) == vm::approx{0.0, 0.0001});
      CHECK(vm::dot(texX, normal) == vm::approx{0.0, 0.0001});
      CHECK(vm::dot(texY, normal) == vm::approx{0.0, 0.0001});
    }
  }

  SECTION("brush primitive matrix round trips through UV axes")
  {
    struct TestCase
    {
      vm::vec3d normal;
      Quake3BrushPrimitiveMatrix matrix;
      vm::vec2f textureSize;
    };

    // clang-format off
    const auto testCases = std::vector<TestCase>{
      {{0, 0, 1},  {{0.03125, 0.0, 14.0}, {0.0, 0.03125, 4.5}},     {64, 64}},
      {{0, 1, 0},  {{0.01, -0.02, 3.0},   {0.005, 0.015, -7.0}},    {128, 256}},
      {{1, 0, 0},  {{0.0, 0.015625, 0.0}, {0.015625, 0.0, 0.0}},    {64, 64}},
      {vm::normalize(vm::vec3d{1, 2, 3}),
                   {{0.02, 0.01, 1.25},   {-0.01, 0.03, -2.5}},     {64, 128}},
    };
    // clang-format on

    for (const auto& testCase : testCases)
    {
      const auto uvAxes = brushPrimitiveMatrixToUVAxes(
        testCase.normal, testCase.matrix, testCase.textureSize);

      const auto roundTripped = uvAxesToBrushPrimitiveMatrix(
        testCase.normal, uvAxes.uAxis, uvAxes.vAxis, uvAxes.offset, testCase.textureSize);

      CHECK(roundTripped.row0 == vm::approx{testCase.matrix.row0, 0.0001});
      CHECK(roundTripped.row1 == vm::approx{testCase.matrix.row1, 0.0001});
    }
  }

  SECTION("brush primitive matrix maps to the expected UV axes")
  {
    // A floor facing up, textured at one repeat per 64 units with no offset. With a 64x64
    // texture this collapses to the bare axis base, which is the simplest case to verify.
    const auto normal = vm::vec3d{0, 0, 1};
    const auto matrix =
      Quake3BrushPrimitiveMatrix{{1.0 / 64.0, 0.0, 0.0}, {0.0, 1.0 / 64.0, 0.0}};
    const auto textureSize = vm::vec2f{64, 64};

    const auto uvAxes = brushPrimitiveMatrixToUVAxes(normal, matrix, textureSize);
    const auto [texX, texY] = computeAxisBase(normal);

    CHECK(uvAxes.uAxis == vm::approx{texX, 0.0001});
    CHECK(uvAxes.vAxis == vm::approx{texY, 0.0001});
    CHECK(uvAxes.offset == vm::vec2f{0, 0});
  }

  SECTION("parallel UV decomposition round trips modified attributes")
  {
    // Build the projection that a face with the given offset/scale/rotation would have,
    // serialize it to a brush primitive matrix (the way the map writer does), then
    // decompose it back and check the attributes survive. This mirrors a save/load round
    // trip and covers non-64 texture sizes, so it fails for the old code that always
    // assumed 64x64 and discarded scale/rotation.
    struct TestCase
    {
      vm::vec3d normal;
      vm::vec2f textureSize;
      vm::vec2f offset;
      vm::vec2f scale;
      float rotation;
    };

    // clang-format off
    const auto testCases = std::vector<TestCase>{
      {{0, 0, 1},  {64, 64},   {0, 0},   {0.5f, 0.5f},   0.0f},
      {{0, 0, 1},  {128, 128}, {16, -8}, {0.5f, 0.5f},   0.0f},
      {{0, 0, 1},  {128, 256}, {10, 20}, {0.25f, 2.0f},  30.0f},
      {{0, 1, 0},  {64, 128},  {-12, 7}, {1.0f, 0.5f},   210.0f},
      {{1, 0, 0},  {256, 256}, {5, 5},   {0.75f, 0.75f}, 90.0f},
      {vm::normalize(vm::vec3d{1, 2, 3}),
                   {64, 64},   {3, -4},  {0.5f, 1.5f},   45.0f},
      // a mirror is folded onto the V axis, so a mirrored face has a negative Y scale
      {{0, 0, 1},  {128, 128}, {-10, -20}, {0.5f, -0.5f}, 45.0f},
      // rotations at the 180 and 270 degree boundaries with all attributes modified
      {{0, 1, 0},  {64, 128},  {12, 7},    {0.75f, 1.5f}, 180.0f},
      {{1, 0, 0},  {256, 256}, {5, 5},     {2.0f, 0.5f},  270.0f},
      // a second oblique normal
      {vm::normalize(vm::vec3d{-3, 1, -2}),
                   {128, 256}, {6, -9},    {0.5f, 1.5f},  90.0f},
    };
    // clang-format on

    for (const auto& testCase : testCases)
    {
      const auto [baseUAxis, baseVAxis] = computeInitialAxes(testCase.normal);
      const auto textureNormal = vm::cross(baseUAxis, baseVAxis);
      const auto rotation =
        vm::quatd{textureNormal, vm::to_radians(double(testCase.rotation))};
      const auto uAxis = rotation * baseUAxis;
      const auto vAxis = rotation * baseVAxis;

      // the effective axes as stored by the serializer (axes divided by their scale)
      const auto effectiveUAxis = uAxis / double(testCase.scale.x());
      const auto effectiveVAxis = vAxis / double(testCase.scale.y());

      const auto matrix = uvAxesToBrushPrimitiveMatrix(
        testCase.normal,
        effectiveUAxis,
        effectiveVAxis,
        testCase.offset,
        testCase.textureSize);

      const auto uv =
        brushPrimitiveMatrixToParallelUV(testCase.normal, matrix, testCase.textureSize);

      CHECK(uv.offset == vm::approx{testCase.offset, 0.0001f});
      CHECK(uv.scale == vm::approx{testCase.scale, 0.0001f});
      CHECK(uv.rotation == vm::approx{testCase.rotation, 0.01f});
      CHECK(uv.uAxis == vm::approx{uAxis, 0.0001});
      CHECK(uv.vAxis == vm::approx{vAxis, 0.0001});
    }
  }
}

} // namespace tb::mdl
