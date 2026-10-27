/*
 Copyright (C) 2026 Jackson Palmer

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

#include "mdl/RockFormation.h"

#include "base/Macros.h"

#include "kd/ranges/cartesian_product_view.h"
#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"
#include "kd/unpack.h"

#include "vm/mat_ext.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <ranges>
#include <utility>

namespace tb::mdl
{
namespace
{

constexpr auto BaseFlattening = double{0.15};

struct Range
{
  double min;
  double max;
};

struct DetailRange
{
  double atZero;
  double atOne;
};

struct CountRange
{
  size_t atZero;
  size_t atOne;

  size_t operator()(const double detail) const
  {
    return atZero + size_t(std::lround(detail * double(atOne - atZero)));
  }
};

/**
 * A deterministic random stream based on std::mt19937_64. The engine itself is fully
 * specified by the standard, so its output is portable across standard libraries, but the
 * distributions in <random> are not, so we may get different output from the same
 * parameters on different platforms. Draw from named locals only: the evaluation order
 * of function arguments is unspecified.
 */
class RandomStream
{
private:
  std::mt19937_64 m_engine;

public:
  explicit RandomStream(const uint64_t seed)
    : m_engine{seed}
  {
  }

  /** Returns the next value in [0, 1). */
  double next()
  {
    auto dist = std::uniform_real_distribution<double>{0.0, 1.0};
    return dist(m_engine);
  }

  /** Returns the next value in [min, max). */
  double next(const double min, const double max) { return min + next() * (max - min); }

  /** Returns the next value in the given range. */
  double next(const Range& range) { return next(range.min, range.max); }
};

using PointCloud = std::vector<vm::vec3d>;
using RockFormation = std::vector<PointCloud>;

double mix(const DetailRange& range, const double detail)
{
  return vm::mix(range.atZero, range.atOne, detail);
}

/** Interpolates such that equal steps in detail scale the result by equal factors. */
double mixGeometric(const DetailRange& range, const double detail)
{
  return range.atZero * std::pow(range.atOne / range.atZero, detail);
}

/**
 * Raises the detail of formations that are larger than the reference size and lowers the
 * detail of smaller ones. A detail of 0 or 1 is not affected, so that the simplest and
 * the most complex formation remain available at any size.
 */
double detailForSize(const double detail, const double size)
{
  constexpr auto ReferenceSize = double{64.0};
  constexpr auto Exponent = double{0.5};
  return std::pow(detail, std::pow(ReferenceSize / std::max(size, 1.0), Exponent));
}

double shortestSide(const vm::vec3d& size)
{
  return std::min({size.x(), size.y(), size.z()});
}

/**
 * The size that determines the detail of a boulder. Both the shortest and the longest
 * side matter, so that a flat or an elongated boulder is more detailed than a round one
 * of the same thickness.
 */
double boulderSize(const vm::vec3d& size)
{
  const auto [shortest, longest] = std::minmax({size.x(), size.y(), size.z()});
  return std::sqrt(shortest * longest);
}

/**
 * The number of pieces to place along the given length. It grows with the square root of
 * the length so that a larger formation has both more and larger pieces.
 */
size_t countAlong(
  const double length,
  const DetailRange& density,
  const double detail,
  const size_t minCount,
  const size_t maxCount)
{
  return std::clamp(
    size_t(std::lround(mixGeometric(density, detail) * std::sqrt(length))),
    minCount,
    maxCount);
}

/** The number of sides of an outline that runs around the given size in the XY plane. */
size_t numOutlineSides(
  const vm::vec3d& size, const DetailRange& density, const double detail)
{
  constexpr auto MinSides = size_t{5};
  constexpr auto MaxSides = size_t{32};
  const auto circumference = vm::Cd::pi() * (size.x() + size.y()) / 2.0;
  return countAlong(circumference, density, detail, MinSides, MaxSides);
}

