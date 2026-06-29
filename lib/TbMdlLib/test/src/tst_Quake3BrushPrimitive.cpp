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
#include "mdl/Quake3BrushPrimitive.h"

#include "vm/approx.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

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
}

} // namespace tb::mdl
