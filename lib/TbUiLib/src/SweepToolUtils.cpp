/*
 Copyright (C) 2026 Jackson Palmer
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

#include "ui/SweepToolUtils.h"

#include "Logger.h"
#include "gl/MaterialManager.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h" // IWYU pragma: keep
#include "mdl/BrushFaceAttributes.h"
#include "mdl/BrushNode.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "render/BrushRenderer.h"
#include "ui/MapDocument.h"

#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/result.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/polygon_io.h" // IWYU pragma: keep
#include "vm/quat.h"
#include "vm/quat_io.h" // IWYU pragma: keep
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <array>
#include <cmath>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>

namespace tb::ui
{

namespace
{

std::optional<vm::vec3d> arcPivot(
  const SweepSource& source, const SweepTransform& transform, const vm::quatd& rotation)
{
  const auto theta = rotation.angle();
  if (std::abs(theta) < vm::Cd::almost_zero())
  {
    return std::nullopt;
  }

  const auto axis = vm::normalize(rotation.axis());
  const auto c0 = source.center;
  const auto c1 = transform.destinationCenter(source);

  // work in the plane perpendicular to the axis; the rise becomes a helix lift
  const auto rise = vm::dot(c1 - c0, axis);
  const auto u1 = c1 - axis * rise;
  const auto chord = u1 - c0;
  const auto chordLength = vm::length(chord);
  if (chordLength < vm::Cd::almost_zero())
  {
    // no lateral travel, so there is no circle to fit
    return std::nullopt;
  }

  auto inPlanePerp = vm::cross(axis, chord);
  const auto perpLength = vm::length(inPlanePerp);
  if (perpLength < vm::Cd::almost_zero())
  {
    return std::nullopt;
  }
  inPlanePerp = inPlanePerp / perpLength;

  const auto mid = (c0 + u1) * 0.5;
  const auto distanceMidToCenter = (chordLength * 0.5) / std::tan(theta * 0.5);

  // pick the candidate center whose rotation by theta maps c0 onto u1
  const auto turn = vm::quatd{axis, theta};
  auto bestPivot = mid;
  auto bestError = std::numeric_limits<double>::max();
  for (const auto sign : {1.0, -1.0})
  {
    const auto pivot = mid + inPlanePerp * (distanceMidToCenter * sign);
    const auto mapped = pivot + turn * (c0 - pivot);
    if (const auto error = vm::squared_length(mapped - u1); error < bestError)
    {
      bestError = error;
      bestPivot = pivot;
    }
  }
  return bestPivot;
}

vm::vec3d sBendOffset(
  const SweepSource& source,
  const SweepTransform& transform,
  const double t,
  const vm::quatd& rotation)
{
  // cubic Hermite with end tangents along the source normal and the rotated cap normal
  const auto chordLength = vm::length(transform.translation);
  const auto startTangent = source.normal * chordLength;
  const auto endTangent = (rotation * source.normal) * chordLength;

  const auto t2 = t * t;
  const auto t3 = t2 * t;
  return startTangent * (t3 - 2.0 * t2 + t)
         + transform.translation * (-2.0 * t3 + 3.0 * t2) + endTangent * (t3 - t2);
}

vm::mat4x4d stationTransformWithPivot(
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters,
  const double t,
  const vm::quatd& rotationFull,
  const std::optional<vm::vec3d>& pivot)
{
  const auto c0 = source.center;
  const auto angle = rotationFull.angle();
  const auto hasRotation = std::abs(angle) > vm::Cd::almost_zero();
  // axis() is degenerate at ~0 rotation, so pick a safe default
  const auto axis = hasRotation ? vm::normalize(rotationFull.axis()) : vm::vec3d{0, 0, 1};
  const auto rotation = vm::quatd{axis, angle * t};
  const auto factors = vm::vec3d{1, 1, 1} + (transform.scale - vm::vec3d{1, 1, 1}) * t;

  if (parameters.pathMode == SweepPathMode::Arc && pivot)
  {
    const auto rise = vm::dot(transform.translation, axis);
    auto m = vm::translation_matrix(-c0);
    m = vm::scaling_matrix(factors) * m;
    m = vm::translation_matrix(c0) * m; // scale about c0
    m = vm::translation_matrix(-*pivot) * m;
    m = vm::rotation_matrix(rotation) * m;                      // revolve about the pivot
    m = vm::translation_matrix(*pivot + axis * (rise * t)) * m; // helix lift
    return m;
  }
  // no usable pivot, fall through to the straight path

  const auto sBend = parameters.pathMode == SweepPathMode::SBend
                     && !vm::is_zero(source.normal, vm::Cd::almost_zero());
  auto m = vm::translation_matrix(-c0);
  m = vm::scaling_matrix(factors) * m;
  m = vm::rotation_matrix(rotation) * m;
  m = vm::translation_matrix(c0) * m;
  m =
    vm::translation_matrix(
      sBend ? sBendOffset(source, transform, t, rotationFull) : transform.translation * t)
    * m;
  return m;
}

std::optional<vm::vec3d> stationArcPivot(
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters,
  const vm::quatd& rotationFull)
{
  return parameters.pathMode == SweepPathMode::Arc
           ? arcPivot(source, transform, rotationFull)
           : std::nullopt;
}

std::vector<vm::mat4x4d> computeSweepPowers(
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters,
  const vm::quatd& rotation,
  const std::optional<vm::vec3d>& pivot)
{
  // iteration r continues from the previous cap, i.e. applies the cap transform r times,
  // so powers[r] is the exclusive prefix product capTransform^r
  const auto capTransform =
    stationTransformWithPivot(source, transform, parameters, 1.0, rotation, pivot);
  const auto capTransforms = std::views::iota(size_t{0}, parameters.iterations)
                             | std::views::transform([&](auto) { return capTransform; });
  auto powers = std::vector<vm::mat4x4d>{};
  std::exclusive_scan(
    capTransforms.begin(),
    capTransforms.end(),
    std::back_inserter(powers),
    vm::mat4x4d::identity(),
    std::multiplies<>());
  return powers;
}

std::vector<std::vector<vm::mat4x4d>> computeTransformTable(
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters)
{
  const auto rotation = transform.effectiveRotation();
  const auto pivot = stationArcPivot(source, transform, parameters, rotation);
  const auto powers = computeSweepPowers(source, transform, parameters, rotation, pivot);
  contract_assert(powers.size() == parameters.iterations);

  return std::views::iota(0u, parameters.iterations)
         | std::views::transform([&](const auto r) {
             return std::views::iota(0u, parameters.segments + 1u)
                    | std::views::transform([&](const auto s) {
                        const auto t = double(s) / double(parameters.segments);
                        return powers[r]
                               * stationTransformWithPivot(
                                 source, transform, parameters, t, rotation, pivot);
                      })
                    | kdl::ranges::to<std::vector>();
           })
         | kdl::ranges::to<std::vector>();
}

void setMaterial(mdl::Brush& brush, gl::Material* material)
{
  for (auto& brushFace : brush.faces())
  {
    brushFace.setMaterial(material);
  }
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const SweepAlignment rhs)
{
  switch (rhs)
  {
  case SweepAlignment::Integer:
    lhs << "Integer";
    break;
  case SweepAlignment::Free:
    lhs << "Free";
    break;
  }

  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const SweepPathMode rhs)
{
  switch (rhs)
  {
  case SweepPathMode::Arc:
    lhs << "Arc";
    break;
  case SweepPathMode::Straight:
    lhs << "Straight";
    break;
  case SweepPathMode::SBend:
    lhs << "S-bend";
    break;
  }

  return lhs;
}

kdl_reflect_impl(SweepTransform);
kdl_reflect_impl(SweepParameters);
kdl_reflect_impl(SweepFace);
kdl_reflect_impl(SweepSource);

vm::vec3d SweepTransform::destinationCenter(const SweepSource& source) const
{
  return source.center + translation;
}

vm::quatd SweepTransform::effectiveRotation() const
{
  // quat::angle() runs 0..2pi, so a ring dragged 330 degrees would sweep all 330; above a
  // half-turn, use the smaller turn about the opposite axis instead
  const auto theta = rotation.angle();
  if (theta <= vm::Cd::pi() + vm::Cd::almost_zero())
  {
    return rotation;
  }

  // a full turn is the identity, and axis() is degenerate here
  const auto shortAngle = vm::Cd::two_pi() - theta;
  return shortAngle < vm::Cd::almost_zero()
           ? vm::quatd{vm::vec3d{0, 0, 1}, 0.0}
           : vm::quatd{-vm::normalize(rotation.axis()), shortAngle};
}

bool SweepTransform::isNoOp() const
{
  return vm::is_zero(translation, vm::Cd::almost_zero())
         && effectiveRotation().angle() < vm::Cd::almost_zero()
         && vm::is_equal(scale, vm::vec3d{1, 1, 1}, vm::Cd::almost_zero());
}

vm::mat4x4d stationTransform(
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters,
  const double t,
  const vm::quatd& rotationFull)
{
  return stationTransformWithPivot(
    source,
    transform,
    parameters,
    t,
    rotationFull,
    stationArcPivot(source, transform, parameters, rotationFull));
}

std::map<mdl::Node*, std::vector<std::unique_ptr<mdl::BrushNode>>> generateSweepBrushes(
  mdl::Map& map,
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters)
{
  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults,
    map.gameInfo().gameConfig.faceAttribsConfig.uvDefaults};

  const auto materialName = map.currentMaterialName();
  auto* material = map.materialManager().material(materialName);

  const auto snapToInteger = parameters.alignment == SweepAlignment::Integer;
  const auto N = parameters.segments;

  // precompute the transform table once since it never depends on the face or vertex
  const auto transforms = computeTransformTable(source, transform, parameters);

  // a station vertex depends only on (r, s), so adjacent segments compute their shared
  // cap vertices identically and the mesh stays watertight even when snapping
  const auto station = [&](const vm::vec3d& v, const size_t r, const size_t s) {
    const auto p = transforms[r][s] * v;
    // station 0 of the first iteration is the source face itself and must stay exact
    return snapToInteger && (s > 0 || r > 0) ? vm::round(p) : p;
  };

  // each source face produces its own run of brushes, grouped under its original parent
  auto result = std::map<mdl::Node*, std::vector<std::unique_ptr<mdl::BrushNode>>>{};
  for (const auto& sourceFace : source.faces)
  {
    // fall back to the default insertion parent if the captured parent has been deleted
    auto* parent = sourceFace.parent ? sourceFace.parent : parentForNodes(map);

    const auto& sourceVertices = sourceFace.polygon.vertices();
    for (size_t r = 0; r < parameters.iterations; ++r)
    {
      for (size_t i = 0; i < N; ++i)
      {
        const auto points = sourceVertices | std::views::transform([&](const auto& v) {
                              return std::array{station(v, r, i), station(v, r, i + 1)};
                            })
                            | std::views::join | kdl::ranges::to<std::vector>();

        builder.createBrush(points, materialName) | kdl::transform([&](auto brush) {
          setMaterial(brush, material);
          result[parent].push_back(std::make_unique<mdl::BrushNode>(std::move(brush)));
        }) | kdl::transform_error([&](auto e) {
          // a degenerate segment cannot form a brush; skip it
          map.logger().debug() << "Sweep: could not create segment brush: " << e.msg;
        });
      }
    }
  }

  return result;
}

} // namespace tb::ui
