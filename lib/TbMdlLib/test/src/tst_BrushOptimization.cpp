/*
 Copyright (C) 2026

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

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushOptimization.h"
#include "mdl/CircleShape.h"
#include "mdl/MapFormat.h"

#include "kd/result.h"

#include "vm/bbox.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("BrushOptimization")
{
  SECTION("detect axis aligned cuboids")
  {
    const auto builder = BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}};
    const auto cuboid =
      builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 32, 16}}, "material")
      | kdl::value();
    const auto wedge = builder.createCylinder(
                         vm::bbox3d{{0, 0, 0}, {64, 32, 16}},
                         EdgeAlignedCircle{3},
                         vm::axis::z,
                         "material")
                       | kdl::value();

    CHECK(isAxisAlignedCuboid(cuboid));
    CHECK_FALSE(isAxisAlignedCuboid(wedge));
  }

  SECTION("merge adjacent cuboids")
  {
    const auto candidates = createBrushOptimizationCandidates(
      {vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, vm::bbox3d{{32, 0, 0}, {64, 64, 64}}});

    REQUIRE(candidates.size() == 1u);
    CHECK(candidates.front().bounds == std::vector{vm::bbox3d{{0, 0, 0}, {64, 64, 64}}});
    CHECK(candidates.front().internalFaceArea == 0.0);
  }

  SECTION("return alternative decompositions")
  {
    const auto candidates = createBrushOptimizationCandidates(
      {vm::bbox3d{{0, 0, 0}, {64, 32, 16}}, vm::bbox3d{{0, 32, 0}, {32, 64, 16}}});

    REQUIRE(candidates.size() == 1u);
    CHECK(
      candidates.front().bounds
      == std::vector{
        vm::bbox3d{{0, 0, 0}, {32, 64, 16}}, vm::bbox3d{{32, 0, 0}, {64, 32, 16}}});
    CHECK(candidates.front().internalFaceArea == 512.0);
  }

  SECTION("find an optimal decomposition missed by greedy axis sweeps")
  {
    // .#..
    // .##.
    // ##..
    // .#..
    const auto candidates = createBrushOptimizationCandidates({
      vm::bbox3d{{16, 0, 0}, {32, 16, 16}},
      vm::bbox3d{{0, 16, 0}, {16, 32, 16}},
      vm::bbox3d{{16, 16, 0}, {32, 32, 16}},
      vm::bbox3d{{16, 32, 0}, {32, 48, 16}},
      vm::bbox3d{{32, 32, 0}, {48, 48, 16}},
      vm::bbox3d{{16, 48, 0}, {32, 64, 16}},
    });

    REQUIRE_FALSE(candidates.empty());
    CHECK(candidates.front().bounds.size() == 3u);
    CHECK(
      candidates.front().bounds
      == std::vector{
        vm::bbox3d{{0, 16, 0}, {16, 32, 16}},
        vm::bbox3d{{16, 0, 0}, {32, 64, 16}},
        vm::bbox3d{{32, 32, 0}, {48, 48, 16}},
      });
  }

  SECTION("do not return the unchanged decomposition")
  {
    CHECK(
      createBrushOptimizationCandidates({vm::bbox3d{{0, 0, 0}, {16, 16, 16}}}).empty());
    CHECK(createBrushOptimizationCandidates(
            {vm::bbox3d{{0, 0, 0}, {16, 16, 16}}, vm::bbox3d{{32, 0, 0}, {48, 16, 16}}})
            .empty());
  }

  SECTION("remove contained cuboids")
  {
    const auto candidates = createBrushOptimizationCandidates(
      {vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, vm::bbox3d{{16, 16, 16}, {48, 48, 48}}});

    REQUIRE(candidates.size() == 1u);
    CHECK(candidates.front().bounds == std::vector{vm::bbox3d{{0, 0, 0}, {64, 64, 64}}});
  }
}

} // namespace tb::mdl