std::vector<vm::vec3d> makeSpherePoints(const size_t numPoints)
{
  const auto goldenAngle = vm::Cd::pi() * (3.0 - std::sqrt(5.0));

  return std::views::iota(0u, numPoints) | std::views::transform([&](const auto i) {
           const auto z = 1.0 - 2.0 * (double(i) + 0.5) / double(numPoints);
           const auto r = std::sqrt(1.0 - z * z);
           const auto theta = goldenAngle * double(i);
           return vm::vec3d{r * std::cos(theta), r * std::sin(theta), z};
         })
         | kdl::ranges::to<std::vector>();
}

PointCloud makeRing(
  const size_t numSides,
  const double radiusX,
  const double radiusY,
  const double z,
  const double phase = 0.0,
  const vm::vec2d& offset = {})
{
  return std::views::iota(size_t{0}, numSides) | std::views::transform([&](const auto i) {
           const auto angle = double(i) / double(numSides) * 2.0 * vm::Cd::pi() + phase;
           return vm::vec3d{
             std::cos(angle) * radiusX + offset.x(),
             std::sin(angle) * radiusY + offset.y(),
             z};
         })
         | kdl::ranges::to<std::vector>();
}

PointCloud makeRingPrism(
  const size_t numSides,
  const vm::vec2d& bottomRadius,
  const vm::vec2d& topRadius,
  const double height,
  const double phase = 0.0,
  const vm::vec2d& topOffset = {})
{
  return kdl::views::concat(
           makeRing(numSides, bottomRadius.x(), bottomRadius.y(), 0.0, phase),
           makeRing(numSides, topRadius.x(), topRadius.y(), height, phase, topOffset))
         | kdl::ranges::to<std::vector>();
}

/**
 * The angle of element `index` of `count` around a circle. Element 0 sits at the centre,
 * so the remaining elements share the full turn between them.
 */
double satelliteAngle(const size_t index, const size_t count, const double jitter)
{
  return double(index - 1u) / double(count - 1u) * 2.0 * vm::Cd::pi() + jitter;
}

void flattenBase(PointCloud& points, const double amount)
{
  if (amount <= 0.0 || points.size() < 3u)
  {
    return;
  }

  auto zValues = points
                 | std::views::transform([](const auto& point) { return point.z(); })
                 | kdl::ranges::to<std::vector>();
  std::ranges::sort(zValues);

  const auto clampedAmount = std::clamp(amount, 0.0, 1.0);
  const auto baseZ = std::max(
    zValues.front() + clampedAmount * (zValues.back() - zValues.front()), zValues[2]);
  for (auto& point : points)
  {
    point[2] = std::max(point.z(), baseZ);
  }
}

void ground(PointCloud& points)
{
  const auto minZ = std::ranges::min(
    points | std::views::transform([](const auto& point) { return point.z(); }));
  for (auto& point : points)
  {
    point[2] -= minZ;
  }
}

/** Moves each of the given points along the Z axis by a random amount in the range. */
template <std::ranges::range R>
void jitterAlongZ(R&& points, const Range& range, RandomStream& stream)
{
  for (auto& point : points)
  {
    point[2] += stream.next(range);
  }
}

PointCloud scaleAndOffset(
  const PointCloud& points, const vm::vec3d& scale, const vm::vec3d& offset = {})
{
  return points | std::views::transform([&](const auto& point) {
           return point * scale + offset;
         })
         | kdl::ranges::to<std::vector>();
}

