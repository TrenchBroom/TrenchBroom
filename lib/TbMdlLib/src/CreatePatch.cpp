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

#include "mdl/CreatePatch.h"

#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/UVUtils.h"

#include "vm/vec.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace tb::mdl
{
namespace
{

BezierPatch makePatch(
  const BrushFace& face,
  const size_t pointRowCount,
  const size_t pointColumnCount,
  const vm::vec3d& c0,
  const vm::vec3d& c1,
  const vm::vec3d& c2,
  const vm::vec3d& c3)
{
  auto controlPoints = std::vector<BezierPatch::Point>{};
  controlPoints.reserve(pointRowCount * pointColumnCount);

  for (size_t i = 0; i < pointRowCount; ++i)
  {
    const auto s = double(i) / double(pointRowCount - 1);
    for (size_t j = 0; j < pointColumnCount; ++j)
    {
      const auto t = double(j) / double(pointColumnCount - 1);

      // Bilinear interpolation of the four corners:
      // B(s,t) = (1-s)(1-t)*c0 + (1-s)*t*c1 + s*t*c2 + s*(1-t)*c3
      //
      // Corner layout:
      //   c0 = (row=0,    col=0   )
      //   c1 = (row=0,    col=last)
      //   c2 = (row=last, col=last)
      //   c3 = (row=last, col=0   )
      const auto pos =
        (1.0 - s) * (1.0 - t) * c0 + (1.0 - s) * t * c1 + s * t * c2 + s * (1.0 - t) * c3;

      const auto uv = vm::vec2d{face.uvCoords(pos)};
      controlPoints.push_back(
        BezierPatch::Point{pos.x(), pos.y(), pos.z(), uv.x(), uv.y()});
    }
  }

  return BezierPatch{
    pointRowCount,
    pointColumnCount,
    std::move(controlPoints),
    face.attributes().materialName()};
}

double measureAngle(
  const vm::vec3d& v0, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& normal)
{
  return vm::measure_angle(v0 - v1, v2 - v1, normal);
}

// Computes the variance of the internal angles (in radians) of a quad
double computeQuadAngleSymmetryScore(
  const vm::vec3d& v0,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& normal)
{
  const auto a0 = measureAngle(v3, v0, v1, normal);
  const auto a1 = measureAngle(v0, v1, v2, normal);
  const auto a2 = measureAngle(v1, v2, v3, normal);
  const auto a3 = measureAngle(v2, v3, v0, normal);

  const auto mean = (a0 + a1 + a2 + a3) / 4.0;
  const auto var = (a0 - mean) * (a0 - mean) + (a1 - mean) * (a1 - mean)
                   + (a2 - mean) * (a2 - mean) + (a3 - mean) * (a3 - mean);
  return var;
}

// Computes the variance of the edge lengths of a quad
double computeQuadEdgeSymmetryScore(
  const vm::vec3d& v0, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3)
{
  const auto e0 = vm::length(v1 - v0);
  const auto e1 = vm::length(v2 - v1);
  const auto e2 = vm::length(v3 - v2);
  const auto e3 = vm::length(v0 - v3);
  const auto mean = (e0 + e1 + e2 + e3) / 4.0;
  const auto var = (e0 - mean) * (e0 - mean) + (e1 - mean) * (e1 - mean)
                   + (e2 - mean) * (e2 - mean) + (e3 - mean) * (e3 - mean);
  return var;
}

// Combines angle and edge symmetry scores for a quad
auto computeQuadSymmetryScore(
  const vm::vec3d& v0,
  const vm::vec3d& v1,
  const vm::vec3d& v2,
  const vm::vec3d& v3,
  const vm::vec3d& normal)
{
  const auto angleScore = computeQuadAngleSymmetryScore(v0, v1, v2, v3, normal);
  const auto edgeScore = computeQuadEdgeSymmetryScore(v0, v1, v2, v3);
  return std::tuple{angleScore, edgeScore};
}

auto selectStartVertex(std::vector<vm::vec3d> vertices, const vm::vec3d& normal)
{
  std::ranges::reverse(vertices);

  const auto n = vertices.size();
  if (n >= 4)
  {
    // For quads and higher, try all possible consecutive quads and pick the most
    // symmetric
    const auto mapToScore = [&](const auto i) {
      // Indices of the quad: i, i+1, i+2, i+3 (mod n)
      const auto& v0 = vertices[i % n];
      const auto& v1 = vertices[(i + 1) % n];
      const auto& v2 = vertices[(i + 2) % n];
      const auto& v3 = vertices[(i + 3) % n];
      return computeQuadSymmetryScore(v0, v1, v2, v3, normal);
    };

    const auto bestStartIndex = *std::ranges::min_element(
      std::views::iota(0u, n), std::less<std::tuple<double, double>>{}, mapToScore);

    // Rotate so that bestStart is first
    std::ranges::rotate(vertices, std::next(vertices.begin(), bestStartIndex));
  }

  return vertices;
}

} // namespace

std::vector<BezierPatch> createPatch(
  const BrushFace& face, const size_t pointRowCount, const size_t pointColumnCount)
{
  contract_assert(pointRowCount > 2 && pointRowCount % 2 == 1);
  contract_assert(pointColumnCount > 2 && pointColumnCount % 2 == 1);

  const auto n = face.vertexCount();
  contract_assert(n >= 3);

  const auto vertices = selectStartVertex(face.vertexPositions(), face.normal());

  // Clip the polygon into patches using sliding windows over boundary vertices:
  //
  //   Full quads: (V[0],V[1],V[2],V[3]), (V[2],V[3],V[4],V[5]), ...
  //
  // If N is odd, append a degenerate (triangular) patch from the last 3 vertices:
  //
  //   (V[N-3], V[N-2], V[N-1], V[N-1])
  //
  // Total patch count: floor((N-1)/2)

  const auto patchCount = (n - 1) / 2;
  auto patches = std::vector<BezierPatch>{};
  patches.reserve(patchCount);

  for (size_t first = 0; first + 3 < n; first += 2)
  {
    patches.push_back(makePatch(
      face,
      pointRowCount,
      pointColumnCount,
      vertices[first],
      vertices[first + 1],
      vertices[first + 2],
      vertices[first + 3]));
  }

  if (n % 2 != 0)
  {
    // Degenerate triangle: use the last three vertices, wrapping to the start for c3.
    // This avoids overlap with the previous quad patch.
    patches.push_back(makePatch(
      face,
      pointRowCount,
      pointColumnCount,
      vertices[n - 2],
      vertices[n - 1],
      vertices[0],
      vertices[0]));
  }

  return patches;
}
} // namespace tb::mdl