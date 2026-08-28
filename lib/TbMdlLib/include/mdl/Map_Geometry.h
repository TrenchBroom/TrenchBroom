/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/BrushOptimization.h"

#include "kd/reflection_decl.h"

#include "vm/bbox.h"
#include "vm/mat.h"
#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{
class Map;
class BrushNode;
class Node;

/**
 * Describes one convex brush to create from a set of vertices. The points are passed to
 * BrushBuilder, so they must describe a valid convex polyhedron within the map bounds.
 */
struct BrushCreationSpec
{
  std::vector<vm::vec3d> points;
  std::string materialName;
};

/**
 * A 2D, open path network swept through a rectangular vertical profile. Paths are
 * deliberately kept separate: a junction or overlapping run is ambiguous authored
 * geometry and is rejected instead of silently filling extra space.
 */
struct PlanarPathSweepSpec
{
  std::vector<std::vector<vm::vec2d>> chains;
  double bottom;
  double top;
  double thickness;
  std::string materialName;
};

/**
 * Builds one convex prism per non-collinear path run. Adjacent runs use a bounded
 * miter at their common vertex, so ordinary turns have neither a crack nor a
 * doubled-up corner. Collinear points are removed before brushes are generated.
 * Returns no value for degenerate, reversing, or excessively acute turns.
 */
std::optional<std::vector<BrushCreationSpec>> createPlanarPathSweepBrushSpecs(
  const PlanarPathSweepSpec& spec);

/**
 * One horizontal band of a closed planar profile. Insets are measured from the source
 * contour (not from the preceding band), which makes the profile stable when a client
 * reorders or adds bands. Every band is extruded between bottom and top.
 */
struct PlanarProfileBand
{
  double inset;
  double bottom;
  double top;
  std::string materialName;
  std::string role;
};

/** An optional filled centre remaining after the last profile band. */
struct PlanarProfileCore
{
  double bottom;
  double top;
  std::string materialName;
  std::string role;
};

/**
 * A closed simple XY contour, decomposed into mitered inset rings and an optional
 * triangulated core. The generated solids are always convex vertical prisms. Insets
 * that collapse, self-intersect, or invert their contour are rejected.
 */
struct PlanarProfileSpec
{
  std::vector<vm::vec2d> contour;
  std::vector<PlanarProfileBand> bands;
  std::optional<PlanarProfileCore> core;
  double gridSize = 1.0;
};

struct PlanarProfileBrushSpec
{
  BrushCreationSpec brush;
  std::string role;
};

std::optional<std::vector<PlanarProfileBrushSpec>> createPlanarProfileBrushSpecs(
  const PlanarProfileSpec& spec);

/** The plane in which a two-dimensional profile is authored. */
enum class ProfileExtrusionPlane
{
  XY,
  XZ,
  YZ,
};

/**
 * A closed two-dimensional profile extruded through an interval on the remaining
 * coordinate axis. Convex profiles produce one brush; simple concave profiles are
 * deterministically ear-triangulated into convex prisms.
 */
struct ProfileExtrusionSpec
{
  ProfileExtrusionPlane plane;
  std::vector<vm::vec2d> profile;
  double minimum;
  double maximum;
  double gridSize = 1.0;
  std::string materialName;
  std::string role;
};

/**
 * Creates grid-snapped, convex brush specifications for a profile extrusion. The
 * result uses the same role-bearing output type as planar profile construction.
 */
std::optional<std::vector<PlanarProfileBrushSpec>> createProfileExtrusionBrushSpecs(
  const ProfileExtrusionSpec& spec);

bool transformSelection(
  Map& map, const std::string& commandName, const vm::mat4x4d& transformation);

bool translateSelection(Map& map, const vm::vec3d& delta);
bool rotateSelection(
  Map& map, const vm::vec3d& center, const vm::vec3d& axis, double angle);
bool scaleSelection(Map& map, const vm::bbox3d& oldBBox, const vm::bbox3d& newBBox);
bool scaleSelection(Map& map, const vm::vec3d& center, const vm::vec3d& scaleFactors);
bool shearSelection(
  Map& map, const vm::bbox3d& box, const vm::vec3d& sideToShear, const vm::vec3d& delta);