PointCloud makeBoulderPoints(
  const double detail, const double asymmetry, const uint32_t seed)
{
  constexpr auto NumPoints = DetailRange{8.0, 42.0};
  constexpr auto Displacement = DetailRange{0.35, 0.22};
  constexpr auto Jitter = double{0.4};
  constexpr auto WarpFreqX = double{4.2};
  constexpr auto WarpAmpX = double{0.28};
  constexpr auto WarpFreqY = double{3.1};
  constexpr auto WarpAmpY = double{0.22};
  const auto numPoints = size_t(std::lround(mixGeometric(NumPoints, detail)));
  const auto displacement = mix(Displacement, detail);
  auto stream = RandomStream{seed};
  const auto phase = double(seed % 6283u) / 1000.0;

  auto points = makeSpherePoints(numPoints)
                | std::views::transform([&](const auto& point) {
                    const auto jitterX = stream.next(-1.0, 1.0);
                    const auto jitterY = stream.next(-1.0, 1.0);
                    const auto jitterZ = stream.next(-1.0, 1.0);
                    const auto jitter = vm::vec3d{jitterX, jitterY, jitterZ};
                    const auto radius = 1.0 + displacement * stream.next(-1.0, 1.0);
                    auto result = vm::normalize(point + Jitter * jitter) * radius;
                    const auto t = result.z() * 0.5 + 0.5;
                    result[0] += std::sin(t * WarpFreqX + phase) * WarpAmpX * asymmetry;
                    result[1] += std::cos(t * WarpFreqY + phase) * WarpAmpY * asymmetry;
                    return result;
                  })
                | kdl::ranges::to<std::vector>();

  flattenBase(points, BaseFlattening);
  ground(points);
  return points;
}

RockFormation makeBoulder(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  const auto sizedDetail = detailForSize(detail, boulderSize(bounds.size()));
  return {makeBoulderPoints(sizedDetail, sizedDetail, seed)};
}

RockFormation makeShelf(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto SideDensity = DetailRange{0.35, 0.7};
  constexpr auto Cap = double{0.7};
  constexpr auto Skew = double{0.12};
  constexpr auto Height = double{2.0};
  constexpr auto Jitter = Range{0.0, 0.1 * Height};
  auto stream = RandomStream{seed};
  const auto sides = numOutlineSides(bounds.size(), SideDensity, detail);
  const auto phase = double(seed % 6283u) / 1000.0 * 0.12;

  auto points = makeRingPrism(
    sides, {1.0, 0.92}, {Cap, Cap * 0.92}, Height, phase, {Skew, -Skew * 0.4});

  // The top ring comes last. Jitter it so that the top of the shelf is uneven.
  jitterAlongZ(points | std::views::drop(sides), Jitter, stream);

  return {std::move(points)};
}

RockFormation makeStrata(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto MinCount = size_t{2};
  constexpr auto MaxCount = size_t{24};
  constexpr auto LayerDensity = DetailRange{0.5, 1.125};
  constexpr auto SideDensity = DetailRange{0.4, 0.8};
  constexpr auto Overlap = double{0.05};
  constexpr auto Taper = double{0.42};
  constexpr auto Drift = DetailRange{0.025, 0.1};
  constexpr auto Phase = Range{0.0, 0.22};
  constexpr auto TopRadius = Range{0.82, 1.0};
  constexpr auto TopOffset = Range{-0.09, 0.09};
  constexpr auto BottomJitter = Range{-0.1, 0.0};
  constexpr auto TopJitter = Range{0.0, 0.1};
  auto stream = RandomStream{seed};
  const auto size = bounds.size();
  const auto sides = numOutlineSides(size, SideDensity, detail);

  // The layers are stacked along the Z axis, so their number only depends on the height.
  const auto count = countAlong(size.z(), LayerDensity, detail, MinCount, MaxCount);

  auto drift = vm::vec2d{};
  return std::views::iota(0u, count) | std::views::transform([&](const auto i) {
           const auto t = count == 1u ? 0.0 : double(i) / double(count - 1u);
           const auto taper = 1.0 - t * Taper;
           const auto driftX = stream.next(-1.0, 1.0);
           const auto driftY = stream.next(-1.0, 1.0);
           drift = drift + vm::vec2d{driftX, driftY} * mix(Drift, detail);
           const auto phase = stream.next(Phase);

           const auto topRadiusX = taper * stream.next(TopRadius);
           const auto topRadiusY = taper * stream.next(TopRadius);
           const auto topOffsetX = stream.next(TopOffset);
           const auto topOffsetY = stream.next(TopOffset);

           auto points = makeRingPrism(
             sides,
             {taper, taper * 0.9},
             {topRadiusX, topRadiusY},
             1.0,
             phase,
             {topOffsetX, topOffsetY});

           // Jitter the points along the Z axis so that the layers are uneven. A layer
           // has unit height, so the jitter and the overlap are fractions of the layer
           // height. The points only move outwards, that is, down in the bottom ring and
           // up in the top ring, so that no gaps open between adjacent layers. The bottom
           // ring comes first. It remains flat in the first layer so that the formation
           // rests on a flat base.
           if (i > 0u)
           {
             jitterAlongZ(points | std::views::take(sides), BottomJitter, stream);
           }
           jitterAlongZ(points | std::views::drop(sides), TopJitter, stream);

           return scaleAndOffset(
             points,
             {1.0, 1.0, 1.0},
             {drift.x(), drift.y(), double(i) * (1.0 - Overlap)});
         })
         | kdl::ranges::to<std::vector>();
}

