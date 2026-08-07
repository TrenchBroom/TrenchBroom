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

} // namespace tb::mdl
