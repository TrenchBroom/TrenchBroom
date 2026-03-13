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

#include "mdl/CatchConfig.h"
#include "mdl/UVUtils.h"

#include "vm/approx.h"

#include <tuple>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("computeCameraAxesForFaceNormal")
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

} // namespace tb::mdl