RockFormation makeCrag(const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto Count = CountRange{1, 5};
  constexpr auto CoreAsymmetry = double{0.4};
  constexpr auto SatelliteAsymmetry = double{0.25};
  constexpr auto SatelliteDetailLoss = double{0.25};
  constexpr auto CoreScale = double{0.72};
  constexpr auto Exponent = double{1.35};
  constexpr auto Taper = double{0.515};
  constexpr auto BiteFreq = double{2.5};
  constexpr auto BitePhase = double{0.7};
  constexpr auto BiteBias = double{0.55};
  constexpr auto Bite = DetailRange{0.15, 0.58};
  constexpr auto AngleJitter = Range{0.0, 0.65};
  constexpr auto Scale = Range{0.34, 0.56};
  constexpr auto Height = Range{0.3, 0.62};
  constexpr auto Reach = double{0.62};
  auto stream = RandomStream{seed};
  const auto sizedDetail = detailForSize(detail, shortestSide(bounds.size()));
  auto central = makeBoulderPoints(sizedDetail, sizedDetail * CoreAsymmetry, seed);
  const auto maxZ = std::ranges::max(
    central | std::views::transform([](const auto& point) { return point.z(); }));
  for (auto& point : central)
  {
    const auto t = maxZ == 0.0 ? 0.0 : point.z() / maxZ;
    const auto taper = 1.0 - std::pow(t, Exponent) * Taper;
    const auto angle = std::atan2(point.y(), point.x());
    const auto bite = 1.0
                      + (std::abs(std::sin(angle * BiteFreq + BitePhase)) - BiteBias)
                          * mix(Bite, sizedDetail);
    point[0] *= taper * bite;
    point[1] *= taper * bite;
  }

  auto first = std::views::single(scaleAndOffset(central, {CoreScale, CoreScale, 1.0}));

  const auto count = Count(sizedDetail);
  auto rest = std::views::iota(0u, count) | std::views::transform([&](const auto i) {
                const auto angle = double(i) / double(count) * 2.0 * vm::Cd::pi()
                                   + stream.next(AngleJitter);
                const auto scale = stream.next(Scale);
                const auto height = stream.next(Height);

                const auto points = makeBoulderPoints(
                  std::max(sizedDetail - SatelliteDetailLoss, 0.0),
                  sizedDetail * SatelliteAsymmetry,
                  seed + uint32_t((i + 1u) * 7919u));

                return scaleAndOffset(
                  points,
                  {scale, scale, height},
                  {std::cos(angle) * Reach, std::sin(angle) * Reach, 0.0});
              });

  return kdl::views::concat(first, rest) | kdl::ranges::to<std::vector>();
}

PointCloud makeCrystalPoints(
  const size_t sides, const double shoulder, const double phase)
{
  auto points = makeRing(sides, 1.0, 1.0, -1.0, phase);
  auto shoulderRing = makeRing(sides, 1.0, 1.0, shoulder, phase);
  points.insert(std::end(points), std::begin(shoulderRing), std::end(shoulderRing));
  points.emplace_back(0.0, 0.0, 1.0);
  return points;
}

