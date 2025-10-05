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

#include "kdl/reflection_decl.h"

#include "vm/bbox.h"
#include "vm/mat.h"
#include "vm/polygon.h"
#include "vm/segment.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <string>
#include <vector>

namespace tb::mdl
{
class Map;

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
  Map& map, std::vector<vm::vec3d> vertexPositions, const vm::mat4x4d& transform);
bool transformEdges(
  Map& map, std::vector<vm::segment3d> edgePositions, const vm::mat4x4d& transform);
bool transformFaces(
  Map& map, std::vector<vm::polygon3d> facePositions, const vm::mat4x4d& transform);

bool addVertex(Map& map, const vm::vec3d& vertexPosition);
bool removeVertices(
  Map& map, const std::string& commandName, std::vector<vm::vec3d> vertexPositions);

bool snapVertices(Map& map, double snapTo);

bool csgConvexMerge(Map& map);
bool csgSubtract(Map& map);
bool csgIntersect(Map& map);
bool csgHollow(Map& map);

bool extrudeBrushes(
  Map& map, const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta);

} // namespace tb::mdl
