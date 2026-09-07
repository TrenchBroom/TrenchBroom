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

#include "kd/ranges/to.h"

#include "vm/mat_ext.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <ranges>
#include <utility>

namespace tb::mdl
{
namespace
{

struct RockProfile
{
  size_t numPoints;
  double displacement;
};

RockProfile rockProfile(const size_t resolution)
{
  constexpr auto profiles = std::array<RockProfile, 5>{{
    {8, 0.35},
    {12, 0.30},
    {18, 0.28},
    {27, 0.25},
    {42, 0.22},
  }};
  return profiles[std::min(resolution, profiles.size() - 1u)];
}

struct Range
{
  double min;
  double max;
};

struct FormRange
{
  double atZero;
  double atOne;
};

struct CountRule
{
  size_t base;
  size_t perResolution;

  size_t operator()(const size_t resolution) const
  {
    return base + perResolution * std::min(resolution, size_t{4});
  }
};

/**
 * A deterministic random stream based on std::mt19937_64. The engine itself is fully
 * specified by the standard, so its output is portable across standard libraries, but
 * the distributions in <random> are not, so we must convert its output to a double
 * ourselves rather than using e.g. std::uniform_real_distribution. Draw from named
 * locals only: the evaluation order of function arguments is unspecified.
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
  double next() { return double(m_engine() >> 11) * 0x1.0p-53; }

  /** Returns the next value in [min, max). */
  double next(const double min, const double max) { return min + next() * (max - min); }