void placeCrystal(PointCloud& points, const double angle, const double tilt)
{
  const auto axis = vm::vec3d{
    std::cos(angle) * std::sin(tilt), std::sin(angle) * std::sin(tilt), std::cos(tilt)};
  const auto side = vm::vec3d{-std::sin(angle), std::cos(angle), 0.0};
  const auto front = vm::vec3d{
    -std::cos(angle) * std::cos(tilt), -std::sin(angle) * std::cos(tilt), std::sin(tilt)};

  for (auto& point : points)
  {
    point = side * point.x() + front * point.y() + axis * point.z();
  }
}

RockFormation makeCrystal(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto Count = CountRange{3, 11};
  constexpr auto Sides = size_t{6};
  constexpr auto Shoulder = double{0.42};
  constexpr auto AngleJitter = Range{-0.16, 0.16};
  constexpr auto Distance = Range{0.25, 1.0};
  constexpr auto DistanceByDetail = DetailRange{0.15, 1.0};
  constexpr auto Scale = Range{0.32, 0.66};
  constexpr auto Girth = Range{0.1, 0.16};
  constexpr auto Tilt = Range{0.95, 1.7};
  constexpr auto MainTilt = double{0.035};
  constexpr auto SecondaryTilt = double{0.45};
  constexpr auto MainGirth = double{0.2};
  constexpr auto SecondaryGirth = double{0.18};
  constexpr auto SecondaryScale = double{0.9};
  constexpr auto Reach = double{0.08};
  auto stream = RandomStream{seed};
  const auto sizedDetail = detailForSize(detail, shortestSide(bounds.size()));
  const auto count = Count(sizedDetail);

  return std::views::iota(0u, count) | std::views::transform([&](const auto i) {
           const auto main = i == 0u;
           const auto secondary = i == 1u;
           const auto angle =
             main ? 0.0 : satelliteAngle(i, count, stream.next(AngleJitter));
           const auto distance =
             main ? 0.0 : stream.next(Distance) * mix(DistanceByDetail, sizedDetail);
           const auto scale = main        ? 1.0
                              : secondary ? SecondaryScale
                                          : stream.next(Scale);
           const auto tilt = main        ? MainTilt
                             : secondary ? sizedDetail * SecondaryTilt
                                         : sizedDetail * stream.next(Tilt);
           const auto girth = main        ? MainGirth
                              : secondary ? SecondaryGirth
                                          : stream.next(Girth);
           const auto phase = stream.next(0.0, 2.0 * vm::Cd::pi());

           auto points = makeCrystalPoints(Sides, Shoulder, phase);
           for (auto& point : points)
           {
             point = {point.xy() * girth, (point.z() + 1.0) * 0.5 * scale};
           }
           placeCrystal(points, angle, tilt);
           for (auto& point : points)
           {
             point =
               point
               + vm::vec3d{std::cos(angle), std::sin(angle), 0.0} * Reach * distance;
           }
           flattenBase(points, BaseFlattening);
           ground(points);

           return points;
         })
         | kdl::ranges::to<std::vector>();
}

PointCloud makeColumnPoints(const double taper, const double skewX, const double skewY)
{
  auto points = PointCloud{
    {-1, -1, 0},
    {1, -1, 0},
    {1, 1, 0},
    {-1, 1, 0},
    {-1, -1, 1},
    {1, -1, 1},
    {1, 1, 1},
    {-1, 1, 1}};
  for (size_t i = 4u; i < points.size(); ++i)
  {
    points[i][0] = points[i].x() * (1.0 - taper) + skewX;
    points[i][1] = points[i].y() * (1.0 - taper) + skewY;
  }
  return points;
}