bool flipSelection(Map& map, const vm::vec3d& center, vm::axis::type axis);

struct TransformVerticesResult
{
  bool success;
  bool hasRemainingVertices;

  kdl_reflect_decl(TransformVerticesResult, success, hasRemainingVertices);
};

TransformVerticesResult transformVertices(
  Map& map, const std::vector<vm::vec3d>& vertexPositions, const vm::mat4x4d& transform);
bool transformEdges(
  Map& map,
  const std::vector<vm::segment3d>& edgePositions,
  const vm::mat4x4d& transform);
bool transformFaces(
  Map& map,
  const std::vector<vm::polygon3d>& facePositions,
  const vm::mat4x4d& transform);

bool addVertex(Map& map, const vm::vec3d& vertexPosition);
bool removeVertices(
  Map& map, const std::string& commandName, std::vector<vm::vec3d> vertexPositions);

bool snapVertices(Map& map, double snapTo);

bool csgConvexMerge(Map& map);
bool csgSubtract(Map& map);

/**
 * Subtracts subtrahends from exactly the given minuends. Unlike csgSubtract(Map&),
 * this does not discover additional touching brushes. The input sets must be non-empty,
 * contain distinct brushes belonging to map, and be disjoint. The subtrahends and
 * minuends are removed; replacementNodes receives every fragment created for the
 * minuends on success.
 */
bool csgSubtract(
  Map& map,
  const std::vector<BrushNode*>& subtrahendNodes,
  const std::vector<BrushNode*>& minuendNodes,
  std::vector<BrushNode*>& replacementNodes);
bool csgIntersect(Map& map);
bool csgHollow(Map& map);

bool canOptimizeSelectedBrushes(const Map& map);
std::vector<BrushOptimizationCandidate> createSelectedBrushOptimizationCandidates(
  const Map& map);
bool applyBrushOptimizationCandidate(
  Map& map, const BrushOptimizationCandidate& candidate);

bool canOptimizeBrushes(const std::vector<BrushNode*>& brushNodes);
std::vector<BrushOptimizationCandidate> createBrushOptimizationCandidates(
  const Map& map, const std::vector<BrushNode*>& brushNodes);
std::vector<std::vector<BrushNode*>> findBrushOptimizationCohorts(
  const Map& map, const std::vector<BrushNode*>& brushNodes);
bool applyBrushOptimizationCandidate(
  Map& map,
  const std::vector<BrushNode*>& brushNodes,
  const BrushOptimizationCandidate& candidate);

/**
 * Returns whether resultNodes are one of the exact, lower-count decompositions the
 * brush optimizer would produce for sourceNodes. Each range must contain sibling
 * brushes. This is useful for recognizing an optimization after the original
 * brush-node identities have been replaced.
 */
bool isBrushOptimizationResult(
  MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<const BrushNode*>& sourceNodes,
  const std::vector<const BrushNode*>& resultNodes);

enum class BridgeSurfaceDirection
{
  Below,
  Above,
  Centered,
};

bool canBridgeSelectedEdgeChains(const Map& map);
bool bridgeSelectedEdgeChains(
  Map& map,
  double thickness,
  BridgeSurfaceDirection direction,
  const std::string& materialName);

bool canCreateVolumeToPlane(const Map& map);
bool createVolumeToPlane(
  Map& map, vm::axis::type axis, double coordinate, const std::string& materialName);

bool canCreateEqWater(const Map& map);
bool createEqWater(
  Map& map,
  double surfaceHeight,
  double surfaceThickness,
  const std::string& waterMaterialName,
  const std::string& surfaceMaterialName);

/**
 * Creates every requested brush below parent as one undoable map operation. All brush
 * geometry is built before the map is changed, so an invalid specification leaves the
 * map untouched. On success, createdBrushes contains the newly inserted nodes.
 */
bool createBrushes(
  Map& map,
  Node& parent,
  const std::vector<BrushCreationSpec>& specs,
  std::vector<BrushNode*>& createdBrushes);

bool extrudeBrushes(
  Map& map, const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta);

} // namespace tb::mdl