  /** Returns the next value in the given range. */
  double next(const Range& range) { return next(range.min, range.max); }
};

using PointCloud = std::vector<vm::vec3d>;
using RockFormation = std::vector<PointCloud>;

double mix(const double min, const double max, const double amount)
{
  return min + (max - min) * amount;
}

double mix(const FormRange& range, const double form)
{
  return mix(range.atZero, range.atOne, form);
}

std::vector<vm::vec3d> makeSpherePoints(const size_t numPoints)
{
  const auto goldenAngle = vm::Cd::pi() * (3.0 - std::sqrt(5.0));

  return std::views::iota(size_t{0}, numPoints)
         | std::views::transform([&](const auto i) {
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
  auto points = makeRing(numSides, bottomRadius.x(), bottomRadius.y(), 0.0, phase);
  auto top = makeRing(numSides, topRadius.x(), topRadius.y(), height, phase, topOffset);
  points.insert(std::end(points), std::begin(top), std::end(top));
  return points;
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

PointCloud scaleAndOffset(
  PointCloud points, const vm::vec3d& scale, const vm::vec3d& offset = {})
{
  for (auto& point : points)
  {
    point = vm::vec3d{
      point.x() * scale.x() + offset.x(),
      point.y() * scale.y() + offset.y(),
      point.z() * scale.z() + offset.z()};
  }
  return points;
}

PointCloud makeBoulderPoints(
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  constexpr auto Jitter = double{0.4};
  constexpr auto WarpFreqX = double{4.2};
  constexpr auto WarpAmpX = double{0.28};
  constexpr auto WarpFreqY = double{3.1};
  constexpr auto WarpAmpY = double{0.22};
  const auto profile = rockProfile(resolution);
  auto stream = RandomStream{seed};
  const auto phase = double(seed % 6283u) / 1000.0;

  auto points =
    makeSpherePoints(profile.numPoints) | std::views::transform([&](const auto& point) {
      const auto jitterX = stream.next(-1.0, 1.0);
      const auto jitterY = stream.next(-1.0, 1.0);
      const auto jitterZ = stream.next(-1.0, 1.0);
      const auto jitter = vm::vec3d{jitterX, jitterY, jitterZ};
      const auto radius = 1.0 + profile.displacement * stream.next(-1.0, 1.0);
      auto result = vm::normalize(point + Jitter * jitter) * radius;
      const auto t = result.z() * 0.5 + 0.5;
      result[0] += std::sin(t * WarpFreqX + phase) * WarpAmpX * form;
      result[1] += std::cos(t * WarpFreqY + phase) * WarpAmpY * form;
      return result;
    })
    | kdl::ranges::to<std::vector>();

  flattenBase(points, baseFlattening);
  ground(points);
  return points;
}

RockFormation makeShelf(
  const size_t resolution, const double baseFlattening, const double form, uint32_t seed)
{
  constexpr auto Sides = CountRule{5, 1};
  constexpr auto Cap = FormRange{0.45, 0.95};
  constexpr auto Skew = double{0.12};
  const auto sides = Sides(resolution);
  const auto phase = double(seed % 6283u) / 1000.0 * 0.12;
  const auto cap = mix(Cap, form);

  auto points =
    makeRingPrism(sides, {1.0, 0.92}, {cap, cap * 0.92}, 2.0, phase, {Skew, -Skew * 0.4});
  for (auto& point : points)
  {
    point[2] -= 1.0;
  }
  flattenBase(points, baseFlattening);
  ground(points);
  return {std::move(points)};
}

RockFormation makeStrata(
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  constexpr auto Count = CountRule{2, 1};
  constexpr auto Sides = size_t{8};
  constexpr auto Overlap = double{0.72};
  constexpr auto Taper = FormRange{0.12, 0.72};
  constexpr auto DriftBase = double{0.025};
  constexpr auto DriftPerForm = double{0.075};
  constexpr auto Phase = Range{0.0, 0.22};
  constexpr auto TopRadius = Range{0.82, 1.0};
  constexpr auto TopOffset = Range{-0.09, 0.09};
  auto stream = RandomStream{seed};
  const auto count = Count(resolution);
  auto result = RockFormation{};
  result.reserve(count);

  auto drift = vm::vec2d{};
  for (size_t i = 0; i < count; ++i)
  {
    const auto t = count == 1u ? 0.0 : double(i) / double(count - 1u);
    const auto taper = 1.0 - t * mix(Taper, form);
    const auto driftX = stream.next(-1.0, 1.0);
    const auto driftY = stream.next(-1.0, 1.0);
    drift = drift + vm::vec2d{driftX, driftY} * (DriftBase + DriftPerForm * form);
    const auto phase = stream.next(Phase);

    const auto topRadiusX = taper * stream.next(TopRadius);
    const auto topRadiusY = taper * stream.next(TopRadius);
    const auto topOffsetX = stream.next(TopOffset);
    const auto topOffsetY = stream.next(TopOffset);

    auto points = makeRingPrism(
      Sides,
      {taper, taper * 0.9},
      {topRadiusX, topRadiusY},
      1.0,
      phase,
      {topOffsetX, topOffsetY});
    flattenBase(points, baseFlattening);
    ground(points);
    result.push_back(scaleAndOffset(
      std::move(points), {1.0, 1.0, 1.0}, {drift.x(), drift.y(), double(i) * Overlap}));
  }
  return result;
}

RockFormation makeCrag(
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  constexpr auto Count = CountRule{1, 1};
  constexpr auto CoreForm = double{0.4};
  constexpr auto SatelliteForm = double{0.25};
  constexpr auto CoreScale = double{0.72};
  constexpr auto Exponent = double{1.35};
  constexpr auto Taper = FormRange{0.25, 0.78};
  constexpr auto BiteFreq = double{2.5};
  constexpr auto BitePhase = double{0.7};
  constexpr auto BiteBias = double{0.55};
  constexpr auto Bite = FormRange{0.15, 0.58};
  constexpr auto AngleJitter = Range{0.0, 0.65};
  constexpr auto Scale = Range{0.34, 0.56};
  constexpr auto Height = Range{0.3, 0.62};
  constexpr auto Reach = double{0.62};
  auto stream = RandomStream{seed};
  auto central = makeBoulderPoints(resolution, baseFlattening, form * CoreForm, seed);
  const auto maxZ = std::ranges::max(
    central | std::views::transform([](const auto& point) { return point.z(); }));
  for (auto& point : central)
  {
    const auto t = maxZ == 0.0 ? 0.0 : point.z() / maxZ;
    const auto taper = 1.0 - std::pow(t, Exponent) * mix(Taper, form);
    const auto angle = std::atan2(point.y(), point.x());
    const auto bite =
      1.0
      + (std::abs(std::sin(angle * BiteFreq + BitePhase)) - BiteBias) * mix(Bite, form);
    point[0] *= taper * bite;
    point[1] *= taper * bite;
  }

  auto result =
    RockFormation{scaleAndOffset(std::move(central), {CoreScale, CoreScale, 1.0})};
  const auto count = Count(resolution);
  for (size_t i = 0; i < count; ++i)
  {
    const auto angle =
      double(i) / double(count) * 2.0 * vm::Cd::pi() + stream.next(AngleJitter);
    const auto scale = stream.next(Scale);
    auto points = makeBoulderPoints(
      resolution > 0u ? resolution - 1u : 0u,
      baseFlattening,
      form * SatelliteForm,
      seed + uint32_t((i + 1u) * 7919u));
    const auto height = stream.next(Height);
    result.push_back(scaleAndOffset(
      std::move(points),
      {scale, scale, height},
      {std::cos(angle) * Reach, std::sin(angle) * Reach, 0.0}));
  }
  return result;
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
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  constexpr auto Count = CountRule{3, 2};
  constexpr auto Sides = size_t{6};
  constexpr auto Shoulder = double{0.42};
  constexpr auto AngleJitter = Range{-0.16, 0.16};
  constexpr auto Distance = Range{0.25, 1.0};
  constexpr auto DistanceByForm = FormRange{0.15, 1.0};
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
  const auto count = Count(resolution);
  auto result = RockFormation{};
  result.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto main = i == 0u;
    const auto secondary = i == 1u;
    const auto angle = main ? 0.0 : satelliteAngle(i, count, stream.next(AngleJitter));
    const auto distance = main ? 0.0 : stream.next(Distance) * mix(DistanceByForm, form);
    const auto scale = main ? 1.0 : secondary ? SecondaryScale : stream.next(Scale);
    const auto tilt = main        ? MainTilt
                      : secondary ? form * SecondaryTilt
                                  : form * stream.next(Tilt);
    const auto girth = main ? MainGirth : secondary ? SecondaryGirth : stream.next(Girth);

    const auto phase = stream.next(0.0, 2.0 * vm::Cd::pi());
    auto points = makeCrystalPoints(Sides, Shoulder, phase);
    for (auto& point : points)
    {
      point = {point.x() * girth, point.y() * girth, (point.z() + 1.0) * 0.5 * scale};
    }
    placeCrystal(points, angle, tilt);
    for (auto& point : points)
    {
      point[0] += std::cos(angle) * Reach * distance;
      point[1] += std::sin(angle) * Reach * distance;
    }
    flattenBase(points, baseFlattening);
    ground(points);
    result.push_back(std::move(points));
  }
  return result;
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
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  constexpr auto Count = CountRule{2, 2};
  constexpr auto Taper = double{0.16};
  constexpr auto AngleJitter = Range{-0.25, 0.25};
  constexpr auto Distance = Range{0.4, 1.0};
  constexpr auto DistanceByForm = FormRange{0.18, 1.0};
  constexpr auto Width = Range{0.12, 0.25};
  constexpr auto Height = Range{0.18, 0.72};
  constexpr auto Skew = Range{-0.14, 0.14};
  constexpr auto MainWidth = double{0.3};
  constexpr auto MainHeight = double{1.0};
  constexpr auto Reach = double{1.08};
  auto stream = RandomStream{seed};
  const auto count = Count(resolution);
  auto result = RockFormation{};
  result.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto main = i == 0u;
    const auto angle = main ? 0.0 : satelliteAngle(i, count, stream.next(AngleJitter));
    const auto distance = main ? 0.0 : stream.next(Distance) * mix(DistanceByForm, form);
    const auto width = main ? MainWidth : stream.next(Width);
    const auto depth = main ? MainWidth : stream.next(Width);
    const auto height = main ? MainHeight : stream.next(Height);

    const auto skewX = stream.next(Skew);
    const auto skewY = stream.next(Skew);

    auto points = scaleAndOffset(
      makeColumnPoints(Taper, skewX, skewY),
      {width, depth, height},
      {std::cos(angle) * Reach * distance, std::sin(angle) * Reach * distance, 0.0});
    flattenBase(points, baseFlattening);
    ground(points);
    result.push_back(std::move(points));
  }
  return result;
}

RockFormation makeBasalt(
  const vm::bbox3d& bounds,
  const size_t resolution,
  const double form,
  const uint32_t seed)
{
  constexpr auto Sides = size_t{6};
  constexpr auto GridBase = size_t{2};
  constexpr auto HeightLoss = Range{0.08, 0.68};
  auto stream = RandomStream{seed};
  const auto size = bounds.size();
  const auto aspect =
    std::clamp(std::abs(size.x()) / std::max(std::abs(size.y()), 1.0), 0.25, 4.0);
  const auto gridSize = double(resolution + GridBase);
  const auto columns =
    std::max(size_t{2}, size_t(std::ceil(gridSize * std::sqrt(aspect))));
  const auto rows = std::max(size_t{2}, size_t(std::ceil(gridSize / std::sqrt(aspect))));
  const auto rootThree = std::sqrt(3.0);

  auto result = RockFormation{};
  result.reserve(columns * rows);
  for (size_t column = 0; column < columns; ++column)
  {
    for (size_t row = 0; row < rows; ++row)
    {
      const auto tallest = column == columns / 2u && row == rows / 2u;
      const auto height = tallest ? 1.0 : 1.0 - form * stream.next(HeightLoss);
      auto points = makeRingPrism(Sides, {1.0, 1.0}, {1.0, 1.0}, height);
      result.push_back(scaleAndOffset(
        std::move(points),
        {1.0, 1.0, 1.0},
        {double(column) * 1.5,
         (double(row) + (column % 2u == 0u ? 0.0 : 0.5)) * rootThree,
         0.0}));
    }
  }
  return result;
}

RockFormation makeCluster(
  const vm::bbox3d& bounds,
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  constexpr auto CellArea = double{128.0 * 128.0};
  constexpr auto MinCount = size_t{2};
  constexpr auto MaxCount = size_t{96};
  constexpr auto Spread = FormRange{0.42, 1.0};
  constexpr auto BoulderForm = double{0.65};
  constexpr auto Variation = Range{0.88, 1.12};
  constexpr auto SizeByForm = double{0.72};
  constexpr auto Height = Range{0.28, 0.58};
  constexpr auto Fill = double{0.42};
  auto stream = RandomStream{seed};
  const auto size = bounds.size();
  const auto width = std::max(std::abs(size.x()), 1.0);
  const auto depth = std::max(std::abs(size.y()), 1.0);
  const auto area = width * depth;
  const auto count = std::clamp(size_t(std::ceil(area / CellArea)), MinCount, MaxCount);
  const auto aspect = std::clamp(width / depth, 0.125, 8.0);
  const auto columns = size_t(std::ceil(std::sqrt(double(count) * aspect)));
  const auto rows = size_t(std::ceil(double(count) / double(columns)));
  const auto cellWidth = 2.0 / double(columns);
  const auto cellDepth = 2.0 / double(rows);
  const auto spread = mix(Spread, form);

  auto result = RockFormation{};
  result.reserve(count);
  for (size_t i = 0; i < count; ++i)
  {
    const auto column = i % columns;
    const auto row = i / columns;
    const auto x = (-1.0 + (double(column) + stream.next(0.2, 0.8)) * cellWidth) * spread;
    const auto y = (-1.0 + (double(row) + stream.next(0.2, 0.8)) * cellDepth) * spread;
    const auto variation =
      mix(Variation.min, Variation.max, stream.next()) * mix(1.0, SizeByForm, form);
    auto points = makeBoulderPoints(
      resolution, baseFlattening, form * BoulderForm, seed + uint32_t(i * 7919u));
    const auto height = stream.next(Height) * variation;
    result.push_back(scaleAndOffset(
      std::move(points),
      {cellWidth * Fill * variation, cellDepth * Fill * variation, height},
      {x, y, 0.0}));
  }
  return result;
}

void fitFormation(RockFormation& formation, const vm::bbox3d& bounds)
{
  const auto pointBounds = *vm::bbox3d::build(formation | std::views::join);
  const auto transform = vm::translation_matrix(bounds.min)
                         * vm::scaling_matrix(bounds.size() / pointBounds.size())
                         * vm::translation_matrix(-pointBounds.min);
  for (auto& points : formation)
  {
    for (auto& point : points)
    {
      point = vm::round(transform * point);
    }
  }
}

} // namespace

RockFormationPoints makeRockFormation(
  const vm::bbox3d& bounds,
  const RockType type,
  const size_t resolution,
  const double baseFlattening,
  const double form,
  const uint32_t seed)
{
  const auto clampedResolution = std::min(resolution, size_t{4});
  const auto clampedForm = std::clamp(form, 0.0, 1.0);
  auto formation = [&]() {
    switch (type)
    {
    case RockType::Boulder:
      return RockFormation{
        makeBoulderPoints(clampedResolution, baseFlattening, clampedForm, seed)};
    case RockType::Shelf:
      return makeShelf(clampedResolution, baseFlattening, clampedForm, seed);
    case RockType::Strata:
      return makeStrata(clampedResolution, baseFlattening, clampedForm, seed);
    case RockType::Crag:
      return makeCrag(clampedResolution, baseFlattening, clampedForm, seed);
    case RockType::Crystal:
      return makeCrystal(clampedResolution, baseFlattening, clampedForm, seed);
    case RockType::Columns:
      return makeColumns(clampedResolution, baseFlattening, clampedForm, seed);
    case RockType::Basalt:
      return makeBasalt(bounds, clampedResolution, clampedForm, seed);
    case RockType::Cluster:
      return makeCluster(bounds, clampedResolution, baseFlattening, clampedForm, seed);
    }
    return RockFormation{};
  }();

  fitFormation(formation, bounds);
  return formation;
}

} // namespace tb::mdl
