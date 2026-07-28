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

#include "mdl/UvAlignment.h"

#include "base/Macros.h"
#include "mdl/BrushFace.h"
#include "mdl/UvAttributes.h"

#include "kd/contracts.h"
#include "kd/range_utils.h"
#include "kd/ranges/to.h"

#include "vm/scalar.h"

#include <cmath>
#include <ostream>
#include <ranges>
#include <tuple>
#include <vector>

namespace tb::mdl
{
namespace
{

vm::vec2f toAxis(const UvAxis uvAxis)
{
  switch (uvAxis)
  {
  case UvAxis::u:
    return vm::vec2f{1, 0};
  case UvAxis::v:
    return vm::vec2f{0, 1};
    switchDefault();
  }
}

template <typename T>
T findNextScaleFactor(const T f, const UvPolicy uvPolicy)
{
  if (vm::is_zero(f, vm::Cf::almost_zero()))
  {
    return T(1);
  }

  const auto sign = f < T(0) ? T(-1) : T(1);
  const auto magnitude = vm::abs(f);

  const auto nextMagnitude = [&]() -> T {
    switch (uvPolicy)
    {
    case UvPolicy::best:
      return magnitude >= T(1) ? vm::round(magnitude)
                               : T(1) / vm::round(T(1) / magnitude);
    case UvPolicy::next:
      return magnitude >= T(1) ? vm::next_integer(magnitude)
                               : T(1) / vm::prev_integer(T(1) / magnitude);
    case UvPolicy::prev:
      if (vm::is_equal(magnitude, T(1), vm::Cf::almost_zero()))
      {
        return T(0.5);
      }
      return magnitude > T(1) ? vm::prev_integer(magnitude)
                              : T(1) / vm::next_integer(T(1) / magnitude);
      switchDefault();
    }
  }();

  return sign * nextMagnitude;
}

auto scaleFactorToFit(
  const BrushFace& brushFace,
  const UvAxis uvAxis,
  const UvPolicy uvPolicy,
  const UvFitMode uvFitMode)
{
  const auto axis = toAxis(uvAxis);

  const auto distances =
    brushFace.vertices()
    | std::views::transform(
      [toUv = brushFace.toUvCoordSystemMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1})](
        const auto* vertex) { return vm::vec2f{toUv * vertex->position()}; })
    | std::views::transform([&](const auto& v) { return vm::dot(v, axis); })
    | kdl::ranges::to<std::vector>();

  const auto [iMin, iMax] = std::ranges::minmax_element(distances);
  contract_assert(iMin != iMax);

  const auto faceLength = *iMax - *iMin;
  const auto textureLength = vm::dot(brushFace.textureSize(), axis);

  const auto currentScale = vm::dot(brushFace.uvAttributes().scale, axis);

  // If the texture is larger than the face, fitToFace scales it so that the entire
  // texture is visible on the face instead of snapping to a 1/nth subdivision (which
  // trimSheet keeps for trim sheet textures).
  if (uvFitMode == UvFitMode::fitToFace && textureLength > faceLength)
  {
    const auto sign = currentScale < 0.0f ? -1.0f : 1.0f;
    return sign * faceLength / textureLength;
  }

  const auto currentFactor = currentScale * textureLength / faceLength;

  const auto nextFactor = findNextScaleFactor(currentFactor, uvPolicy);
  return nextFactor * faceLength / textureLength;
}

std::tuple<vm::vec2d, bool> findEdgeToAlignTo(
  const BrushFace& brushFace, const UvPolicy uvPolicy)
{
  constexpr auto uAxis = vm::vec2d{1, 0};

  const auto dot = [&](const auto& v) { return vm::dot(v, uAxis); };

  const auto edgeVecs =
    brushFace.geometry()->boundary()
    | std::views::transform(
      [toUv = brushFace.toUvCoordSystemMatrix(
         brushFace.uvAttributes().offset, vm::vec2f{1, 1})](const auto* halfEdge) {
        const auto start = vm::vec2d{toUv * halfEdge->origin()->position()};
        const auto end = vm::vec2d{toUv * halfEdge->next()->origin()->position()};
        return vm::normalize(end - start);
      })
    | kdl::ranges::to<std::vector>();

  // find the edge vec that is closest to the U axis
  const auto iBestMatch = std::ranges::max_element(edgeVecs, std::less<double>{}, dot);
  contract_assert(iBestMatch != std::ranges::end(edgeVecs));

  const auto isExactMatch = vm::is_equal(dot(*iBestMatch), 1.0, vm::Cd::angle_epsilon());

  const auto edgeToAlignTo = [&] {
    switch (uvPolicy)
    {
    case UvPolicy::best:
      return *iBestMatch;
    case UvPolicy::next:
      return isExactMatch ? *kdl::succ(edgeVecs, iBestMatch) : *iBestMatch;
    case UvPolicy::prev:
      return isExactMatch ? *kdl::pred(edgeVecs, iBestMatch) : *iBestMatch;
      switchDefault();
    }
  }();

  return {edgeToAlignTo, isExactMatch};
}

