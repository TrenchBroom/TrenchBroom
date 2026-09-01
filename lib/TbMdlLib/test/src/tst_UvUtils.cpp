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

#include "base/Result.h"
#include "mdl/CatchConfig.h"
#include "mdl/UvAttributes.h"
#include "mdl/UvUtils.h"

#include "kd/result.h"

#include "vm/approx.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <limits>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("UvUtils")
{
  SECTION("computeCameraAxesForFaceNormal")
  {
    CHECK(
      computeCameraAxesForFaceNormal(vm::vec3d{0, 0, 1})
      == std::tuple{vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}});

    CHECK(
      computeCameraAxesForFaceNormal(vm::vec3d{0, 0, -1})
      == std::tuple{vm::vec3d{0, -1, 0}, vm::vec3d{1, 0, 0}});

    CHECK(
      computeCameraAxesForFaceNormal(vm::vec3d{1, 0, 0})
      == std::tuple{vm::vec3d{0, 0, 1}, vm::vec3d{0, 1, 0}});

    SECTION("returns normalized up axis orthogonal to normal")
    {
      const auto normal = vm::normalize(vm::vec3d{1, 2, 3});
      const auto [upAxis, rightAxis] = computeCameraAxesForFaceNormal(normal);

      CHECK(vm::length(upAxis) == vm::approx{1.0});
      CHECK(vm::length(rightAxis) == vm::approx{1.0});
      CHECK(vm::dot(normal, upAxis) == vm::approx{0.0});
      CHECK(vm::dot(normal, rightAxis) == vm::approx{0.0});
      CHECK(vm::dot(upAxis, rightAxis) == vm::approx{0.0});
      CHECK(vm::cross(rightAxis, upAxis) == vm::approx(normal));
    }
  }

  SECTION("validateUvCoordSystem")
  {
    static constexpr auto normal = vm::vec3d{0, 0, 1};
    static constexpr auto uAxis = vm::vec3d{1, 0, 0};
    static constexpr auto vAxis = vm::vec3d{0, 1, 0};
    static constexpr auto nan = std::numeric_limits<double>::quiet_NaN();

    // clang-format off
    const auto 
    [testUAxis,            testVAxis, uvAttributes,                   expected] = GENERATE(table<vm::vec3d, vm::vec3d, UvAttributes, Result<void>>({
    // finite, orthonormal axes
    {uAxis,                vAxis,     {},                             kdl::void_success},
    // a scale of 0 doesn't affect invertibility -- safeScaleAxis treats it as 1
    {uAxis,                vAxis,     {{0, 0}, {0, 0}, 0.0f},         kdl::void_success},
    // a non-finite axis
    {vm::vec3d{nan, 0, 0}, vAxis,     {},                             Error{"UV axes are invalid"}},
    // parallel axes
    {uAxis,                uAxis,     {},                             Error{"UV axes do not form an invertible coordinate system"}},
    // a zero axis
    {vm::vec3d{0, 0, 0},   vAxis,     {},                             Error{"UV axes do not form an invertible coordinate system"}},
    // otherwise-fine axes, but a scale this large makes the resulting matrix look
    // singular under the fixed pivot threshold used to invert it
    {uAxis,                vAxis,     {{0, 0}, {1e30f, 1e30f}, 0.0f}, Error{"UV coordinate system is not invertible"}},
    }));
    // clang-format on

    CAPTURE(testUAxis, testVAxis, uvAttributes);
    CHECK(validateUvCoordSystem(testUAxis, testVAxis, normal, uvAttributes) == expected);
  }
}

} // namespace tb::mdl