RockFormation makeColumns(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto MinCount = size_t{1};
  constexpr auto MaxCount = size_t{10};
  constexpr auto Density = DetailRange{0.25, 0.5};
  constexpr auto Taper = double{0.16};
  constexpr auto Scatter = DetailRange{0.6, 0.95};
  constexpr auto Distance = Range{0.5, 1.0};
  constexpr auto Sideways = Range{-0.4, 0.4};
  constexpr auto Width = Range{0.12, 0.25};
  constexpr auto Height = Range{0.18, 0.72};
  constexpr auto Skew = Range{-0.14, 0.14};
  constexpr auto MainWidth = double{0.3};
  constexpr auto MainHeight = double{1.0};

  struct Column
  {
    vm::vec2d position;
    vm::vec2d halfSize;
  };

  auto stream = RandomStream{seed};

  // The columns grow outwards from the main column, one per cell of a grid. The number
  // of cells along each axis only depends on the length of that axis, and the height
  // does not matter at all.
  const auto size = bounds.size();
  const auto countX = countAlong(size.x(), Density, detail, MinCount, MaxCount);
  const auto countY = countAlong(size.y(), Density, detail, MinCount, MaxCount);
  const auto mainX = countX / 2u;
  const auto mainY = countY / 2u;
  const auto scatter = mix(Scatter, detail);

  const auto distanceToMain = [](const size_t cell, const size_t mainCell) {
    return cell < mainCell ? mainCell - cell : cell - mainCell;
  };

  const auto parentOf = [](const size_t cell, const size_t mainCell) {
    return cell < mainCell ? cell + 1u : cell > mainCell ? cell - 1u : cell;
  };

  // The offset of a column from its parent along one axis, as a fraction of the offset
  // at which the two would only just touch. It is always less than 1 so that a column
  // overlaps its parent.
  const auto nextOffset = [&](const size_t cell, const size_t mainCell) {
    return cell < mainCell   ? -stream.next(Distance) * scatter
           : cell > mainCell ? stream.next(Distance) * scatter
                             : stream.next(Sideways) * scatter;
  };

  // Create the columns from the main column outwards so that the parent of a column,
  // which is its neighbour towards the main column, is always created before it.
  auto cells = kdl::views::cartesian_product(
                 std::views::iota(size_t{0}, countX), std::views::iota(size_t{0}, countY))
               | kdl::ranges::to<std::vector>();
  std::ranges::stable_sort(cells, {}, kdl::unpack([&](const auto x, const auto y) {
                             return std::max(
                               distanceToMain(x, mainX), distanceToMain(y, mainY));
                           }));

  auto columns = std::vector<Column>(countX * countY);
  auto result = RockFormation{};
  result.reserve(cells.size());

  for (const auto& [x, y] : cells)
  {
    const auto main = x == mainX && y == mainY;
    const auto width = main ? MainWidth : stream.next(Width);
    const auto depth = main ? MainWidth : stream.next(Width);
    const auto height = main ? MainHeight : stream.next(Height);
    const auto halfSize = vm::vec2d{width, depth};

    const auto skewX = stream.next(Skew);
    const auto skewY = stream.next(Skew);

    const auto offsetX = main ? 0.0 : nextOffset(x, mainX);
    const auto offsetY = main ? 0.0 : nextOffset(y, mainY);
    const auto& parent = columns[parentOf(x, mainX) * countY + parentOf(y, mainY)];
    const auto position =
      parent.position + vm::vec2d{offsetX, offsetY} * (parent.halfSize + halfSize);

    columns[x * countY + y] = {position, halfSize};
    result.push_back(scaleAndOffset(
      makeColumnPoints(Taper, skewX, skewY),
      {width, depth, height},
      {position.x(), position.y(), 0.0}));
  }

  return result;
}

