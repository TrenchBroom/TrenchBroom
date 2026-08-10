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

#pragma once

#include "kd/reflection_decl.h"

#include "vm/polygon.h"
#include "vm/quat.h"
#include "vm/vec.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

namespace tb
{
namespace mdl
{
class BrushNode;
class Map;
class Node;
} // namespace mdl

namespace ui
{

enum class SweepAlignment
{
  Integer,
  Free,
};

std::ostream& operator<<(std::ostream& lhs, SweepAlignment rhs);

enum class SweepPathMode
{
  Arc,
  Straight,
  SBend,
};

std::ostream& operator<<(std::ostream& lhs, SweepPathMode rhs);

struct SweepFace
{
  vm::polygon3d polygon;
  mdl::Node* parent = nullptr;

  kdl_reflect_decl(SweepFace, polygon, parent);
};

struct SweepSource
{
  std::vector<SweepFace> faces;
  vm::vec3d center;
  vm::vec3d normal;
  vm::vec3d scaleBaseVector;

  kdl_reflect_decl(SweepSource, faces, center, normal, scaleBaseVector);
};

struct SweepTransform
{
  vm::vec3d translation = vm::vec3d{0, 0, 0};
  vm::quatd rotation = vm::quatd{vm::vec3d{0, 0, 1}, 0.0};
  vm::vec3d scale = vm::vec3d{1, 1, 1};

  vm::vec3d destinationCenter(const SweepSource& source) const;
  vm::quatd effectiveRotation() const;
  bool isNoOp() const;

  kdl_reflect_decl(SweepTransform, translation, rotation, scale);
};

struct SweepParameters
{
  size_t segments = 8;
  size_t iterations = 1;
  SweepPathMode pathMode = SweepPathMode::Arc;
  SweepAlignment alignment = SweepAlignment::Integer;

  kdl_reflect_decl(SweepParameters, segments, iterations, pathMode, alignment);
};

/**
 * Computes the transform for a single station (cross-section) along one iteration of the
 * sweep, at fractional position `t` (0 at the source face, 1 at the destination).
 *
 * `rotationFull` is the rotation for a complete iteration (t = 1); this function scales
 * its angle by `t` so that intermediate stations rotate proportionally. In arc mode, the
 * source is revolved about a pivot derived from `source` and `transform`, with the
 * translation along the rotation axis applied as a helical lift; if no usable pivot
 * exists, this falls through to the straight-path behavior. In straight/S-bend mode, the
 * source is scaled and rotated about its center and then translated, following an S-curve
 * offset when `parameters.pathMode` is `SweepPathMode::SBend` and `source.normal` is
 * non-zero.
 *
 * The result does not include the transforms of preceding iterations; callers that sweep
 * multiple iterations compose this with the accumulated transform of prior iterations.
 */
vm::mat4x4d stationTransform(
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters,
  double t,
  const vm::quatd& rotationFull);

/**
 * Generates the brushes that make up a swept shape, by tessellating each source face into
 * `parameters.segments` stations per iteration (via `stationTransform`) and building one
 * brush per pair of adjacent stations.
 *
 * Source faces are swept independently, and the resulting brushes are grouped by the
 * parent node captured on each `SweepFace` (falling back to the map's default insertion
 * parent if that node no longer exists). Station vertices are rounded to integer
 * coordinates when `parameters.alignment` is `SweepAlignment::Integer`, except at the
 * very first station of the first iteration, which is kept exact since it corresponds to
 * the original source face. Segments that degenerate into invalid brushes are skipped and
 * logged rather than causing the whole sweep to fail.
 */
std::map<mdl::Node*, std::vector<std::unique_ptr<mdl::BrushNode>>> generateSweepBrushes(
  mdl::Map& map,
  const SweepSource& source,
  const SweepTransform& transform,
  const SweepParameters& parameters);

} // namespace ui
} // namespace tb
