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

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/CatchConfig.h"
#include "mdl/MapFormat.h"
#include "mdl/RockFormation.h"

#include "kd/range_fold.h"
#include "kd/ranges/to.h"
#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <cassert>
#include <ranges>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
namespace
{

const auto MaterialName = std::string{"someMaterial"};
const auto WorldBounds = vm::bbox3d{8192.0};
const auto Bounds = vm::bbox3d{{-32, -32, -32}, {32, 32, 32}};

// Matches DrawShapeToolRockExtension::RockResolution and the DrawShapeToolParameters
// defaults for rockBaseFlattening, rockForm and rockSeed.
constexpr auto Resolution = size_t{3};
constexpr auto BaseFlattening = 0.15;
constexpr auto Form = 0.5;
constexpr auto Seed = uint32_t{0};

std::vector<Brush> buildBrushes(
  const BrushBuilder& builder, const RockFormationPoints& formation)
{
  return formation | std::views::transform([&](const auto& points) {
           auto brush = builder.createBrush(points, MaterialName) | kdl::value();
           REQUIRE(brush.fullySpecified());
           return brush;
         })
         | kdl::ranges::to<std::vector>();
}

std::vector<Brush> generateActualBrushes(const BrushBuilder& builder, const RockType type)
{
  const auto points =
    makeRockFormation(Bounds, type, Resolution, BaseFlattening, Form, Seed);
  return buildBrushes(builder, points);
}

vm::bbox3d mergedBounds(const std::vector<Brush>& brushes)
{
  return kdl::fold_left_first(
    brushes | std::views::transform(&Brush::bounds),
    [](const auto& lhs, const auto& rhs) { return vm::merge(lhs, rhs); });
}

} // namespace