RockFormation makeBasalt(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto Sides = size_t{6};
  constexpr auto CellWidth = DetailRange{128.0, 32.0};
  constexpr auto MinGridSize = size_t{2};
  constexpr auto MaxCount = size_t{256};
  constexpr auto HeightVariation = double{0.5};
  constexpr auto HeightLoss = Range{0.08, 0.68};
  auto stream = RandomStream{seed};
  const auto size = bounds.size();
  const auto radius = mixGeometric(CellWidth, detail) / 2.0;
  const auto rootThree = std::sqrt(3.0);

  // The grid below is 1.5 * columns + 0.5 radii wide and (rows + 0.5) * sqrt(3) radii
  // deep. If it has too many cells, use fewer and larger cells of the same proportions.
  const auto fitColumns =
    std::max((std::abs(size.x()) / radius - 0.5) / 1.5, double(MinGridSize));
  const auto fitRows =
    std::max(std::abs(size.y()) / (radius * rootThree) - 0.5, double(MinGridSize));
  const auto scale = std::min(std::sqrt(double(MaxCount) / (fitColumns * fitRows)), 1.0);
  const auto columns = std::clamp(
    size_t(std::lround(fitColumns * scale)), MinGridSize, MaxCount / MinGridSize);
  const auto rows =
    std::clamp(size_t(std::lround(fitRows * scale)), MinGridSize, MaxCount / columns);

  return kdl::views::cartesian_product(
           std::views::iota(0u, columns), std::views::iota(0u, rows))
         | std::views::transform(kdl::unpack([&](const auto c, const auto r) {
             const auto tallest = c == columns / 2u && r == rows / 2u;
             const auto height =
               tallest ? 1.0 : 1.0 - HeightVariation * stream.next(HeightLoss);
             const auto points = makeRingPrism(Sides, {1.0, 1.0}, {1.0, 1.0}, height);

             return scaleAndOffset(
               points,
               {1.0, 1.0, 1.0},
               {
                 double(c) * 1.5,
                 (double(r) + (c % 2u == 0u ? 0.0 : 0.5)) * rootThree,
                 0.0,
               });
           }))
         | kdl::ranges::to<std::vector>();
}

RockFormation makeCluster(
  const vm::bbox3d& bounds, const double detail, const uint32_t seed)
{
  constexpr auto CellSize = DetailRange{256.0, 64.0};
  constexpr auto MinCount = size_t{2};
  constexpr auto MaxCount = size_t{96};
  constexpr auto Spread = double{0.71};
  constexpr auto BoulderAsymmetry = double{0.65};
  constexpr auto Variation = Range{0.88, 1.12};
  constexpr auto SizeScale = double{0.86};
  constexpr auto Height = Range{0.28, 0.58};
  constexpr auto Fill = double{0.42};
  auto stream = RandomStream{seed};
  const auto size = bounds.size();
  const auto width = std::max(std::abs(size.x()), 1.0);
  const auto depth = std::max(std::abs(size.y()), 1.0);
  const auto area = width * depth;
  const auto cellSize = mixGeometric(CellSize, detail);
  const auto count =
    std::clamp(size_t(std::ceil(area / (cellSize * cellSize))), MinCount, MaxCount);
  const auto aspect = std::clamp(width / depth, 0.125, 8.0);
  const auto columns = size_t(std::ceil(std::sqrt(double(count) * aspect)));
  const auto rows = size_t(std::ceil(double(count) / double(columns)));
  const auto cellWidth = 2.0 / double(columns);
  const auto cellDepth = 2.0 / double(rows);

  // A boulder is about as wide as a cell and at most as tall as the bounds.
  const auto cellSide = std::sqrt(area / double(count));
  const auto boulderDetail =
    detailForSize(detail, boulderSize({cellSide, cellSide, std::abs(size.z())}));

  return std::views::iota(0u, count) | std::views::transform([&](const auto i) {
           const auto c = double(i % columns);
           const auto r = double(i / columns);
           const auto x = (-1.0 + (c + stream.next(0.2, 0.8)) * cellWidth) * Spread;
           const auto y = (-1.0 + (r + stream.next(0.2, 0.8)) * cellDepth) * Spread;
           const auto variation =
             vm::mix(Variation.min, Variation.max, stream.next()) * SizeScale;
           const auto height = stream.next(Height) * variation;

           const auto points = makeBoulderPoints(
             boulderDetail, boulderDetail * BoulderAsymmetry, seed + uint32_t(i * 7919u));
           return scaleAndOffset(
             points,
             {cellWidth * Fill * variation, cellDepth * Fill * variation, height},
             {x, y, 0.0});
         })
         | kdl::ranges::to<std::vector>();
}

