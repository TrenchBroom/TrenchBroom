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
        {-5, 18, 29},   {-14, -1, 31}, {7, -9, 32},    {20, 13, 14},   {-20, 2, 23},
        {9, -10, 19},   {-23, 18, 22}, {-18, -14, 2},  {23, 5, 27},    {-32, 15, 11},
        {6, -32, -5},   {8, 32, 0},    {-23, -1, -14}, {32, -4, 9},    {-12, 25, -9},
        {11, -21, -24}, {19, 14, -32}, {-28, 3, -7},   {18, -3, -26},  {-4, 31, -12},
        {-25, -8, -29}, {28, 2, -28},  {-25, 22, -27}, {10, -25, -27}, {20, 28, -32},
        {-13, 9, -32},  {14, 6, -32},
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
        {22, 22, -32},
        {0, 31, -32},
        {-23, 21, -32},
        {-32, -1, -32},
        {-22, -23, -32},
        {0, -32, -32},
        {23, -23, -32},
        {27, 0, -17},
        {18, 23, -17},
        {-3, 32, -17},
        {-24, 22, -17},
        {-32, 0, -17},
        {-23, -23, -17},
        {-2, -32, -17},
        {19, -22, -17},
      },
      {
        {28, 6, -20},
        {16, 24, -20},
        {-6, 28, -20},
        {-24, 16, -20},
        {-28, -5, -20},
        {-16, -23, -20},
        {5, -27, -20},
        {24, -15, -20},
        {27, 6, -5},
        {16, 24, -5},
        {-3, 28, -5},
        {-19, 16, -5},
        {-23, -6, -5},
        {-12, -25, -5},
        {7, -29, -5},
        {24, -16, -5},
      },
      {
        {24, 5, -7},
        {13, 21, -7},
        {-6, 25, -7},
        {-22, 15, -7},
        {-26, -4, -7},
        {-16, -20, -7},
        {3, -24, -7},
        {19, -14, -7},
        {21, 3, 7},
        {11, 19, 7},
        {-7, 23, 7},
        {-22, 13, 7},
        {-26, -6, 7},
        {-16, -22, 7},
        {2, -27, 7},
        {17, -16, 7},
      },
      {
        {21, 1, 5},
        {14, 16, 5},
        {-2, 22, 5},
        {-17, 15, 5},
        {-23, 0, 5},
        {-16, -15, 5},
        {0, -21, 5},
        {15, -14, 5},
        {18, 3, 20},
        {12, 16, 20},
        {-1, 22, 20},
        {-14, 15, 20},
        {-19, 1, 20},
        {-13, -12, 20},
        {0, -18, 20},
        {13, -11, 20},
      },
      {
        {17, 3, 17},
        {11, 16, 17},
        {-2, 21, 17},
        {-15, 15, 17},
        {-20, 2, 17},
        {-14, -11, 17},
        {-1, -15, 17},
        {12, -10, 17},
        {13, 6, 32},
        {8, 19, 32},
        {-3, 24, 32},
        {-14, 18, 32},
        {-18, 5, 32},
        {-13, -8, 32},
        {-2, -13, 32},
        {9, -7, 32},
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
        {-2, 6, 29},     {-4, -1, 31},  {3, -5, 32},    {10, 5, 14},   {-8, -1, 23},
        {4, -7, 19},     {-9, 6, 22},   {-11, -11, 2},  {10, 1, 27},   {-17, 7, 11},
        {2, -21, -5},    {4, 17, 0},    {-14, -4, -14}, {13, -4, 9},   {-8, 12, -9},
        {6, -21, -24},   {14, 6, -32},  {-18, -1, -7},  {10, -6, -26}, {-4, 20, -12},
        {-17, -10, -29}, {16, -3, -28}, {-19, 12, -27}, {5, -23, -27}, {12, 15, -32},
        {-11, 3, -32},   {9, -1, -32},
      },
      {
        {23, -3, -11},
        {2, 9, -14},
        {21, -14, -15},
        {25, 12, -18},
        {1, 2, -18},
        {30, -9, -19},
        {15, 14, -24},
        {15, -16, -22},
        {27, 14, -19},
        {5, 4, -22},
        {23, -13, -32},
        {21, 19, -32},
        {9, -8, -27},
        {32, -6, -30},
        {14, 14, -31},
        {20, -5, -32},
        {25, 9, -32},
        {13, -2, -32},
      },
      {
        {-8, 19, -13},
        {-17, 28, -17},
        {-9, 10, -17},
        {2, 26, -18},
        {-13, 19, -16},
        {-2, 6, -19},
        {-9, 27, -18},
        {-15, 10, -26},
        {1, 21, -23},
        {-17, 24, -27},
        {-1, 3, -30},
        {-8, 32, -31},
        {-19, 7, -31},
        {2, 17, -32},
        {-18, 27, -29},
        {-8, 4, -31},
        {-5, 20, -32},
        {-9, 15, -32},
      },
      {
        {-8, -10, 7},
        {-24, 0, 0},
        {-12, -13, -3},
        {-11, -4, -3},
        {-24, -7, 0},
        {-6, -15, -1},
        {-15, 6, -7},
        {-24, -21, -17},
        {-7, -5, -20},
        {-32, -5, -26},
        {-16, -19, -26},
        {-11, 0, -16},
        {-24, -13, -27},
        {-4, -11, -27},
        {-20, -2, -32},
        {-18, -14, -32},
        {-15, -1, -32},
        {-17, -2, -32},
      },
      {
        {9, -14, -1},
        {2, -17, -6},
        {11, -24, -9},
        {19, -12, -14},
        {-2, -16, -13},
        {18, -20, -9},
        {5, -5, -16},
        {-2, -27, -14},
        {22, -14, -13},
        {-6, -12, -20},
        {9, -32, -28},
        {11, 0, -25},
        {3, -20, -31},
        {23, -20, -32},
        {8, -7, -32},
        {3, -26, -32},
        {15, -13, -32},
        {6, -18, -32},
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
        {-21, 6, -32},
        {-21, -11, -32},
        {-3, -21, -32},
        {15, -13, -32},
        {16, 5, -32},
        {-2, 15, -32},
        {-18, 6, 11},
        {-19, -11, 11},
        {-1, -21, 10},
        {18, -13, 10},
        {19, 5, 10},
        {1, 15, 10},
        {1, -3, 32},
      },
      {
        {12, -14, -32},
        {17, 1, -32},
        {4, 12, -32},
        {-14, 9, -32},
        {-19, -7, -32},
        {-6, -18, -32},
        {27, -12, 6},
        {32, 3, 4},
        {19, 14, 6},
        {1, 11, 9},
        {-4, -5, 10},
        {9, -16, 9},
        {20, 0, 26},
      },
      {
        {-6, 10, -32},
        {-16, 3, -28},
        {-11, -9, -25},
        {3, -14, -27},
        {13, -7, -32},
        {8, 5, -32},
        {11, 24, -12},
        {1, 17, -5},
        {5, 5, -2},
        {20, 0, -5},
        {30, 7, -11},
        {25, 19, -15},
        {22, 18, 1},
      },
      {
        {9, -3, -32},
        {7, 5, -32},
        {-4, 8, -32},
        {-14, 3, -32},
        {-12, -5, -31},
        {-1, -8, -29},
        {10, 20, -4},
        {8, 28, -9},
        {-4, 31, -11},
        {-13, 26, -8},
        {-11, 17, -3},
        {0, 14, -1},
        {-1, 32, 5},
      },
      {
        {-2, 8, -32},
        {-13, 4, -32},
        {-16, -5, -32},
        {-8, -10, -31},
        {2, -6, -28},
        {5, 3, -31},
        {-19, 19, -12},
        {-29, 16, -15},
        {-32, 7, -13},
        {-25, 1, -8},
        {-15, 5, -6},
        {-12, 14, -8},
        {-28, 15, -1},
      },
      {
        {-16, 1, -32},
        {-14, -11, -32},
        {-2, -15, -31},
        {7, -8, -28},
        {6, 4, -28},
        {-6, 9, -32},
        {-29, 1, -19},
        {-27, -11, -18},
        {-15, -15, -14},
        {-6, -8, -10},
        {-8, 4, -11},
        {-19, 8, -15},
        {-23, -3, -7},
      },
      {
        {2, 1, -26},
        {-10, 5, -29},
        {-18, -2, -32},
        {-14, -13, -32},
        {-2, -16, -32},
        {7, -9, -29},
        {-9, -7, -9},
        {-21, -4, -12},
        {-29, -11, -17},
        {-25, -21, -19},
        {-13, -25, -16},
        {-4, -18, -11},
        {-21, -18, -7},
      },
      {
        {8, -14, -32},
        {14, -5, -32},
        {4, 2, -25},
        {-11, 0, -26},
        {-17, -9, -32},
        {-7, -17, -32},
        {11, -30, -23},
        {16, -20, -17},
        {7, -13, -11},
        {-9, -16, -11},
        {-14, -25, -17},
        {-5, -32, -23},
        {2, -29, -11},
      },
      {
        {-13, -7, -26},
        {-7, -13, -32},
        {4, -10, -32},
        {10, -1, -32},
        {4, 5, -26},
        {-7, 2, -23},
        {1, -22, -11},
        {7, -28, -17},
        {19, -25, -21},
        {25, -16, -17},
        {19, -10, -11},
        {7, -13, -7},
        {19, -26, -8},
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
        {-21, -13, -32},
        {12, -13, -32},
        {12, 15, -32},
        {-21, 15, -32},
        {-20, -9, 32},
        {8, -9, 32},
        {8, 15, 32},
        {-20, 15, 32},
      },
      {
        {11, -10, -32},
        {32, -10, -32},
        {32, 2, -32},
        {11, 2, -32},
        {12, -9, 1},
        {30, -9, 1},
        {30, 1, 1},
        {12, 1, 1},
      },
      {
        {-5, 7, -32},
        {17, 7, -32},
        {17, 31, -32},
        {-5, 31, -32},
        {-4, 9, -9},
        {15, 9, -9},
        {15, 28, -9},
        {-4, 28, -9},
      },
      {
        {-26, 17, -32},
        {-4, 17, -32},
        {-4, 32, -32},
        {-26, 32, -32},
        {-24, 18, -15},
        {-6, 18, -15},
        {-6, 31, -15},
        {-24, 31, -15},
      },
      {
        {-25, 1, -32},
        {-12, 1, -32},
        {-12, 19, -32},
        {-25, 19, -32},
        {-24, 3, 3},
        {-13, 3, 3},
        {-13, 19, 3},
        {-24, 19, 3},
      },
      {
        {-32, -11, -32},
        {-12, -11, -32},
        {-12, 4, -32},
        {-32, 4, -32},
        {-29, -10, 13},
        {-12, -10, 13},
        {-12, 2, 13},
        {-29, 2, 13},
      },
      {
        {-21, -32, -32},
        {4, -32, -32},
        {4, -18, -32},
        {-21, -18, -32},
        {-20, -30, -16},
        {1, -30, -16},
        {1, -19, -16},
        {-20, -19, -16},
      },
      {
        {-2, -24, -32},
        {19, -24, -32},
        {19, -6, -32},
        {-2, -6, -32},
        {1, -24, -16},
        {18, -24, -16},
        {18, -9, -16},
        {1, -9, -16},
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
        {-16, -26, 26},
        {-20, -20, 26},
        {-28, -20, 26},
        {-32, -26, 26},
        {-28, -32, 26},
        {-20, -32, 26},
      },
      {
        {-16, -15, -32},
        {-20, -9, -32},
        {-28, -9, -32},
        {-32, -15, -32},
        {-28, -20, -32},
        {-20, -20, -32},
        {-16, -15, 10},
        {-20, -9, 10},
        {-28, -9, 10},
        {-32, -15, 10},
        {-28, -20, 10},
        {-20, -20, 10},
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
        {-16, 9, 18},
        {-20, 15, 18},
        {-28, 15, 18},
        {-32, 9, 18},
        {-28, 3, 18},
        {-20, 3, 18},
      },
      {
        {-16, 20, -32},
        {-20, 26, -32},
        {-28, 26, -32},
        {-32, 20, -32},
        {-28, 15, -32},
        {-20, 15, -32},
        {-16, 20, 19},
        {-20, 26, 19},
        {-28, 26, 19},
        {-32, 20, 19},
        {-28, 15, 19},
        {-20, 15, 19},
      },
      {
        {-4, -20, -32},
        {-8, -15, -32},
        {-16, -15, -32},
        {-20, -20, -32},
        {-16, -26, -32},
        {-8, -26, -32},
        {-4, -20, 28},
        {-8, -15, 28},
        {-16, -15, 28},
        {-20, -20, 28},
        {-16, -26, 28},
        {-8, -26, 28},
      },
      {
        {-4, -9, -32},
        {-8, -3, -32},
        {-16, -3, -32},
        {-20, -9, -32},
        {-16, -15, -32},
        {-8, -15, -32},
        {-4, -9, 17},
        {-8, -3, 17},
        {-16, -3, 17},
        {-20, -9, 17},
        {-16, -15, 17},
        {-8, -15, 17},
      },
      {
        {-4, 3, -32},
        {-8, 9, -32},
        {-16, 9, -32},
        {-20, 3, -32},
        {-16, -3, -32},
        {-8, -3, -32},
        {-4, 3, 21},
        {-8, 9, 21},
        {-16, 9, 21},
        {-20, 3, 21},
        {-16, -3, 21},
        {-8, -3, 21},
      },
      {
        {-4, 15, -32},
        {-8, 20, -32},
        {-16, 20, -32},
        {-20, 15, -32},
        {-16, 9, -32},
        {-8, 9, -32},
        {-4, 15, 13},
        {-8, 20, 13},
        {-16, 20, 13},
        {-20, 15, 13},
        {-16, 9, 13},
        {-8, 9, 13},
      },
      {
        {-4, 26, -32},
        {-8, 32, -32},
        {-16, 32, -32},
        {-20, 26, -32},
        {-16, 20, -32},
        {-8, 20, -32},
        {-4, 26, 12},
        {-8, 32, 12},
        {-16, 32, 12},
        {-20, 26, 12},
        {-16, 20, 12},
        {-8, 20, 12},
      },
      {
        {8, -26, -32},
        {4, -20, -32},
        {-4, -20, -32},
        {-8, -26, -32},
        {-4, -32, -32},
        {4, -32, -32},
        {8, -26, 21},
        {4, -20, 21},
        {-4, -20, 21},
        {-8, -26, 21},
        {-4, -32, 21},
        {4, -32, 21},
      },
      {
        {8, -15, -32},
        {4, -9, -32},
        {-4, -9, -32},
        {-8, -15, -32},
        {-4, -20, -32},
        {4, -20, -32},
        {8, -15, 17},
        {4, -9, 17},
        {-4, -9, 17},
        {-8, -15, 17},
        {-4, -20, 17},
        {4, -20, 17},
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
        {8, 9, 12},
        {4, 15, 12},
        {-4, 15, 12},
        {-8, 9, 12},
        {-4, 3, 12},
        {4, 3, 12},
      },
      {
        {8, 20, -32},
        {4, 26, -32},
        {-4, 26, -32},
        {-8, 20, -32},
        {-4, 15, -32},
        {4, 15, -32},
        {8, 20, 23},
        {4, 26, 23},
        {-4, 26, 23},
        {-8, 20, 23},
        {-4, 15, 23},
        {4, 15, 23},
      },
      {
        {20, -20, -32},
        {16, -15, -32},
        {8, -15, -32},
        {4, -20, -32},
        {8, -26, -32},
        {16, -26, -32},
        {20, -20, 25},
        {16, -15, 25},
        {8, -15, 25},
        {4, -20, 25},
        {8, -26, 25},
        {16, -26, 25},
      },
      {
        {20, -9, -32},
        {16, -3, -32},
        {8, -3, -32},
        {4, -9, -32},
        {8, -15, -32},
        {16, -15, -32},
        {20, -9, 21},
        {16, -3, 21},
        {8, -3, 21},
        {4, -9, 21},
        {8, -15, 21},
        {16, -15, 21},
      },
      {
        {20, 3, -32},
        {16, 9, -32},
        {8, 9, -32},
        {4, 3, -32},
        {8, -3, -32},
        {16, -3, -32},
        {20, 3, 14},
        {16, 9, 14},
        {8, 9, 14},
        {4, 3, 14},
        {8, -3, 14},
        {16, -3, 14},
      },
      {
        {20, 15, -32},
        {16, 20, -32},
        {8, 20, -32},
        {4, 15, -32},
        {8, 9, -32},
        {16, 9, -32},
        {20, 15, 16},
        {16, 20, 16},
        {8, 20, 16},
        {4, 15, 16},
        {8, 9, 16},
        {16, 9, 16},
      },
      {
        {20, 26, -32},
        {16, 32, -32},
        {8, 32, -32},
        {4, 26, -32},
        {8, 20, -32},
        {16, 20, -32},
        {20, 26, 19},
        {16, 32, 19},
        {8, 32, 19},
        {4, 26, 19},
        {8, 20, 19},
        {16, 20, 19},
      },
      {
        {32, -26, -32},
        {28, -20, -32},
        {20, -20, -32},
        {16, -26, -32},
        {20, -32, -32},
        {28, -32, -32},
        {32, -26, 24},
        {28, -20, 24},
        {20, -20, 24},
        {16, -26, 24},
        {20, -32, 24},
        {28, -32, 24},
      },
      {
        {32, -15, -32},
        {28, -9, -32},
        {20, -9, -32},
        {16, -15, -32},
        {20, -20, -32},
        {28, -20, -32},
        {32, -15, 27},
        {28, -9, 27},
        {20, -9, 27},
        {16, -15, 27},
        {20, -20, 27},
        {28, -20, 27},
      },
      {
        {32, -3, -32},
        {28, 3, -32},
        {20, 3, -32},
        {16, -3, -32},
        {20, -9, -32},
        {28, -9, -32},
        {32, -3, 18},
        {28, 3, 18},
        {20, 3, 18},
        {16, -3, 18},
        {20, -9, 18},
        {28, -9, 18},
      },
      {
        {32, 9, -32},
        {28, 15, -32},
        {20, 15, -32},
        {16, 9, -32},
        {20, 3, -32},
        {28, 3, -32},
        {32, 9, 19},
        {28, 15, 19},
        {20, 15, 19},
        {16, 9, 19},
        {20, 3, 19},
        {28, 3, 19},
      },
      {
        {32, 20, -32},
        {28, 26, -32},
        {20, 26, -32},
        {16, 20, -32},
        {20, 15, -32},
        {28, 15, -32},
        {32, 20, 27},
        {28, 26, 27},
        {20, 26, 27},
        {16, 20, 27},
        {20, 15, 27},
        {28, 15, 27},
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
        {-20, 24, 23},  {-24, 12, 25},  {-15, 7, 26},   {-10, 21, 10},  {-26, 13, 18},
        {-14, 6, 14},   {-28, 23, 17},  {-26, 3, -1},   {-8, 15, 21},   {-32, 21, 7},
        {-16, -9, -7},  {-15, 32, -3},  {-29, 10, -16}, {-4, 9, 5},     {-24, 27, -12},
        {-14, -3, -25}, {-10, 20, -32}, {-31, 14, -10}, {-11, 9, -27},  {-20, 31, -14},
        {-30, 6, -29},  {-7, 12, -28},  {-30, 25, -27}, {-14, -5, -27}, {-10, 29, -32},
        {-24, 17, -32}, {-12, 14, -32},
      },
      {
        {22, -16, 32},  {4, -2, 27},    {20, -27, 24}, {23, 0, 19},    {3, -9, 16},
        {28, -22, 14},  {15, 6, -1},    {15, -31, 10}, {24, 5, 19},    {6, -7, 9},
        {24, -32, -14}, {21, 15, -3},   {8, -24, -1},  {32, -21, -9},  {12, 8, -14},
        {20, -27, -23}, {26, 2, -2},    {-2, -14, -5}, {27, -20, -22}, {16, 11, -32},
        {9, -21, -26},  {31, -2, -16},  {6, -8, -19},  {23, -20, -31}, {19, 7, -32},
        {9, -13, -32},  {22, -10, -32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, RockType::Cluster);
    const auto expectedBrushes = buildBrushes(builder, clusterPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }
}

} // namespace tb::mdl