TEST_CASE("makeRockFormation")
{
  // Expected point clouds captured from the actual output of makeRockFormation for each
  // rock type, using the resolution used by the shape tool
  // (DrawShapeToolRockExtension::RockResolution) and the default parameter values from
  // DrawShapeToolParameters (rockBaseFlattening = 0.15, rockForm = 0.5, rockSeed = 0), in
  // a bounding box of {-32, -32, -32} to {32, 32, 32}. These tests only guard against
  // future regressions and do not verify that the generated shapes are correct.

  const auto builder = BrushBuilder{MapFormat::Standard, WorldBounds};

  SECTION("Boulder")
  {
    const auto boulderPoints = RockFormationPoints{
      {
        {20, -4, 25},   {-27, 5, 21},  {-12, -12, 32}, {7, 15, 22},   {-31, 3, 15},
        {16, -7, 16},   {1, 25, 8},    {-11, -14, 19}, {14, -4, 20},  {-24, 5, 0},
        {10, -21, -10}, {13, 30, -8},  {-25, -24, 7},  {22, -21, 3},  {-19, 29, 2},
        {9, -32, -20},  {20, 24, -27}, {-32, 6, -5},   {32, -16, -8}, {-5, 32, -12},
        {-8, -15, -26}, {28, 8, -25},  {-22, 17, -32}, {4, -8, -31},  {7, 27, -23},
        {-13, 3, -32},  {13, -4, -32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Boulder);
    const auto expectedBrushes = buildBrushes(builder, boulderPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Shelf")
  {
    const auto shelfPoints = RockFormationPoints{
      {
        {32, 0, -32},
        {23, 23, -32},
        {0, 32, -32},
        {-23, 23, -32},
        {-32, 0, -32},
        {-23, -23, -32},
        {0, -32, -32},
        {23, -23, -32},
        {26, -2, 32},
        {20, 14, 32},
        {4, 21, 32},
        {-12, 14, 32},
        {-19, -2, 32},
        {-12, -18, 32},
        {4, -24, 32},
        {20, -18, 32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Shelf);
    const auto expectedBrushes = buildBrushes(builder, shelfPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Strata")
  {
    const auto strataPoints = RockFormationPoints{
      {
        {32, 0, -32},
        {23, 23, -32},
        {0, 32, -32},
        {-22, 22, -32},
        {-31, 0, -32},
        {-22, -23, -32},
        {1, -32, -32},
        {23, -22, -32},
        {31, -2, -17},
        {22, 19, -17},
        {-1, 28, -17},
        {-23, 19, -17},
        {-32, -2, -17},
        {-23, -23, -17},
        {0, -32, -17},
        {22, -23, -17},
      },
      {
        {29, 5, -20},
        {17, 23, -20},
        {-4, 27, -20},
        {-22, 14, -20},
        {-26, -7, -20},
        {-14, -25, -20},
        {7, -29, -20},
        {25, -17, -20},
        {26, 6, -5},
        {15, 25, -5},
        {-4, 29, -5},
        {-20, 16, -5},
        {-23, -7, -5},
        {-12, -26, -5},
        {7, -31, -5},
        {23, -17, -5},
      },
      {
        {27, 2, -7},
        {18, 19, -7},
        {0, 24, -7},
        {-17, 15, -7},
        {-23, -4, -7},
        {-13, -21, -7},
        {5, -26, -7},
        {22, -17, -7},
        {28, 4, 7},
        {19, 20, 7},
        {2, 25, 7},
        {-14, 16, 7},
        {-20, -1, 7},
        {-11, -18, 7},
        {7, -23, 7},
        {23, -14, 7},
      },
      {
        {25, 2, 5},
        {17, 17, 5},
        {1, 23, 5},
        {-13, 15, 5},
        {-19, -1, 5},
        {-11, -16, 5},
        {5, -21, 5},
        {19, -13, 5},
        {24, 1, 20},
        {16, 16, 20},
        {1, 22, 20},
        {-14, 14, 20},
        {-19, -2, 20},
        {-11, -18, 20},
        {4, -23, 20},
        {18, -15, 20},
      },
      {
        {21, 7, 17},
        {13, 18, 17},
        {0, 21, 17},
        {-12, 13, 17},
        {-14, -1, 17},
        {-7, -13, 17},
        {7, -15, 17},
        {19, -7, 17},
        {16, 8, 32},
        {9, 19, 32},
        {-2, 22, 32},
        {-12, 14, 32},
        {-15, 1, 32},
        {-8, -9, 32},
        {4, -12, 32},
        {14, -5, 32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Strata);
    const auto expectedBrushes = buildBrushes(builder, strataPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Crag")
  {
    const auto cragPoints = RockFormationPoints{
      {
        {12, 2, 25},    {-11, 7, 21},  {-1, -2, 32},   {6, 9, 22},   {-15, 5, 15},
        {11, 0, 16},    {4, 20, 8},    {-2, -5, 19},   {9, 2, 20},   {-15, 6, 0},
        {9, -15, -10},  {10, 22, -8},  {-13, -13, 7},  {18, -12, 3}, {-7, 18, 2},
        {8, -23, -20},  {18, 19, -27}, {-22, 7, -5},   {24, -9, -8}, {-2, 27, -12},
        {-5, -13, -26}, {26, 8, -25},  {-17, 15, -32}, {6, -7, -31}, {8, 23, -23},
        {-6, 2, -32},   {13, -3, -32},
      },
      {
        {23, 23, -12},
        {17, 17, -14},
        {17, 4, -13},
        {25, 22, -19},
        {8, 8, -22},
        {30, 4, -20},
        {8, 29, -26},
        {10, 4, -16},
        {32, 23, -23},
        {7, 26, -27},
        {28, 5, -25},
        {23, 29, -27},
        {9, 6, -25},
        {28, 12, -30},
        {12, 19, -29},
        {23, 7, -32},
        {24, 15, -32},
        {11, 18, -32},
      },
      {
        {-7, 18, -13},
        {-14, 30, -14},
        {-13, 10, -18},
        {-6, 27, -13},
        {-20, 20, -16},
        {2, 20, -20},
        {-10, 32, -17},
        {-14, 13, -21},
        {0, 29, -22},
        {-18, 28, -26},
        {-5, 9, -30},
        {-6, 30, -28},
        {-17, 12, -30},
        {-1, 22, -32},
        {-12, 27, -32},
        {-11, 14, -32},
        {-6, 28, -32},
        {-9, 25, -32},
      },
      {
        {-11, -1, -12},
        {-19, 9, -11},
        {-22, -7, -10},
        {-9, 5, -13},
        {-28, 0, -15},
        {-5, -1, -19},
        {-19, 15, -13},
        {-26, -8, -23},
        {2, 4, -25},
        {-32, 10, -23},
        {-11, -8, -29},
        {-17, 15, -29},
        {-26, 0, -32},
        {-3, 5, -32},
        {-18, 15, -29},
        {-19, -10, -32},
        {-10, 2, -32},
        {-26, 2, -32},
      },
      {
        {13, -15, 7},
        {6, -9, -3},
        {10, -28, -10},
        {18, -3, -4},
        {4, -17, -13},
        {26, -26, -15},
        {15, -1, -15},
        {9, -32, -7},
        {24, -9, -15},
        {0, -8, -19},
        {26, -25, -20},
        {16, 3, -28},
        {4, -22, -31},
        {23, -14, -28},
        {8, -4, -25},
        {10, -22, -32},
        {25, -9, -32},
        {13, -19, -32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Crag);
    const auto expectedBrushes = buildBrushes(builder, cragPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Crystal")
  {
    const auto crystalPoints = RockFormationPoints{
      {
        {16, 12, -32},
        {-2, 16, -32},
        {-13, 1, -32},
        {-8, -17, -32},
        {9, -21, -32},
        {21, -7, -32},
        {18, 12, 10},
        {1, 16, 10},
        {-11, 1, 11},
        {-6, -17, 11},
        {11, -21, 10},
        {23, -7, 10},
        {7, -3, 32},
      },
      {
        {8, 14, -32},
        {-6, 9, -32},
        {-10, -8, -32},
        {2, -20, -32},
        {16, -14, -32},
        {20, 3, -32},
        {21, 14, 7},
        {6, 8, 9},
        {3, -9, 10},
        {14, -20, 8},
        {29, -15, 5},
        {32, 2, 4},
        {23, -3, 26},
      },
      {
        {1, 8, -32},
        {-3, 2, -26},
        {1, -8, -24},
        {10, -11, -28},
        {14, -5, -32},
        {10, 5, -32},
        {14, 19, -17},
        {10, 13, -12},
        {14, 3, -10},
        {22, 0, -14},
        {27, 6, -20},
        {23, 16, -21},
        {24, 14, -10},
      },
      {
        {16, 2, -32},
        {9, 10, -32},
        {-3, 10, -32},
        {-8, 0, -30},
        {-1, -9, -25},
        {11, -8, -26},
        {17, 23, -11},
        {10, 32, -17},
        {-3, 31, -16},
        {-8, 22, -10},
        {-1, 13, -5},
        {12, 14, -5},
        {5, 31, -2},
      },
      {
        {1, 11, -32},
        {-9, 1, -32},
        {-8, -10, -32},
        {2, -12, -25},
        {12, -2, -25},
        {11, 10, -31},
        {-11, 25, -24},
        {-21, 15, -24},
        {-20, 3, -17},
        {-10, 2, -10},
        {0, 12, -10},
        {-1, 23, -17},
        {-16, 19, -11},
      },
      {
        {10, -2, -26},
        {5, 8, -29},
        {-4, 6, -32},
        {-6, -5, -32},
        {-1, -14, -32},
        {7, -13, -29},
        {-14, -5, -1},
        {-19, 4, -4},
        {-28, 3, -10},
        {-30, -8, -13},
        {-25, -18, -10},
        {-17, -16, -4},
        {-32, -8, 3},
      },
      {
        {9, -12, -32},
        {9, -3, -31},
        {1, 3, -32},
        {-6, 0, -32},
        {-6, -9, -32},
        {1, -15, -32},
        {-6, -29, -5},
        {-6, -20, -2},
        {-13, -14, -3},
        {-20, -17, -7},
        {-20, -26, -10},
        {-13, -32, -9},
        {-19, -30, 6},
      },
      {
        {-1, 4, -27},
        {-7, -5, -31},
        {-2, -14, -32},
        {9, -14, -32},
        {15, -5, -32},
        {10, 4, -28},
        {1, -11, -9},
        {-5, -20, -13},
        {0, -29, -18},
        {10, -29, -18},
        {16, -20, -14},
        {11, -11, -10},
        {6, -26, -6},
      },
      {
        {16, 1, -32},
        {8, 10, -28},
        {-4, 4, -25},
        {-7, -9, -28},
        {2, -17, -32},
        {13, -12, -32},
        {24, -6, -18},
        {15, 2, -12},
        {3, -3, -10},
        {0, -16, -13},
        {9, -25, -18},
        {20, -20, -21},
        {15, -15, -9},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Crystal);
    const auto expectedBrushes = buildBrushes(builder, crystalPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Columns")
  {
    const auto columnsPoints = RockFormationPoints{
      {
        {-14, -18, -32},
        {12, -18, -32},
        {12, 10, -32},
        {-14, 10, -32},
        {-11, -16, 32},
        {12, -16, 32},
        {12, 7, 32},
        {-11, 7, 32},
      },
      {
        {20, -19, -32},
        {32, -19, -32},
        {32, -3, -32},
        {20, -3, -32},
        {22, -18, -14},
        {32, -18, -14},
        {32, -5, -14},
        {22, -5, -14},
      },
      {
        {-3, 4, -32},
        {17, 4, -32},
        {17, 22, -32},
        {-3, 22, -32},
        {-1, 6, -1},
        {16, 6, -1},
        {16, 21, -1},
        {-1, 21, -1},
      },
      {
        {-13, 10, -32},
        {0, 10, -32},
        {0, 32, -32},
        {-13, 32, -32},
        {-11, 13, 9},
        {0, 13, 9},
        {0, 32, 9},
        {-11, 32, 9},
      },
      {
        {-32, 2, -32},
        {-15, 2, -32},
        {-15, 18, -32},
        {-32, 18, -32},
        {-30, 4, -12},
        {-16, 4, -12},
        {-16, 18, -12},
        {-30, 18, -12},
      },
      {
        {-18, -20, -32},
        {-7, -20, -32},
        {-7, -8, -32},
        {-18, -8, -32},
        {-18, -19, 8},
        {-8, -19, 8},
        {-8, -10, 8},
        {-18, -10, 8},
      },
      {
        {-15, -32, -32},
        {0, -32, -32},
        {0, -14, -32},
        {-15, -14, -32},
        {-14, -31, -18},
        {-2, -31, -18},
        {-2, -16, -18},
        {-14, -16, -18},
      },
      {
        {12, -32, -32},
        {26, -32, -32},
        {26, -16, -32},
        {12, -16, -32},
        {12, -30, -4},
        {24, -30, -4},
        {24, -16, -4},
        {12, -16, -4},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Columns);
    const auto expectedBrushes = buildBrushes(builder, columnsPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Basalt")
  {
    const auto basaltPoints = RockFormationPoints{
      {
        {-16, -26, -32},
        {-20, -20, -32},
        {-28, -20, -32},
        {-32, -26, -32},
        {-28, -32, -32},
        {-20, -32, -32},
        {-16, -26, 12},
        {-20, -20, 12},
        {-28, -20, 12},
        {-32, -26, 12},
        {-28, -32, 12},
        {-20, -32, 12},
      },
      {
        {-16, -15, -32},
        {-20, -9, -32},
        {-28, -9, -32},
        {-32, -15, -32},
        {-28, -20, -32},
        {-20, -20, -32},
        {-16, -15, 21},
        {-20, -9, 21},
        {-28, -9, 21},
        {-32, -15, 21},
        {-28, -20, 21},
        {-20, -20, 21},
      },
      {
        {-16, -3, -32},
        {-20, 3, -32},
        {-28, 3, -32},
        {-32, -3, -32},
        {-28, -9, -32},
        {-20, -9, -32},
        {-16, -3, 29},
        {-20, 3, 29},
        {-28, 3, 29},
        {-32, -3, 29},
        {-28, -9, 29},
        {-20, -9, 29},
      },
      {
        {-16, 9, -32},
        {-20, 15, -32},
        {-28, 15, -32},
        {-32, 9, -32},
        {-28, 3, -32},
        {-20, 3, -32},
        {-16, 9, 11},
        {-20, 15, 11},
        {-28, 15, 11},
        {-32, 9, 11},
        {-28, 3, 11},
        {-20, 3, 11},
      },
      {
        {-16, 20, -32},
        {-20, 26, -32},
        {-28, 26, -32},
        {-32, 20, -32},
        {-28, 15, -32},
        {-20, 15, -32},
        {-16, 20, 27},
        {-20, 26, 27},
        {-28, 26, 27},
        {-32, 20, 27},
        {-28, 15, 27},
        {-20, 15, 27},
      },
      {
        {-4, -20, -32},
        {-8, -15, -32},
        {-16, -15, -32},
        {-20, -20, -32},
        {-16, -26, -32},
        {-8, -26, -32},
        {-4, -20, 23},
        {-8, -15, 23},
        {-16, -15, 23},
        {-20, -20, 23},
        {-16, -26, 23},
        {-8, -26, 23},
      },
      {
        {-4, -9, -32},
        {-8, -3, -32},
        {-16, -3, -32},
        {-20, -9, -32},
        {-16, -15, -32},
        {-8, -15, -32},
        {-4, -9, 26},
        {-8, -3, 26},
        {-16, -3, 26},
        {-20, -9, 26},
        {-16, -15, 26},
        {-8, -15, 26},
      },
      {
        {-4, 3, -32},
        {-8, 9, -32},
        {-16, 9, -32},
        {-20, 3, -32},
        {-16, -3, -32},
        {-8, -3, -32},
        {-4, 3, 15},
        {-8, 9, 15},
        {-16, 9, 15},
        {-20, 3, 15},
        {-16, -3, 15},
        {-8, -3, 15},
      },
      {
        {-4, 15, -32},
        {-8, 20, -32},
        {-16, 20, -32},
        {-20, 15, -32},
        {-16, 9, -32},
        {-8, 9, -32},
        {-4, 15, 25},
        {-8, 20, 25},
        {-16, 20, 25},
        {-20, 15, 25},
        {-16, 9, 25},
        {-8, 9, 25},
      },
      {
        {-4, 26, -32},
        {-8, 32, -32},
        {-16, 32, -32},
        {-20, 26, -32},
        {-16, 20, -32},
        {-8, 20, -32},
        {-4, 26, 11},
        {-8, 32, 11},
        {-16, 32, 11},
        {-20, 26, 11},
        {-16, 20, 11},
        {-8, 20, 11},
      },
      {
        {8, -26, -32},
        {4, -20, -32},
        {-4, -20, -32},
        {-8, -26, -32},
        {-4, -32, -32},
        {4, -32, -32},
        {8, -26, 22},
        {4, -20, 22},
        {-4, -20, 22},
        {-8, -26, 22},
        {-4, -32, 22},
        {4, -32, 22},
      },
      {
        {8, -15, -32},
        {4, -9, -32},
        {-4, -9, -32},
        {-8, -15, -32},
        {-4, -20, -32},
        {4, -20, -32},
        {8, -15, 15},
        {4, -9, 15},
        {-4, -9, 15},
        {-8, -15, 15},
        {-4, -20, 15},
        {4, -20, 15},
      },
      {
        {8, -3, -32},
        {4, 3, -32},
        {-4, 3, -32},
        {-8, -3, -32},
        {-4, -9, -32},
        {4, -9, -32},
        {8, -3, 32},
        {4, 3, 32},
        {-4, 3, 32},
        {-8, -3, 32},
        {-4, -9, 32},
        {4, -9, 32},
      },
      {
        {8, 9, -32},
        {4, 15, -32},
        {-4, 15, -32},
        {-8, 9, -32},
        {-4, 3, -32},
        {4, 3, -32},
        {8, 9, 19},
        {4, 15, 19},
        {-4, 15, 19},
        {-8, 9, 19},
        {-4, 3, 19},
        {4, 3, 19},
      },
      {
        {8, 20, -32},
        {4, 26, -32},
        {-4, 26, -32},
        {-8, 20, -32},
        {-4, 15, -32},
        {4, 15, -32},
        {8, 20, 19},
        {4, 26, 19},
        {-4, 26, 19},
        {-8, 20, 19},
        {-4, 15, 19},
        {4, 15, 19},
      },
      {
        {20, -20, -32},
        {16, -15, -32},
        {8, -15, -32},
        {4, -20, -32},
        {8, -26, -32},
        {16, -26, -32},
        {20, -20, 16},
        {16, -15, 16},
        {8, -15, 16},
        {4, -20, 16},
        {8, -26, 16},
        {16, -26, 16},
      },
      {
        {20, -9, -32},
        {16, -3, -32},
        {8, -3, -32},
        {4, -9, -32},
        {8, -15, -32},
        {16, -15, -32},
        {20, -9, 19},
        {16, -3, 19},
        {8, -3, 19},
        {4, -9, 19},
        {8, -15, 19},
        {16, -15, 19},
      },
      {
        {20, 3, -32},
        {16, 9, -32},
        {8, 9, -32},
        {4, 3, -32},
        {8, -3, -32},
        {16, -3, -32},
        {20, 3, 20},
        {16, 9, 20},
        {8, 9, 20},
        {4, 3, 20},
        {8, -3, 20},
        {16, -3, 20},
      },
      {
        {20, 15, -32},
        {16, 20, -32},
        {8, 20, -32},
        {4, 15, -32},
        {8, 9, -32},
        {16, 9, -32},
        {20, 15, 15},
        {16, 20, 15},
        {8, 20, 15},
        {4, 15, 15},
        {8, 9, 15},
        {16, 9, 15},
      },
      {
        {20, 26, -32},
        {16, 32, -32},
        {8, 32, -32},
        {4, 26, -32},
        {8, 20, -32},
        {16, 20, -32},
        {20, 26, 26},
        {16, 32, 26},
        {8, 32, 26},
        {4, 26, 26},
        {8, 20, 26},
        {16, 20, 26},
      },
      {
        {32, -26, -32},
        {28, -20, -32},
        {20, -20, -32},
        {16, -26, -32},
        {20, -32, -32},
        {28, -32, -32},
        {32, -26, 13},
        {28, -20, 13},
        {20, -20, 13},
        {16, -26, 13},
        {20, -32, 13},
        {28, -32, 13},
      },
      {
        {32, -15, -32},
        {28, -9, -32},
        {20, -9, -32},
        {16, -15, -32},
        {20, -20, -32},
        {28, -20, -32},
        {32, -15, 13},
        {28, -9, 13},
        {20, -9, 13},
        {16, -15, 13},
        {20, -20, 13},
        {28, -20, 13},
      },
      {
        {32, -3, -32},
        {28, 3, -32},
        {20, 3, -32},
        {16, -3, -32},
        {20, -9, -32},
        {28, -9, -32},
        {32, -3, 17},
        {28, 3, 17},
        {20, 3, 17},
        {16, -3, 17},
        {20, -9, 17},
        {28, -9, 17},
      },
      {
        {32, 9, -32},
        {28, 15, -32},
        {20, 15, -32},
        {16, 9, -32},
        {20, 3, -32},
        {28, 3, -32},
        {32, 9, 12},
        {28, 15, 12},
        {20, 15, 12},
        {16, 9, 12},
        {20, 3, 12},
        {28, 3, 12},
      },
      {
        {32, 20, -32},
        {28, 26, -32},
        {20, 26, -32},
        {16, 20, -32},
        {20, 15, -32},
        {28, 15, -32},
        {32, 20, 23},
        {28, 26, 23},
        {20, 26, 23},
        {16, 20, 23},
        {20, 15, 23},
        {28, 15, 23},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Basalt);
    const auto expectedBrushes = buildBrushes(builder, basaltPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Cluster")
  {
    const auto clusterPoints = RockFormationPoints{
      {
        {5, 3, 25},     {-27, 11, 21}, {-17, -4, 32},  {-4, 19, 22},  {-31, 8, 15},
        {1, 0, 16},     {-9, 27, 8},   {-17, -6, 19},  {0, 3, 20},    {-27, 10, 0},
        {-4, -13, -10}, {-2, 31, -8},  {-27, -14, 7},  {5, -12, 3},   {-23, 30, 2},
        {-5, -22, -20}, {3, 24, -27},  {-32, 10, -5},  {11, -8, -8},  {-14, 32, -12},
        {-16, -8, -26}, {8, 12, -25},  {-25, 18, -32}, {-7, -2, -31}, {-6, 27, -23},
        {-18, 6, -32},  {-1, 1, -32},
      },
      {
        {15, 17, 28},  {10, 4, 23},   {9, -13, 27}, {20, 13, 11},   {-2, -11, 8},
        {25, -16, 15}, {-6, 25, -2},  {2, -11, 24}, {29, 16, 3},    {-7, 20, -4},
        {23, -16, 3},  {17, 26, -3},  {-3, -15, 2}, {29, -7, -8},   {-2, 12, -13},
        {15, -32, -6}, {27, 10, -6},  {-8, 6, -8},  {25, -7, -24},  {14, 21, -26},
        {0, -21, -19}, {32, -5, -27}, {-1, 5, -32}, {16, -16, -22}, {25, 12, -32},
        {11, -2, -32}, {15, 12, -32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Cluster);
    const auto expectedBrushes = buildBrushes(builder, clusterPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }
}

} // namespace tb::mdl
