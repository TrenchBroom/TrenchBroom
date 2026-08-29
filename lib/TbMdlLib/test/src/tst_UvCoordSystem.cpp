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

#include "base/Result.h"
#include "mdl/CatchConfig.h"
#include "mdl/ParallelUvCoordSystem.h"
#include "mdl/ParaxialUvCoordSystem.h"
#include "mdl/UvAttributes.h"
#include "mdl/UvCoordSystem.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <optional>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("UvCoordSystem")
{
  SECTION("takeSnapshot")
  {
    SECTION("paraxial UV axes cannot be snapshotted")
    {
      const auto paraxial =
        UvCoordSystem{ParaxialUvCoordSystem{vm::vec3d{0, 0, 1}, UvAttributes{}}};
      CHECK(paraxial.takeSnapshot() == std::nullopt);
    }

    SECTION("parallel UV axes can be snapshotted and restored")
    {
      auto parallel = UvCoordSystem{
        ParallelUvCoordSystem{vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, UvAttributes{}}};
      const auto snapshot = parallel.takeSnapshot();
      REQUIRE(
        snapshot
        == std::optional{UvCoordSystemSnapshot{vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}}});

      parallel.reset(vm::vec3d{0, 0, 1});
      REQUIRE(parallel.takeSnapshot() != snapshot);

      parallel.restoreSnapshot(*snapshot);
      CHECK(parallel.uAxis() == vm::vec3d{0, 1, 0});
      CHECK(parallel.vAxis() == vm::vec3d{1, 0, 0});
    }
  }

  SECTION("translate")
  {
    const auto normal = vm::vec3d{0, 0, 1};
    const auto offset = vm::vec2f{2, 3};

    SECTION("uses the U axis as horizontal if it is closer to the right axis")
    {
      auto system = UvCoordSystem{
        ParallelUvCoordSystem{vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{}}};
      system.translate(normal, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, offset);
      CHECK(system.uvAttributes().offset == vm::vec2f{-2, -3});
    }

    SECTION("uses the V axis as horizontal if it is closer to the right axis")
    {
      auto system = UvCoordSystem{
        ParallelUvCoordSystem{vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, UvAttributes{}}};
      system.translate(normal, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, offset);
      CHECK(system.uvAttributes().offset == vm::vec2f{-3, -2});
    }

    SECTION(
      "falls back to comparing against the up axis if neither axis is clearly closer "
      "to the right axis")
    {
      const auto right = vm::normalize(vm::vec3d{1, 1, 0});

      SECTION("uses the V axis as horizontal if the U axis is closer to the up axis")
      {
        auto system = UvCoordSystem{
          ParallelUvCoordSystem{vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{}}};
        system.translate(normal, vm::vec3d{1, 0, 0}, right, offset);
        CHECK(system.uvAttributes().offset == vm::vec2f{-3, -2});
      }

      SECTION("uses the U axis as horizontal if the V axis is closer to the up axis")
      {
        auto system = UvCoordSystem{
          ParallelUvCoordSystem{vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{}}};
        system.translate(normal, vm::vec3d{0, 1, 0}, right, offset);
        CHECK(system.uvAttributes().offset == vm::vec2f{-2, -3});
      }

      SECTION("does nothing if neither axis can be clearly chosen")
      {
        auto system = UvCoordSystem{
          ParallelUvCoordSystem{vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{}}};
        const auto up = vm::normalize(vm::vec3d{1, -1, 0});
        system.translate(normal, up, right, offset);
        CHECK(system.uvAttributes().offset == vm::vec2f{0, 0});
      }
    }

    SECTION("flips the offset direction for a negative scale")
    {
      auto system = UvCoordSystem{ParallelUvCoordSystem{
        vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{{}, {-1, -1}, 0.0f}}};
      system.translate(normal, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, offset);
      CHECK(system.uvAttributes().offset == vm::vec2f{2, 3});
    }
  }
}

TEST_CASE("ParallelUvCoordSystem")
{
  SECTION("createFromPoints")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto point0 = vm::vec3d{0, 0, 0};
    const auto point1 = vm::vec3d{1, 0, 0};
    const auto point2 = vm::vec3d{0, 1, 0};

    SECTION("returns the same axes and attributes as the constructor")
    {
      const auto expected = ParallelUvCoordSystem{point0, point1, point2, uvAttributes};

      const auto actual =
        ParallelUvCoordSystem::createFromPoints(point0, point1, point2, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uAxis() == expected.uAxis());
      CHECK(actual.value().vAxis() == expected.vAxis());
      CHECK(actual.value().uvAttributes() == expected.uvAttributes());
    }

    SECTION("returns an error if the points do not define a plane")
    {
      const auto degeneratePoint2 = vm::vec3d{2, 0, 0};
      CHECK(
        ParallelUvCoordSystem::createFromPoints(
          point0, point1, degeneratePoint2, uvAttributes)
        == Result<ParallelUvCoordSystem>{Error{"Face points do not define a plane"}});
    }
  }

  SECTION("createFromAxes")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto uAxis = vm::vec3d{1, 0, 0};
    const auto vAxis = vm::vec3d{0, 1, 0};

    SECTION("returns the same axes and attributes as the constructor")
    {
      const auto expected = ParallelUvCoordSystem{uAxis, vAxis, uvAttributes};

      const auto actual =
        ParallelUvCoordSystem::createFromAxes(uAxis, vAxis, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uAxis() == expected.uAxis());
      CHECK(actual.value().vAxis() == expected.vAxis());
      CHECK(actual.value().uvAttributes() == expected.uvAttributes());
    }
  }
}

TEST_CASE("ParaxialUvCoordSystem")
{
  SECTION("createFromPoints")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto point0 = vm::vec3d{0, 0, 0};
    const auto point1 = vm::vec3d{1, 0, 0};
    const auto point2 = vm::vec3d{0, 1, 0};

    SECTION("returns the same axes and attributes as the constructor")
    {
      const auto expected = ParaxialUvCoordSystem{point0, point1, point2, uvAttributes};

      const auto actual =
        ParaxialUvCoordSystem::createFromPoints(point0, point1, point2, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uAxis() == expected.uAxis());
      CHECK(actual.value().vAxis() == expected.vAxis());
      CHECK(actual.value().uvAttributes() == expected.uvAttributes());
    }

    SECTION("returns an error if the points do not define a plane")
    {
      const auto degeneratePoint2 = vm::vec3d{2, 0, 0};
      CHECK(
        ParaxialUvCoordSystem::createFromPoints(
          point0, point1, degeneratePoint2, uvAttributes)
        == Result<ParaxialUvCoordSystem>{Error{"Face points do not define a plane"}});
    }
  }

  SECTION("createFromNormal")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto normal = vm::vec3d{0, 0, 1};

    SECTION("returns the same axes and attributes as the constructor")
    {
      const auto expected = ParaxialUvCoordSystem{normal, uvAttributes};

      const auto actual = ParaxialUvCoordSystem::createFromNormal(normal, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uAxis() == expected.uAxis());
      CHECK(actual.value().vAxis() == expected.vAxis());
      CHECK(actual.value().uvAttributes() == expected.uvAttributes());
    }
  }
}

} // namespace tb::mdl