float normalizeAngle(const float angleInDegrees)
{
  return vm::correct(vm::mod(angleInDegrees, 360.0f));
}

float normalizeOffset(const float offset, const float length)
{
  const auto normalizedOffset = vm::correct(vm::mod(offset, length));
  return normalizedOffset < 0.0f ? normalizedOffset + length : normalizedOffset;
}

auto toFactor(const UvSign uvSign)
{
  switch (uvSign)
  {
  case UvSign::plus:
    return +1.0f;
  case UvSign::minus:
    return -1.0f;
    switchDefault();
  }
}

auto makeVertexToUvAxisTransform(
  const BrushFace& brushFace, const UvAxis uvAxis, const UvSign uvSign)
{
  const auto axis = toAxis(uvAxis);
  const auto dirFactor = toFactor(uvSign);

  const auto toUv =
    brushFace.toUvCoordSystemMatrix(vm::vec2f{0, 0}, brushFace.uvAttributes().scale);

  return [=](const auto& vertex) {
    const auto uvCoords = vm::vec2f{toUv * vertex->position()};
    return vm::dot(uvCoords, dirFactor * axis);
  };
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const UvPolicy rhs)
{
  switch (rhs)
  {
  case UvPolicy::best:
    lhs << "best";
    break;
  case UvPolicy::next:
    lhs << "next";
    break;
  case UvPolicy::prev:
    lhs << "previous";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const UvFitMode rhs)
{
  switch (rhs)
  {
  case UvFitMode::fitToFace:
    lhs << "fitToFace";
    break;
  case UvFitMode::trimSheet:
    lhs << "trimSheet";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const UvAxis rhs)
{
  switch (rhs)
  {
  case UvAxis::u:
    lhs << "u";
    break;
  case UvAxis::v:
    lhs << "v";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const UvSign rhs)
{
  switch (rhs)
  {
  case UvSign::plus:
    lhs << "plus";
    break;
  case UvSign::minus:
    lhs << "minus";
    break;
  }
  return lhs;
}

vm::vec3d anchorVertex(
  const BrushFace& brushFace, const UvAxis uvAxis, const UvSign preferredSign)
{
  auto vertices = brushFace.vertices();
  const auto findMaxVertex = [&](const auto uvSign) {
    const auto iMax = std::ranges::max_element(
      vertices,
      std::less<float>{},
      makeVertexToUvAxisTransform(brushFace, uvAxis, uvSign));
    contract_assert(iMax != std::ranges::end(vertices));

    return (*iMax)->position();
  };

  const auto otherSign = static_cast<UvSign>(1 - static_cast<int>(preferredSign));

  for (const auto uvSign : {preferredSign, otherSign})
  {
    if (isJustified(brushFace, uvAxis, uvSign))
    {
      return findMaxVertex(uvSign);
    }
  }

  return findMaxVertex(preferredSign);
}

bool isAligned(const BrushFace& brushFace)
{
  const auto [edgeToAlignTo, isExactMatch] = findEdgeToAlignTo(brushFace, UvPolicy::best);
  return isExactMatch;
}

bool isJustified(const BrushFace& brushFace, const UvAxis uvAxis, const UvSign uvSign)
{
  const auto axis = toAxis(uvAxis);
  const auto dirFactor = toFactor(uvSign);

  auto distances =
    brushFace.vertices()
    | std::views::transform(makeVertexToUvAxisTransform(brushFace, uvAxis, uvSign));

  const auto [iMin, iMax] = std::ranges::minmax_element(distances);
  contract_assert(iMin != iMax);

  const auto textureLength = vm::dot(brushFace.textureSize(), axis);
  const auto faceLength = *iMax - *iMin;
  const auto numSubDivisions =
    vm::is_equal(std::fmod(textureLength, faceLength), 0.0f, vm::Cf::almost_zero())
      ? size_t(std::round(textureLength / faceLength))
      : 1u;

  const auto subDivisionLength = textureLength / float(numSubDivisions);
  const auto maxOffset = -dirFactor * *iMax;
  const auto currentOffset =
    normalizeOffset(vm::dot(brushFace.uvAttributes().offset, axis), textureLength);

  return std::ranges::any_of(std::views::iota(0u, numSubDivisions), [&](const auto div) {
    const auto potentialOffset = float(div) * subDivisionLength + maxOffset;
    return vm::is_equal(
      normalizeOffset(potentialOffset, textureLength),
      currentOffset,
      vm::Cf::almost_zero());
  });
}

bool isFitted(const BrushFace& brushFace, UvAxis uvAxis)
{
  const auto value =
    scaleFactorToFit(brushFace, uvAxis, UvPolicy::best, UvFitMode::fitToFace);

  switch (uvAxis)
  {
  case UvAxis::u:
    return vm::is_equal(brushFace.uvAttributes().scale.x(), value, vm::Cf::almost_zero());
  case UvAxis::v:
    return vm::is_equal(brushFace.uvAttributes().scale.y(), value, vm::Cf::almost_zero());
    switchDefault();
  }
}

UpdateBrushFaceAttributes align(const BrushFace& brushFace, const UvPolicy uvPolicy)
{
  const auto [edgeToAlignTo, isExactMatch] = findEdgeToAlignTo(brushFace, uvPolicy);

  const auto angleInDegrees =
    brushFace.measureUvAngle(vm::vec2f{0, 0}, vm::vec2f{edgeToAlignTo});

  return {
    .rotation = SetValue{normalizeAngle(angleInDegrees)},
  };
}

UpdateBrushFaceAttributes justify(
  const BrushFace& brushFace,
  const UvAxis uvAxis,
  const UvSign uvSign,
  const UvPolicy uvPolicy)
{
  const auto axis = toAxis(uvAxis);
  const auto dirFactor = toFactor(uvSign);

  auto distances =
    brushFace.vertices()
    | std::views::transform(makeVertexToUvAxisTransform(brushFace, uvAxis, uvSign));

  const auto [iMin, iMax] = std::ranges::minmax_element(distances);
  contract_assert(iMin != iMax);

  // if the texture length is a multiple of the face length, we can cycle through
  // different offsets corresponding to the integer divisor
  const auto textureLength = vm::dot(brushFace.textureSize(), axis);
  const auto faceLength = *iMax - *iMin;
  const auto numSubDivisions =
    vm::is_equal(std::fmod(textureLength, faceLength), 0.0f, vm::Cf::almost_zero())
      ? size_t(std::round(textureLength / faceLength))
      : 1u;

  const auto subDivisionLength = textureLength / float(numSubDivisions);
  const auto maxOffset = -dirFactor * *iMax;
  const auto potentialOffsets =
    std::views::iota(0u, numSubDivisions) | std::views::transform([&](const auto div) {
      const auto potentialOffset = float(div) * subDivisionLength + maxOffset;
      return normalizeOffset(potentialOffset, textureLength);
    })
    | kdl::ranges::to<std::vector>();

  const auto currentOffset =
    normalizeOffset(vm::dot(brushFace.uvAttributes().offset, axis), textureLength);
  const auto iBestMatch = std::ranges::min_element(
    potentialOffsets, std::less<float>{}, [&](const auto& potentialOffset) {
      return std::abs(potentialOffset - currentOffset);
    });

  const auto isExactMatch =
    vm::is_equal(*iBestMatch, currentOffset, vm::Cf::almost_zero());
  const auto newValue = [&] {
    switch (uvPolicy)
    {
    case UvPolicy::best:
      return potentialOffsets.front();
    case UvPolicy::next:
      return isExactMatch ? *kdl::succ(potentialOffsets, iBestMatch)
                          : potentialOffsets.front();
    case UvPolicy::prev:
      return isExactMatch ? *kdl::pred(potentialOffsets, iBestMatch)
                          : potentialOffsets.front();
      switchDefault();
    }
  }();

  switch (uvAxis)
  {
  case UvAxis::u:
    return {
      .xOffset = SetValue{newValue},
    };
  case UvAxis::v:
    return {
      .yOffset = SetValue{newValue},
    };
    switchDefault();
  }
}

UpdateBrushFaceAttributes fit(
  const BrushFace& brushFace,
  const UvAxis uvAxis,
  const UvPolicy uvPolicy,
  const UvFitMode uvFitMode)
{
  const auto value = scaleFactorToFit(brushFace, uvAxis, uvPolicy, uvFitMode);

  switch (uvAxis)
  {
  case UvAxis::u:
    return {
      .xScale = SetValue{value},
    };
  case UvAxis::v:
    return {
      .yScale = SetValue{value},
    };
    switchDefault();
  }
}

} // namespace tb::mdl