RockFormation fitFormation(const RockFormation& formation, const vm::bbox3d& bounds)
{
  const auto pointBounds = *vm::bbox3d::build(formation | std::views::join);
  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size() / pointBounds.size())
                         * vm::translation_matrix(-pointBounds.min);

  return formation | std::views::transform([&](const auto& points) {
           return points | std::views::transform([&](const auto point) {
                    return vm::round(transform * point);
                  })
                  | kdl::ranges::to<std::vector>();
         })
         | kdl::ranges::to<std::vector>();
}

/**
 * Slants the top of each of the given prisms by tilting its normal away from the Z axis
 * by a random angle and in a random direction. The top points of a prism make up the
 * second half of its points, in the same order as its bottom points. Each top point moves
 * along the edge that connects it to its bottom point, so that the top and the sides of
 * the prism remain flat. The highest point of a top remains in place, so that the prism
 * does not grow.
 */
void slantTops(RockFormation& prisms, const uint32_t seed)
{
  constexpr auto Tilt = Range{0.0, vm::to_radians(8.0)};
  constexpr auto MaxDrop = double{0.25};

  // The slants have a stream of their own so that they do not depend on the draws that
  // shape the formation.
  auto stream = RandomStream{seed + 7919u};

  for (auto& points : prisms)
  {
    const auto numSides = points.size() / 2u;
    const auto top = points | std::views::drop(numSides);

    // Rounding can flatten a prism entirely, which leaves nothing to slant.
    const auto height = points.back().z() - points.front().z();
    if (height <= 0.0)
    {
      continue;
    }

    // Slant a squat prism less so that its top drops by at most a fraction of its height.
    const auto extent = vm::length(vm::bbox3d::build(top)->size().xy());
    const auto maxTilt = std::atan(MaxDrop * height / extent);
    const auto tilt = std::min(stream.next(Tilt), maxTilt);
    const auto direction = stream.next(0.0, 2.0 * vm::Cd::pi());

    // Tilt the normal about the X axis, then turn it about the Z axis.
    const auto normal = vm::rotation_matrix(tilt, 0.0, direction) * vm::vec3d{0, 0, 1};

    // The slanted top passes through the top point that remains highest, and all other
    // top points move down onto it.
    const auto anchor = std::ranges::min(
      top, {}, [&](const auto& point) { return vm::dot(point, normal); });

    for (size_t i = 0; i < numSides; ++i)
    {
      auto& point = points[numSides + i];
      const auto edge = point - points[i];
      point = point - edge * (vm::dot(point - anchor, normal) / vm::dot(edge, normal));
    }
  }
}

} // namespace

RockFormationPoints makeRockFormation(
  const vm::bbox3d& bounds, const RockType type, const double detail, const uint32_t seed)
{
  const auto clampedDetail = std::clamp(detail, 0.0, 1.0);
  auto formation = [&]() {
    switch (type)
    {
    case RockType::Boulder:
      return makeBoulder(bounds, clampedDetail, seed);
    case RockType::Shelf:
      return makeShelf(bounds, clampedDetail, seed);
    case RockType::Strata:
      return makeStrata(bounds, clampedDetail, seed);
    case RockType::Crag:
      return makeCrag(bounds, clampedDetail, seed);
    case RockType::Crystal:
      return makeCrystal(bounds, clampedDetail, seed);
    case RockType::Columns:
      return makeColumns(bounds, clampedDetail, seed);
    case RockType::Basalt:
      return makeBasalt(bounds, clampedDetail, seed);
    case RockType::Cluster:
      return makeCluster(bounds, clampedDetail, seed);
      switchDefault();
    }
  }();

  auto result = fitFormation(formation, bounds);

  // The tops can only be slanted once the formation is fitted: fitting scales the axes
  // independently, which would change the angle of a slanted top, and rounding its points
  // would bend it.
  if (type == RockType::Columns || type == RockType::Basalt)
  {
    slantTops(result, seed);
  }

  return result;
}

} // namespace tb::mdl
