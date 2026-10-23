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
#include "mdl/BrushFace.h"
#include "mdl/CatchConfig.h"
#include "mdl/MapFormat.h"
#include "mdl/RockFormation.h"

#include "kd/ranges/fold.h"
#include "kd/ranges/to.h"
#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
namespace
{

const auto MaterialName = std::string{"someMaterial"};
const auto WorldBounds = vm::bbox3d{8192.0};
const auto Bounds = vm::bbox3d{{-32, -32, -32}, {32, 32, 32}};

// Basalt and Cluster derive the number of their pieces from the size of the bounds, and
// only create the minimum number of pieces in the smaller bounds.
const auto FieldBounds = vm::bbox3d{{-128, -128, -32}, {128, 128, 32}};

// Matches the DrawShapeToolParameters defaults for rockDetail and rockSeed.
constexpr auto Detail = 0.5;
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

std::vector<Brush> generateActualBrushes(
  const BrushBuilder& builder, const vm::bbox3d& bounds, const RockType type)
{
  const auto points = makeRockFormation(bounds, type, Detail, Seed);
  return buildBrushes(builder, points);
}

size_t numPoints(const RockFormationPoints& formation)
{
  return size_t(std::ranges::distance(formation | std::views::join));
}

vm::bbox3d mergedBounds(const std::vector<Brush>& brushes)
{
  contract_pre(!brushes.empty());

  return *kdl::ranges::fold_left_first(
    brushes | std::views::transform(&Brush::bounds),
    [](const auto& lhs, const auto& rhs) { return vm::merge(lhs, rhs); });
}

// The points of a slanted top are not rounded, so they can only be compared
// approximately.
bool approxEqual(const std::vector<vm::vec3d>& lhs, const std::vector<vm::vec3d>& rhs)
{
  return std::ranges::equal(lhs, rhs, [](const auto& lhsPoint, const auto& rhsPoint) {
    return vm::is_equal(lhsPoint, rhsPoint, 0.001);
  });
}

template <std::ranges::range R>
bool isFlat(R&& points)
{
  const auto [minZ, maxZ] = std::ranges::minmax(
    points | std::views::transform([](const auto& point) { return point.z(); }));
  return minZ == maxZ;
}

} // namespace

TEST_CASE("makeRockFormation")
{
  // Expected point clouds captured from the actual output of makeRockFormation for each
  // rock type, using the default parameter values from DrawShapeToolParameters
  // (rockDetail = 0.5, rockSeed = 0), in a bounding box of {-32, -32, -32} to
  // {32, 32, 32}, or {-128, -128, -32} to {128, 128, 32} for Basalt and Cluster. These
  // tests only guard against future regressions and do not verify that the generated
  // shapes are correct.

  const auto builder = BrushBuilder{MapFormat::Standard, WorldBounds};

  SECTION("Boulder")
  {
    const auto boulderPoints = RockFormationPoints{
      {
        {-4, 17, 31},
        {-17, -1, 32},
        {6, -16, 31},
        {19, 14, 11},
        {-23, 0, 19},
        {11, -13, 13},
        {-23, 22, 13},
        {-17, -15, -10},
        {29, 5, 17},
        {-32, 16, 1},
        {5, -32, -19},
        {7, 32, -18},
        {-20, -1, -24},
        {32, -2, -23},
        {-10, 21, -25},
        {5, -8, -32},
        {3, 4, -32},
        {-21, 3, -32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, Bounds, RockType::Boulder);
    const auto expectedBrushes = buildBrushes(builder, boulderPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Shelf")
  {
    const auto shelfPoints = RockFormationPoints{
      {
        {32, 0, -32},
        {19, 26, -32},
        {-9, 32, -32},
        {-32, 14, -32},
        {-32, -14, -32},
        {-9, -32, -32},
        {19, -26, -32},
        {26, -2, 27},
        {17, 16, 32},
        {-3, 21, 26},
        {-19, 8, 30},
        {-19, -12, 29},
        {-3, -24, 27},
        {17, -20, 30},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, Bounds, RockType::Shelf);
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
        {27, 0, -21},
        {18, 23, -20},
        {-3, 32, -20},
        {-24, 22, -21},
        {-32, 0, -20},
        {-23, -23, -20},
        {-2, -32, -21},
        {19, -22, -21},
      },
      {
        {29, 5, -22},
        {17, 24, -23},
        {-5, 29, -23},
        {-24, 17, -23},
        {-29, -4, -22},
        {-17, -23, -22},
        {4, -28, -22},
        {23, -16, -22},
        {24, 5, -10},
        {14, 23, -10},
        {-6, 28, -10},
        {-24, 17, -10},
        {-29, -3, -10},
        {-18, -21, -10},
        {2, -26, -10},
        {20, -15, -10},
      },
      {
        {27, 3, -12},
        {19, 21, -12},
        {0, 28, -12},
        {-18, 20, -12},
        {-26, 1, -11},
        {-17, -17, -12},
        {2, -24, -12},
        {20, -16, -12},
        {26, 2, 0},
        {18, 20, 1},
        {2, 27, 1},
        {-13, 19, 0},
        {-19, 0, 0},
        {-12, -17, 0},
        {4, -24, 0},
        {20, -16, 1},
      },
      {
        {26, 4, -2},
        {17, 19, -1},
        {-1, 24, -1},
        {-17, 15, -1},
        {-22, -2, -1},
        {-13, -17, -1},
        {5, -22, -2},
        {21, -13, -2},
        {20, 2, 10},
        {13, 16, 11},
        {-3, 20, 11},
        {-17, 12, 11},
        {-21, -3, 10},
        {-13, -18, 11},
        {2, -22, 11},
        {16, -14, 11},
      },
      {
        {24, 2, 9},
        {16, 16, 9},
        {0, 20, 9},
        {-14, 12, 10},
        {-18, -3, 9},
        {-10, -17, 9},
        {6, -21, 9},
        {20, -13, 9},
        {23, 3, 21},
        {16, 16, 21},
        {1, 20, 21},
        {-11, 12, 21},
        {-15, -2, 21},
        {-8, -14, 21},
        {6, -18, 21},
        {19, -11, 21},
      },
      {
        {21, 2, 19},
        {15, 14, 20},
        {1, 19, 19},
        {-11, 12, 20},
        {-16, -1, 20},
        {-9, -13, 20},
        {4, -17, 20},
        {17, -11, 20},
        {21, 0, 32},
        {15, 12, 32},
        {4, 17, 31},
        {-7, 10, 32},
        {-10, -3, 31},
        {-5, -16, 31},
        {7, -20, 32},
        {17, -14, 31},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, Bounds, RockType::Strata);
    const auto expectedBrushes = buildBrushes(builder, strataPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Strata detail adds more, thinner layers")
  {
    const auto low = makeRockFormation(Bounds, RockType::Strata, 0.0, Seed);
    const auto high = makeRockFormation(Bounds, RockType::Strata, 1.0, Seed);

    CHECK(high.size() > low.size());
  }

  SECTION("Strata layers stay flush regardless of detail")
  {
    const auto detail = GENERATE(0.0, 0.5, 1.0);

    CAPTURE(detail);

    const auto formation = makeRockFormation(Bounds, RockType::Strata, detail, Seed);

    // Each layer consists of its bottom ring followed by its top ring. The rings are
    // uneven, so the lowest point of a top ring must not be below the highest point of
    // the bottom ring of the next layer.
    const auto sides = formation.front().size() / 2u;
    for (size_t i = 0; i + 1 < formation.size(); ++i)
    {
      const auto minTopZ = std::ranges::min(
        formation[i] | std::views::drop(sides)
        | std::views::transform([](const auto& point) { return point.z(); }));
      const auto maxBottomZ = std::ranges::max(
        formation[i + 1] | std::views::take(sides)
        | std::views::transform([](const auto& point) { return point.z(); }));
      CHECK(minTopZ >= maxBottomZ);
    }
  }

  SECTION("Strata layers are uneven, but the base is flat")
  {
    // The layers must be tall enough for the jitter to survive the rounding.
    const auto detail = GENERATE(0.0, 0.5, 1.0);
    const auto tallBounds = vm::bbox3d{{-32, -32, -256}, {32, 32, 256}};

    CAPTURE(detail);

    const auto formation = makeRockFormation(tallBounds, RockType::Strata, detail, Seed);

    // Each layer consists of its bottom ring followed by its top ring.
    const auto sides = formation.front().size() / 2u;

    CHECK(isFlat(formation.front() | std::views::take(sides)));
    CHECK(!isFlat(formation.front() | std::views::drop(sides)));
    for (const auto& layer : formation | std::views::drop(1))
    {
      CHECK(!isFlat(layer | std::views::take(sides)));
      CHECK(!isFlat(layer | std::views::drop(sides)));
    }
  }

  SECTION("Crag")
  {
    const auto cragPoints = RockFormationPoints{
      {
        {-4, 12, 31},
        {-10, 4, 32},
        {0, -4, 31},
        {9, 13, 11},
        {-15, 4, 19},
        {4, -5, 13},
        {-15, 16, 13},
        {-17, -10, -10},
        {14, 7, 17},
        {-27, 15, 1},
        {-1, -22, -19},
        {1, 28, -18},
        {-18, 1, -24},
        {19, 0, -23},
        {-11, 18, -25},
        {3, -8, -32},
        {0, 5, -32},
        {-21, 4, -32},
      },
      {
        {25, 1, -17},
        {-3, 17, -21},
        {20, -14, -21},
        {26, 20, -27},
        {-4, 7, -26},
        {32, -6, -27},
        {13, 21, -32},
        {13, -14, -32},
        {28, 24, -31},
        {3, 9, -32},
        {20, -2, -32},
        {15, 9, -32},
      },
      {
        {-20, 20, -14},
        {-32, 32, -19},
        {-22, 9, -19},
        {-9, 29, -21},
        {-29, 20, -18},
        {-13, 4, -23},
        {-22, 31, -22},
        {-28, 11, -30},
        {-12, 22, -28},
        {-30, 25, -32},
        {-16, 9, -32},
        {-23, 27, -32},
      },
      {
        {5, -20, 4},
        {-16, -7, -5},
        {-1, -26, -9},
        {1, -12, -8},
        {-17, -17, -5},
        {8, -28, -10},
        {-5, 1, -16},
        {-15, -32, -28},
        {2, -15, -30},
        {-20, -16, -32},
        {-8, -26, -32},
        {-1, -14, -32},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, Bounds, RockType::Crag);
    const auto expectedBrushes = buildBrushes(builder, cragPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Crystal")
  {
    const auto crystalPoints = RockFormationPoints{
      {
        {-17, 7, -32},
        {-17, -14, -32},
        {-1, -25, -32},
        {17, -16, -32},
        {18, 5, -32},
        {1, 16, -32},
        {-14, 7, 11},
        {-15, -14, 11},
        {2, -25, 10},
        {19, -16, 10},
        {20, 5, 10},
        {3, 16, 10},
        {3, -4, 32},
      },
      {
        {13, -18, -32},
        {18, 0, -32},
        {6, 14, -32},
        {-11, 9, -32},
        {-16, -9, -32},
        {-3, -22, -32},
        {27, -15, 6},
        {32, 3, 4},
        {20, 16, 6},
        {3, 12, 9},
        {-2, -6, 10},
        {11, -20, 9},
        {21, -1, 26},
      },
      {
        {-7, 9, -32},
        {-14, -1, -28},
        {-6, -14, -25},
        {8, -16, -27},
        {15, -5, -32},
        {7, 7, -32},
        {4, 30, -12},
        {-3, 19, -5},
        {5, 7, -2},
        {19, 5, -5},
        {26, 15, -11},
        {18, 28, -15},
        {16, 25, 1},
      },
      {
        {9, 1, -32},
        {3, 9, -32},
        {-8, 6, -32},
        {-13, -4, -32},
        {-6, -11, -31},
        {5, -9, -29},
        {-3, 24, -4},
        {-9, 32, -9},
        {-20, 30, -11},
        {-25, 20, -8},
        {-18, 12, -3},
        {-8, 15, -1},
        {-19, 32, 5},
      },
      {
        {-9, 5, -32},
        {-12, -5, -32},
        {-7, -15, -32},
        {2, -14, -31},
        {5, -4, -28},
        {0, 5, -31},
        {-28, 3, -12},
        {-32, -7, -15},
        {-27, -17, -13},
        {-18, -16, -8},
        {-14, -6, -6},
        {-19, 3, -8},
        {-31, -7, -1},
      },
      {
        {-10, -13, -32},
        {2, -19, -32},
        {11, -11, -31},
        {9, 1, -28},
        {-3, 7, -28},
        {-13, -1, -32},
        {-16, -24, -19},
        {-4, -30, -18},
        {6, -22, -14},
        {3, -10, -10},
        {-9, -4, -11},
        {-19, -12, -15},
        {-9, -22, -7},
      },
      {
        {-4, 1, -26},
        {-10, -9, -29},
        {-5, -19, -32},
        {8, -18, -32},
        {14, -8, -32},
        {9, 2, -29},
        {3, -12, -9},
        {-4, -22, -12},
        {2, -32, -17},
        {14, -31, -19},
        {21, -20, -16},
        {15, -11, -11},
        {11, -27, -7},
      },
    };

    const auto actualBrushes = generateActualBrushes(builder, Bounds, RockType::Crystal);
    const auto expectedBrushes = buildBrushes(builder, crystalPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == Bounds);
  }

  SECTION("Columns")
  {
    const auto columnsPoints = RockFormationPoints{
      {
        {-17, -15, -32},
        {20, -15, -32},
        {20, 19, -32},
        {-17, 19, -32},
        {-16.0034, -10.0168, 31.7850},
        {15.2013, -10.2013, 29.4233},
        {15.1843, 19, 29.6412},
        {-16, 19, 32},
      },
      {
        {-21, -29, -32},
        {-5, -29, -32},
        {-5, -7, -32},
        {-21, -7, -32},
        {-20.0210, -27.0420, -2.6299},
        {-6.9980, -27.0020, -2.0303},
        {-7, -8, -2},
        {-20.0200, -7.9800, -2.5994},
      },
      {
        {-29, -9, -32},
        {0, -9, -32},
        {0, 11, -32},
        {-29, 11, -32},
        {-25, -8, 2},
        {-0.9910, -8.0090, 1.6956},
        {-0.9775, 9.0450, 1.2352},
        {-25.0540, 9.0270, 1.5413},
      },
      {
        {-32, 8, -32},
        {-4, 8, -32},
        {-4, 32, -32},
        {-32, 32, -32},
        {-31.0168, 8.9832, -1.5209},
        {-7.9343, 8.9836, -1.5093},
        {-8, 29, -1},
        {-31.0004, 29.0011, -1.0115},
      },
      {
        {-9, -25, -32},
        {8, -25, -32},
        {8, -9, -32},
        {-9, -9, -32},
        {-7.1029, -23.1029, -20.6174},
        {7.1831, -23.3663, -22.1978},
        {7.1293, -9, -21.5514},
        {-7, -9, -20},
      },
      {
        {1, 5, -32},
        {19, 5, -32},
        {19, 22, -32},
        {1, 22, -32},
        {1.9395, 7.8186, -5.6928},
        {17.0413, 7.9381, -4.5782},
        {17, 22, -4},
        {1.9605, 22, -5.1051},
      },
      {
        {5, -32, -32},
        {30, -32, -32},
        {30, -7, -32},
        {5, -7, -32},
        {5.9956, -31.0044, 7.8240},
        {27.1002, -31.0334, 6.6638},
        {27.0869, -9.9131, 6.8417},
        {6, -10, 8},
      },
      {
        {11, -14, -32},
        {32, -14, -32},
        {32, 6, -32},
        {11, 6, -32},
        {12, -13, -3},
        {30.0021, -13.0011, -3.0307},
        {30.0256, 4.0256, -3.3713},
        {11.9883, 4.0235, -3.3404},
      },
      {
        {9, 9, -32},
        {27, 9, -32},
        {27, 26, -32},
        {9, 26, -32},
        {10.9889, 10.9889, -0.1779},
        {26.0012, 10.9977, -0.0372},
        {26, 25, 0},
        {10.9912, 25.0044, -0.1406},
      },
    };

    const auto actualPoints = makeRockFormation(Bounds, RockType::Columns, Detail, Seed);

    CHECK_THAT(actualPoints, Catch::Matchers::RangeEquals(columnsPoints, approxEqual));

    CHECK(mergedBounds(buildBrushes(builder, actualPoints)) == Bounds);
  }

  SECTION("Basalt")
  {
    const auto basaltPoints = RockFormationPoints{
      {
        {-64, -100, -32},
        {-80, -71, -32},
        {-112, -71, -32},
        {-128, -100, -32},
        {-112, -128, -32},
        {-80, -128, -32},
        {-64, -100, 21.1590},
        {-80, -71, 22.5844},
        {-112, -71, 25.0048},
        {-128, -100, 26},
        {-112, -128, 24.5821},
        {-80, -128, 22.1616},
      },
      {
        {-64, -43, -32},
        {-80, -14, -32},
        {-112, -14, -32},
        {-128, -43, -32},
        {-112, -71, -32},
        {-80, -71, -32},
        {-64, -43, 10},
        {-80, -14, 9.3097},
        {-112, -14, 7.8364},
        {-128, -43, 7.0534},
        {-112, -71, 7.7453},
        {-80, -71, 9.2186},
      },
      {
        {-64, 14, -32},
        {-80, 43, -32},
        {-112, 43, -32},
        {-128, 14, -32},
        {-112, -14, -32},
        {-80, -14, -32},
        {-64, 14, 27.6354},
        {-80, 43, 27.0559},
        {-112, 43, 27.4620},
        {-128, 14, 28.4475},
        {-112, -14, 29},
        {-80, -14, 28.5939},
      },
      {
        {-64, 71, -32},
        {-80, 100, -32},
        {-112, 100, -32},
        {-128, 71, -32},
        {-112, 43, -32},
        {-80, 43, -32},
        {-64, 71, 17.2701},
        {-80, 100, 18},
        {-112, 100, 17.9839},
        {-128, 71, 17.2380},
        {-112, 43, 16.5335},
        {-80, 43, 16.5496},
      },
      {
        {-16, -71, -32},
        {-32, -43, -32},
        {-64, -43, -32},
        {-80, -71, -32},
        {-64, -100, -32},
        {-32, -100, -32},
        {-16, -71, 11.9728},
        {-32, -43, 14.9778},
        {-64, -43, 18.4914},
        {-80, -71, 19},
        {-64, -100, 15.9504},
        {-32, -100, 12.4367},
      },
      {
        {-16, -14, -32},
        {-32, 14, -32},
        {-64, 14, -32},
        {-80, -14, -32},
        {-64, -43, -32},
        {-32, -43, -32},
        {-16, -14, 28},
        {-32, 14, 27.9816},
        {-64, 14, 25.6302},
        {-80, -14, 23.2972},
        {-64, -43, 23.2743},
        {-32, -43, 25.6256},
      },
      {
        {-16, 43, -32},
        {-32, 71, -32},
        {-64, 71, -32},
        {-80, 43, -32},
        {-64, 14, -32},
        {-32, 14, -32},
        {-16, 43, 13.4823},
        {-32, 71, 14.5967},
        {-64, 71, 16.3555},
        {-80, 43, 17},
        {-64, 14, 15.8772},
        {-32, 14, 14.1184},
      },
      {
        {-16, 100, -32},
        {-32, 128, -32},
        {-64, 128, -32},
        {-80, 100, -32},
        {-64, 71, -32},
        {-32, 71, -32},
        {-16, 100, 20.3381},
        {-32, 128, 19.8055},
        {-64, 128, 19.8601},
        {-80, 100, 20.4474},
        {-64, 71, 21},
        {-32, 71, 20.9453},
      },
      {
        {32, -100, -32},
        {16, -71, -32},
        {-16, -71, -32},
        {-32, -100, -32},
        {-16, -128, -32},
        {16, -128, -32},
        {32, -100, 13},
        {16, -71, 12.9271},
        {-16, -71, 12.6273},
        {-32, -100, 12.4004},
        {-16, -128, 12.4759},
        {16, -128, 12.7757},
      },
      {
        {32, -43, -32},
        {16, -14, -32},
        {-16, -14, -32},
        {-32, -43, -32},
        {-16, -71, -32},
        {16, -71, -32},
        {32, -43, 10.1748},
        {16, -14, 7.1685},
        {-16, -14, 6.0723},
        {-32, -43, 7.9823},
        {-16, -71, 10.9038},
        {16, -71, 12},
      },
      {
        {32, 14, -32},
        {16, 43, -32},
        {-16, 43, -32},
        {-32, 14, -32},
        {-16, -14, -32},
        {16, -14, -32},
        {32, 14, 23.5377},
        {16, 43, 26.8193},
        {-16, 43, 31.0505},
        {-32, 14, 32},
        {-16, -14, 28.7586},
        {16, -14, 24.5274},
      },
      {
        {32, 71, -32},
        {16, 100, -32},
        {-16, 100, -32},
        {-32, 71, -32},
        {-16, 43, -32},
        {16, 43, -32},
        {32, 71, 21},
        {16, 100, 19.7733},
        {-16, 100, 16.7887},
        {-32, 71, 15.0308},
        {-16, 43, 16.2666},
        {16, 43, 19.2513},
      },
      {
        {80, -71, -32},
        {64, -43, -32},
        {32, -43, -32},
        {16, -71, -32},
        {32, -100, -32},
        {64, -100, -32},
        {80, -71, 15.8410},
        {64, -43, 17},
        {32, -43, 15.2381},
        {16, -71, 12.3172},
        {32, -100, 11.0854},
        {64, -100, 12.8473},
      },
      {
        {80, -14, -32},
        {64, 14, -32},
        {32, 14, -32},
        {16, -14, -32},
        {32, -43, -32},
        {64, -43, -32},
        {80, -14, 10.4985},
        {64, 14, 11.4255},
        {32, 14, 12},
        {16, -14, 11.6475},
        {32, -43, 10.6976},
        {64, -43, 10.1232},
      },
      {
        {80, 43, -32},
        {64, 71, -32},
        {32, 71, -32},
        {16, 43, -32},
        {32, 14, -32},
        {64, 14, -32},
        {80, 43, 14.4361},
        {64, 71, 16.0967},
        {32, 71, 20.3787},
        {16, 43, 23},
        {32, 14, 21.3565},
        {64, 14, 17.0746},
      },
      {
        {80, 100, -32},
        {64, 128, -32},
        {32, 128, -32},
        {16, 100, -32},
        {32, 71, -32},
        {64, 71, -32},
        {80, 100, 22.8152},
        {64, 128, 20.4599},
        {32, 128, 20.2097},
        {16, 100, 22.3148},
        {32, 71, 24.7498},
        {64, 71, 25},
      },
      {
        {128, -100, -32},
        {112, -71, -32},
        {80, -71, -32},
        {64, -100, -32},
        {80, -128, -32},
        {112, -128, -32},
        {128, -100, 20.7267},
        {112, -71, 21},
        {80, -71, 20.8215},
        {64, -100, 20.3697},
        {80, -128, 20.1089},
        {112, -128, 20.2874},
      },
      {
        {128, -43, -32},
        {112, -14, -32},
        {80, -14, -32},
        {64, -43, -32},
        {80, -71, -32},
        {112, -71, -32},
        {128, -43, 14},
        {112, -14, 11.8622},
        {80, -14, 7.8621},
        {64, -43, 5.9999},
        {80, -71, 8.1330},
        {112, -71, 12.1330},
      },
      {
        {128, 14, -32},
        {112, 43, -32},
        {80, 43, -32},
        {64, 14, -32},
        {80, -14, -32},
        {112, -14, -32},
        {128, 14, 12.4909},
        {112, 43, 12.0495},
        {80, 43, 13.6044},
        {64, 14, 15.6006},
        {80, -14, 16},
        {112, -14, 14.4451},
      },
      {
        {128, 71, -32},
        {112, 100, -32},
        {80, 100, -32},
        {64, 71, -32},
        {80, 43, -32},
        {112, 43, -32},
        {128, 71, 14.0922},
        {112, 100, 18.2923},
        {80, 100, 19},
        {64, 71, 15.5077},
        {80, 43, 11.4403},
        {112, 43, 10.7325},
      },
    };

    const auto actualPoints =
      makeRockFormation(FieldBounds, RockType::Basalt, Detail, Seed);

    CHECK_THAT(actualPoints, Catch::Matchers::RangeEquals(basaltPoints, approxEqual));

    CHECK(mergedBounds(buildBrushes(builder, actualPoints)) == FieldBounds);
  }

  SECTION("Cluster")
  {
    const auto clusterPoints = RockFormationPoints{
      {
        {-81, 4, 18},     {-100, -22, 18},  {-64, -42, 18},  {-45, -1, 2},
        {-111, -21, 9},   {-59, -40, 4},    {-113, 9, 5},    {-104, -45, -14},
        {-31, -14, 9},    {-128, 0, -5},    {-68, -71, -20}, {-65, 25, -19},
        {-111, -25, -26}, {-21, -28, -18},  {-94, 10, -25},  {-66, -44, -32},
        {-63, -13, -32},  {-117, -19, -27}, {-62, -21, -32}, {-88, 7, -32},
      },
      {
        {68, -88, 30},   {-5, -54, 23},    {59, -119, 21},  {72, -47, 13},
        {-8, -74, 11},   {89, -106, 9},    {38, -38, -5},   {37, -128, 3},
        {77, -38, 11},   {6, -68, 3},      {72, -122, -24}, {64, -19, -21},
        {16, -107, -11}, {101, -100, -19}, {36, -36, -23},  {61, -101, -30},
        {81, -50, -18},  {3, -86, -32},    {76, -87, -32},  {46, -54, -32},
      },
      {
        {-39, 84, 32},  {-71, 115, 22},  {-42, 54, 21},  {1, 106, 20},   {-54, 83, 26},
        {-16, 40, 18},  {-42, 111, 18},  {-70, 50, -3},  {-7, 89, 6},    {-80, 100, -6},
        {-14, 25, -12}, {-43, 128, -19}, {-88, 38, -19}, {0, 74, -21},   {-81, 113, -7},
        {-44, 23, -13}, {-22, 91, -32},  {-62, 65, -32}, {-38, 55, -32}, {-52, 98, -32},
      },
      {
        {108, 34, 23}, {47, 71, 14},  {94, 27, 10},  {98, 57, 10},   {47, 46, 14},
        {117, 18, 13}, {81, 93, 5},   {45, 0, -9},   {113, 56, -12}, {11, 57, -19},
        {75, 5, -22},  {99, 71, -6},  {41, 24, -22}, {128, 32, -20}, {53, 73, -32},
        {66, 18, -32}, {87, 74, -25}, {42, 71, -32}, {70, 14, -32},  {58, 46, -32},
      },
    };

    const auto actualBrushes =
      generateActualBrushes(builder, FieldBounds, RockType::Cluster);
    const auto expectedBrushes = buildBrushes(builder, clusterPoints);

    CHECK_THAT(actualBrushes, Catch::Matchers::UnorderedRangeEquals(expectedBrushes));

    CHECK(mergedBounds(actualBrushes) == FieldBounds);
  }

  SECTION("Detail changes the formation")
  {
    using T = std::tuple<RockType, vm::bbox3d>;

    const auto [type, bounds] = GENERATE(values<T>({
      {RockType::Boulder, Bounds},
      {RockType::Shelf, Bounds},
      {RockType::Strata, Bounds},
      {RockType::Crag, Bounds},
      {RockType::Crystal, Bounds},
      {RockType::Columns, Bounds},
      {RockType::Basalt, FieldBounds},
      {RockType::Cluster, FieldBounds},
    }));

    CAPTURE(type);

    const auto simple = makeRockFormation(bounds, type, 0.0, Seed);
    const auto complex = makeRockFormation(bounds, type, 1.0, Seed);

    CHECK(simple != complex);
    CHECK(mergedBounds(buildBrushes(builder, simple)) == bounds);
    CHECK(mergedBounds(buildBrushes(builder, complex)) == bounds);
  }

  SECTION("Detail is clamped")
  {
    const auto type = GENERATE(
      RockType::Boulder,
      RockType::Shelf,
      RockType::Strata,
      RockType::Crag,
      RockType::Crystal,
      RockType::Columns,
      RockType::Basalt,
      RockType::Cluster);

    CAPTURE(type);

    CHECK(
      makeRockFormation(FieldBounds, type, -1.0, Seed)
      == makeRockFormation(FieldBounds, type, 0.0, Seed));
    CHECK(
      makeRockFormation(FieldBounds, type, 2.0, Seed)
      == makeRockFormation(FieldBounds, type, 1.0, Seed));
  }

  SECTION("Basalt and Cluster create more pieces in larger bounds")
  {
    const auto type = GENERATE(RockType::Basalt, RockType::Cluster);
    const auto largeBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};

    CAPTURE(type);

    CHECK(
      makeRockFormation(largeBounds, type, Detail, Seed).size()
      > makeRockFormation(FieldBounds, type, Detail, Seed).size());
  }

  SECTION("Boulder, Crag and Crystal become more detailed in larger bounds")
  {
    const auto type = GENERATE(RockType::Boulder, RockType::Crag, RockType::Crystal);
    const auto largeBounds = vm::bbox3d{{-256, -256, -256}, {256, 256, 256}};

    CAPTURE(type);

    CHECK(
      numPoints(makeRockFormation(largeBounds, type, Detail, Seed))
      > numPoints(makeRockFormation(Bounds, type, Detail, Seed)));

    // The simplest and the most complex formation do not depend on the bounds.
    CHECK(
      numPoints(makeRockFormation(largeBounds, type, 0.0, Seed))
      == numPoints(makeRockFormation(Bounds, type, 0.0, Seed)));
    CHECK(
      numPoints(makeRockFormation(largeBounds, type, 1.0, Seed))
      == numPoints(makeRockFormation(Bounds, type, 1.0, Seed)));
  }

  SECTION("Boulder considers its shortest and its longest side")
  {
    const auto flatBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};
    const auto largeBounds = vm::bbox3d{{-256, -256, -256}, {256, 256, 256}};

    const auto small = makeRockFormation(Bounds, RockType::Boulder, Detail, Seed);
    const auto flat = makeRockFormation(flatBounds, RockType::Boulder, Detail, Seed);
    const auto large = makeRockFormation(largeBounds, RockType::Boulder, Detail, Seed);

    CHECK(numPoints(flat) > numPoints(small));
    CHECK(numPoints(flat) < numPoints(large));
  }

  SECTION("Strata gets more layers as its height grows")
  {
    const auto detail = GENERATE(0.0, 0.5, 1.0);
    const auto flatBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};
    const auto tallBounds = vm::bbox3d{{-32, -32, -256}, {32, 32, 256}};

    CAPTURE(detail);

    const auto small = makeRockFormation(Bounds, RockType::Strata, detail, Seed);
    const auto flat = makeRockFormation(flatBounds, RockType::Strata, detail, Seed);
    const auto tall = makeRockFormation(tallBounds, RockType::Strata, detail, Seed);

    CHECK(tall.size() > small.size());
    CHECK(flat.size() == small.size());
  }

  SECTION("Strata layers get more sides as its width and depth grow")
  {
    const auto detail = GENERATE(0.0, 0.5, 1.0);
    const auto wideBounds = vm::bbox3d{{-256, -32, -32}, {256, 32, 32}};
    const auto flatBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};
    const auto tallBounds = vm::bbox3d{{-32, -32, -256}, {32, 32, 256}};

    CAPTURE(detail);

    const auto small = makeRockFormation(Bounds, RockType::Strata, detail, Seed);
    const auto wide = makeRockFormation(wideBounds, RockType::Strata, detail, Seed);
    const auto flat = makeRockFormation(flatBounds, RockType::Strata, detail, Seed);
    const auto tall = makeRockFormation(tallBounds, RockType::Strata, detail, Seed);

    CHECK(wide.front().size() > small.front().size());
    CHECK(flat.front().size() > wide.front().size());
    CHECK(tall.front().size() == small.front().size());
  }

  SECTION("Strata limits the number of layers and sides")
  {
    const auto tinyBounds = vm::bbox3d{{-4, -4, -4}, {4, 4, 4}};
    const auto hugeBounds = vm::bbox3d{{-4096, -4096, -4096}, {4096, 4096, 4096}};

    const auto tiny = makeRockFormation(tinyBounds, RockType::Strata, 0.0, Seed);
    const auto huge = makeRockFormation(hugeBounds, RockType::Strata, 1.0, Seed);

    CHECK(tiny.size() == 2u);
    CHECK(tiny.front().size() == 10u);
    CHECK(huge.size() == 24u);
    CHECK(huge.front().size() == 64u);
  }

  SECTION("Shelf gets more sides as its width and depth grow")
  {
    const auto detail = GENERATE(0.0, 0.5, 1.0);
    const auto wideBounds = vm::bbox3d{{-256, -32, -32}, {256, 32, 32}};
    const auto flatBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};
    const auto tallBounds = vm::bbox3d{{-32, -32, -256}, {32, 32, 256}};

    CAPTURE(detail);

    const auto small = makeRockFormation(Bounds, RockType::Shelf, detail, Seed);
    const auto wide = makeRockFormation(wideBounds, RockType::Shelf, detail, Seed);
    const auto flat = makeRockFormation(flatBounds, RockType::Shelf, detail, Seed);
    const auto tall = makeRockFormation(tallBounds, RockType::Shelf, detail, Seed);

    CHECK(numPoints(wide) > numPoints(small));
    CHECK(numPoints(flat) > numPoints(wide));
    CHECK(numPoints(tall) == numPoints(small));
  }

  SECTION("Shelf limits the number of sides")
  {
    const auto tinyBounds = vm::bbox3d{{-4, -4, -4}, {4, 4, 4}};
    const auto hugeBounds = vm::bbox3d{{-4096, -4096, -32}, {4096, 4096, 32}};

    CHECK(numPoints(makeRockFormation(tinyBounds, RockType::Shelf, 0.0, Seed)) == 10u);
    CHECK(numPoints(makeRockFormation(hugeBounds, RockType::Shelf, 1.0, Seed)) == 64u);
  }

  SECTION("Shelf has an uneven top, but a flat base")
  {
    // The shelf must be tall enough for the jitter to survive the rounding.
    const auto tallBounds = vm::bbox3d{{-32, -32, -256}, {32, 32, 256}};

    const auto formation = makeRockFormation(tallBounds, RockType::Shelf, Detail, Seed);

    // The shelf consists of its bottom ring followed by its top ring.
    const auto& points = formation.front();
    const auto sides = points.size() / 2u;

    CHECK(isFlat(points | std::views::take(sides)));
    CHECK(!isFlat(points | std::views::drop(sides)));
  }

  SECTION("Crag and Crystal only consider their shortest side")
  {
    const auto type = GENERATE(RockType::Crag, RockType::Crystal);
    const auto flatBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};

    CAPTURE(type);

    CHECK(
      numPoints(makeRockFormation(flatBounds, type, Detail, Seed))
      == numPoints(makeRockFormation(Bounds, type, Detail, Seed)));
  }

  SECTION("Columns adds columns along its width and its depth independently")
  {
    const auto detail = GENERATE(0.0, 0.5, 1.0);
    const auto wideBounds = vm::bbox3d{{-256, -32, -32}, {256, 32, 32}};
    const auto deepBounds = vm::bbox3d{{-32, -256, -32}, {32, 256, 32}};
    const auto flatBounds = vm::bbox3d{{-256, -256, -32}, {256, 256, 32}};
    const auto tallBounds = vm::bbox3d{{-32, -32, -256}, {32, 32, 256}};

    CAPTURE(detail);

    const auto small = makeRockFormation(Bounds, RockType::Columns, detail, Seed);
    const auto wide = makeRockFormation(wideBounds, RockType::Columns, detail, Seed);
    const auto deep = makeRockFormation(deepBounds, RockType::Columns, detail, Seed);
    const auto flat = makeRockFormation(flatBounds, RockType::Columns, detail, Seed);
    const auto tall = makeRockFormation(tallBounds, RockType::Columns, detail, Seed);

    CHECK(wide.size() > small.size());
    CHECK(deep.size() == wide.size());

    // The number of columns is the product of the numbers along each axis.
    CHECK(flat.size() * small.size() == wide.size() * deep.size());

    CHECK(tall.size() == small.size());
  }

  SECTION("Columns limits the number of columns")
  {
    const auto tinyBounds = vm::bbox3d{{-4, -4, -4}, {4, 4, 4}};
    const auto hugeBounds = vm::bbox3d{{-4096, -4096, -32}, {4096, 4096, 32}};

    CHECK(makeRockFormation(tinyBounds, RockType::Columns, 0.0, Seed).size() == 1u);
    CHECK(makeRockFormation(hugeBounds, RockType::Columns, 1.0, Seed).size() == 100u);
  }

  SECTION("Columns overlap")
  {
    const auto detail = GENERATE(0.0, 0.5, 1.0);
    const auto bounds = GENERATE(
      vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
      vm::bbox3d{{-256, -32, -32}, {256, 32, 32}},
      vm::bbox3d{{-256, -256, -32}, {256, 256, 32}});

    CAPTURE(detail, bounds);

    const auto columnBounds = makeRockFormation(bounds, RockType::Columns, detail, Seed)
                              | std::views::transform([](const auto& points) {
                                  return *vm::bbox3d::build(points);
                                })
                              | kdl::ranges::to<std::vector>();

    // The main column comes first, and every other column overlaps a column that comes
    // before it.
    for (size_t i = 1; i < columnBounds.size(); ++i)
    {
      CAPTURE(i);

      CHECK(std::ranges::any_of(
        columnBounds | std::views::take(i),
        [&](const auto& other) { return other.intersects(columnBounds[i]); }));
    }
  }

  SECTION("Columns and Basalt have flat, slightly slanted tops")
  {
    using T = std::tuple<RockType, size_t>;

    const auto [type, expectedFaceCount] = GENERATE(values<T>({
      {RockType::Columns, 6u},
      {RockType::Basalt, 8u},
    }));
    const auto bounds = GENERATE(
      vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
      vm::bbox3d{{-128, -128, -32}, {128, 128, 32}},
      vm::bbox3d{{-128, -128, -4}, {128, 128, 4}});

    CAPTURE(type, bounds);

    const auto formation = makeRockFormation(bounds, type, Detail, Seed);
    for (const auto& brush : buildBrushes(builder, formation))
    {
      // A top or a side that is not flat would be split into several faces.
      CHECK(brush.faceCount() == expectedFaceCount);

      const auto topNormal = std::ranges::max(
        brush.faces()
          | std::views::transform([](const auto& face) { return face.normal(); }),
        {},
        [](const auto& normal) { return normal.z(); });
      CHECK(topNormal != vm::vec3d{0, 0, 1});
      CHECK(topNormal.z() >= std::cos(vm::to_radians(8.0)));
    }
  }

  SECTION("Cluster boulders become more detailed in taller bounds")
  {
    const auto tallBounds = vm::bbox3d{{-128, -128, -128}, {128, 128, 128}};

    const auto flat = makeRockFormation(FieldBounds, RockType::Cluster, Detail, Seed);
    const auto tall = makeRockFormation(tallBounds, RockType::Cluster, Detail, Seed);

    REQUIRE(tall.size() == flat.size());
    CHECK(numPoints(tall) > numPoints(flat));
  }

  SECTION("Basalt limits the number of cells")
  {
    const auto bounds = GENERATE(
      vm::bbox3d{{-4096, -4096, -32}, {4096, 4096, 32}},
      vm::bbox3d{{-4096, -32, -32}, {4096, 32, 32}},
      vm::bbox3d{{-32, -4096, -32}, {32, 4096, 32}});

    CAPTURE(bounds);

    CHECK(makeRockFormation(bounds, RockType::Basalt, 1.0, Seed).size() <= 256u);
  }
}


} // namespace tb::mdl
