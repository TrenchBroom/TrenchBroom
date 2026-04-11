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

#include "mdl/PatchUtils.h"

#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"

#include "kd/contracts.h"

#include "vm/scalar.h"
#include "vm/vec.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
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
  return vm::variance(a0, a1, a2, a3);
}

// Computes the variance of the edge lengths of a quad
double computeQuadEdgeSymmetryScore(
  const vm::vec3d& v0, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3)
{
  const auto e0 = vm::length(v1 - v0);
  const auto e1 = vm::length(v2 - v1);
  const auto e2 = vm::length(v3 - v2);
  const auto e3 = vm::length(v0 - v3);
  return vm::variance(e0, e1, e2, e3);
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

/*
 * The functions below resample a Bezier patch onto a different number of control points
 * by performing an L2 (least squares) projection of its surface onto the new control
 * point grid, rather than naively sampling the surface and using those samples as control
 * points.
 *
 * Every new control point is computed so that the resulting patch is the best possible
 * least squares approximation of the original surface, given that the four corners of
 * each new sub-surface must lie exactly on the original surface (this is an inherent
 * property of how a Bezier patch's corners interpolate its boundary). Concretely, for
 * each new sub-surface, we determine its 9 control points in this order:
 * - the 4 corners, by directly evaluating the original surface (always exact)
 * - the 4 edge midpoints, each fit along a single direction with both adjacent corners
 *   fixed
 * - the center point, fit in both directions at once with the other 8 points fixed
 *
 * The needed integrals are computed via Gauss-Legendre quadrature, split into pieces at
 * the boundaries of the original patch's sub-surfaces. This keeps each piece's integrand
 * an exact low degree polynomial, so the projection reproduces the original surface
 * exactly whenever that is mathematically possible (in particular, whenever the new
 * sub-surface boundaries are a superset of the original ones, e.g. resampling at the same
 * resolution, or refining to a multiple of the original sub-surface count), and yields
 * the closest quadratic approximation otherwise.
 */

// 3-point Gauss-Legendre quadrature nodes and weights on [0, 1]
const auto gaussLegendreNodes = std::array<double, 3>{
  0.5 - 0.5 * std::sqrt(3.0 / 5.0), 0.5, 0.5 + 0.5 * std::sqrt(3.0 / 5.0)};
const auto gaussLegendreWeights =
  std::array<double, 3>{5.0 / 18.0, 8.0 / 18.0, 5.0 / 18.0};

/*
 * Returns the original patch's sub-surface boundaries (excluding 0 and 1) that lie
 * strictly between a and b, used to split an integral into pieces with exact polynomial
 * integrands.
 *
 * For example, with oldSubSurfaceCount = 3 (old boundaries at 1/3 and 2/3):
 * - (a, b) = (0, 1), the whole domain: returns {1/3, 2/3}, both boundaries.
 * - (a, b) = (0, 0.5), a new sub-surface covering only the first old one and part of the
 *   second: returns {1/3}; 2/3 lies outside [a, b] and is excluded.
 * - (a, b) = (0.5, 1): returns {2/3}; 1/3 lies outside [a, b] and is excluded.
 *
 * With oldSubSurfaceCount = 2 (old boundary at 0.5): (a, b) = (0.5, 1) returns {};
 * the only old boundary coincides with the new one's own corner at a, so it is excluded
 * rather than producing a redundant split there.
 */
std::vector<double> subSurfaceBoundariesWithin(
  const double a, const double b, const size_t oldSubSurfaceCount)
{
  static constexpr auto epsilon = 1e-9;

  auto result = std::vector<double>{};
  for (size_t k = 1; k < oldSubSurfaceCount; ++k)
  {
    const auto boundary = double(k) / double(oldSubSurfaceCount);
    if (boundary > a + epsilon && boundary < b - epsilon)
    {
      result.push_back(boundary);
    }
  }
  return result;
}

/*
 * Computes the new edge control point between cornerA (at parameter a) and cornerB (at
 * parameter b), fitting it to the original patch's surface along the given direction with
 * the corners held fixed. fixedOther is the patch's other (constant) parameter.
 */
BezierPatch::Point fitEdgeControlPoint(
  const BezierPatch& patch,
  const double a,
  const double b,
  const double fixedOther,
  const bool varyingIsColumn,
  const size_t oldSubSurfaceCount,
  const BezierPatch::Point& cornerA,
  const BezierPatch::Point& cornerB)
{
  const auto sample = [&](const double t) {
    return varyingIsColumn ? patch.evaluateAt(t, fixedOther)
                           : patch.evaluateAt(fixedOther, t);
  };

  auto bounds = subSurfaceBoundariesWithin(a, b, oldSubSurfaceCount);
  bounds.insert(bounds.begin(), a);
  bounds.push_back(b);

  // The closed form factors below (15, 0.75) assume integration with respect to the local
  // parameter s in [0, 1], not the global parameter t in [a, b], so each cell's
  // contribution is weighted by width / (b - a) rather than just width.
  const auto intervalWidth = b - a;

  auto integral = BezierPatch::Point{};
  for (size_t i = 0; i + 1 < bounds.size(); ++i)
  {
    const auto lo = bounds[i];
    const auto width = bounds[i + 1] - lo;
    for (size_t g = 0; g < 3; ++g)
    {
      const auto t = lo + gaussLegendreNodes[g] * width;
      const auto localS = (t - a) / (b - a);
      const auto weight =
        gaussLegendreWeights[g] * (width / intervalWidth) * localS * (1.0 - localS);
      integral = integral + weight * sample(t);
    }
  }

  // A quadratic Bezier curve B(s) = (1-s)^2 P0 + 2s(1-s) P1 + s^2 P2 is linear in P1, so
  // minimizing the L2 error integral_0^1 (B(s) - f(s))^2 ds over P1 alone means setting
  // its derivative with respect to P1 to zero, i.e. integral_0^1 (B(s) - f(s)) * s(1-s)
  // ds = 0 (the constant factor 2 from the chain rule drops out as it doesn't affect the
  // root). Substituting B(s) and using the standard Beta-function integrals
  // integral_0^1 s(1-s)^3 ds = integral_0^1 s^3(1-s) ds = 1/20 and
  // integral_0^1 s^2(1-s)^2 ds = 1/30 to evaluate the P0, P1 and P2 terms gives:
  //   (1/20)(P0 + P2) + (1/15) P1 = integral_0^1 f(s) * s(1-s) ds =: integral
  // which rearranges to the closed form used here:
  //   P1 = 15 * integral - 0.75 * (P0 + P2)
  return 15.0 * integral - 0.75 * (cornerA + cornerB);
}

/*
 * Computes the new center control point of a sub-surface spanning [uA, uB] x [vA, vB],
 * fitting it to the original patch's surface with the other 8 control points of the
 * sub-surface held fixed.
 */
BezierPatch::Point fitCenterControlPoint(
  const BezierPatch& patch,
  const double uA,
  const double uB,
  const size_t oldColumnSubSurfaceCount,
  const double vA,
  const double vB,
  const size_t oldRowSubSurfaceCount,
  const BezierPatch::Point& corner00,
  const BezierPatch::Point& corner02,
  const BezierPatch::Point& corner20,
  const BezierPatch::Point& corner22,
  const BezierPatch::Point& edge01,
  const BezierPatch::Point& edge21,
  const BezierPatch::Point& edge10,
  const BezierPatch::Point& edge12)
{
  auto uBounds = subSurfaceBoundariesWithin(uA, uB, oldColumnSubSurfaceCount);
  uBounds.insert(uBounds.begin(), uA);
  uBounds.push_back(uB);

  auto vBounds = subSurfaceBoundariesWithin(vA, vB, oldRowSubSurfaceCount);
  vBounds.insert(vBounds.begin(), vA);
  vBounds.push_back(vB);

  // As in fitEdgeControlPoint, the closed form factors assume integration with respect to
  // the local parameters s, t in [0, 1], so each cell's contribution is weighted by
  // width / intervalWidth rather than just width, in both directions.
  const auto uIntervalWidth = uB - uA;
  const auto vIntervalWidth = vB - vA;

  auto integral = BezierPatch::Point{};
  for (size_t vi = 0; vi + 1 < vBounds.size(); ++vi)
  {
    const auto vLo = vBounds[vi];
    const auto vWidth = vBounds[vi + 1] - vLo;
    for (size_t vg = 0; vg < 3; ++vg)
    {
      const auto v = vLo + gaussLegendreNodes[vg] * vWidth;
      const auto localT = (v - vA) / (vB - vA);
      const auto weightV =
        gaussLegendreWeights[vg] * (vWidth / vIntervalWidth) * localT * (1.0 - localT);

      for (size_t ui = 0; ui + 1 < uBounds.size(); ++ui)
      {
        const auto uLo = uBounds[ui];
        const auto uWidth = uBounds[ui + 1] - uLo;
        for (size_t ug = 0; ug < 3; ++ug)
        {
          const auto u = uLo + gaussLegendreNodes[ug] * uWidth;
          const auto localS = (u - uA) / (uB - uA);
          const auto weightU = gaussLegendreWeights[ug] * (uWidth / uIntervalWidth)
                               * localS * (1.0 - localS);

          integral = integral + (weightV * weightU) * patch.evaluateAt(u, v);
        }
      }
    }
  }

  // Analogous to fitEdgeControlPoint, but for the biquadratic surface B(s,t) = sum_i
  // sum_j Bi(s) Bj(t) Pij, where B0, B1, B2 are the quadratic Bernstein basis functions.
  // Setting the derivative with respect to the center point P11 to zero and expanding
  // B(s,t) reduces, term by term, to the same three 1D integrals used above (1/20 for the
  // B0 and B2 terms, 1/15 for the B1 term), applied independently to s and t:
  //
  //   (1/20 * 1/20) cornerSum + (1/20 * 1/15) edgeSum + (1/15 * 1/15) P11
  //  = integral_0^1 integral_0^1 f(s,t) * s(1-s) t(1-t) ds dt =: integral
  //
  // where cornerSum and edgeSum below sum the 4 corner and 4 edge control points
  // respectively (each corner contributes a 1/20 * 1/20 = 1/400 term, each edge a 1/20 *
  // 1/15 = 1/300 term). Rearranging for P11 gives: P11 = 225 * integral - 0.5625 *
  // cornerSum - 0.75 * edgeSum (225 = 1 / (1/15)^2, 0.5625 = (1/400) * 225, 0.75 =
  // (1/300) * 225)
  const auto cornerSum = corner00 + corner02 + corner20 + corner22;
  const auto edgeSum = edge01 + edge21 + edge10 + edge12;
  return 225.0 * integral - 0.5625 * cornerSum - 0.75 * edgeSum;
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

  // Clip the polygon into quads by starting with (V[0],V[1],V[2],V[3]) and then
  // repeatedly extending outward by one vertex on each side of the current quad's
  // diagonal (the one side of the quad that is not a boundary edge of the face):
  //
  //   (V[0],V[1],V[2],V[3]), (V[N-1],V[0],V[3],V[4]), (V[N-2],V[N-1],V[4],V[5]), ...
  //
  // Each new quad shares the previous quad's diagonal (never a boundary edge) with it,
  // so the patches tile the polygon exactly, without overlap or gaps. If a single vertex
  // is left over, it becomes the apex of a degenerate (triangular) patch closing the same
  // diagonal as the last quad.
  //
  // Total patch count: floor((N-1)/2)

  const auto patchCount = (n - 1) / 2;
  auto patches = std::vector<BezierPatch>{};
  patches.reserve(patchCount);

  if (n == 3)
  {
    patches.push_back(makePatch(
      face,
      pointRowCount,
      pointColumnCount,
      vertices[1],
      vertices[2],
      vertices[0],
      vertices[0]));
    return patches;
  }

  patches.push_back(makePatch(
    face,
    pointRowCount,
    pointColumnCount,
    vertices[0],
    vertices[1],
    vertices[2],
    vertices[3]));

  auto lo = size_t{0};
  auto hi = size_t{3};
  auto remaining = n - 4;

  while (remaining >= 2)
  {
    const auto newLo = (lo + n - 1) % n;
    const auto newHi = (hi + 1) % n;
    patches.push_back(makePatch(
      face,
      pointRowCount,
      pointColumnCount,
      vertices[newLo],
      vertices[lo],
      vertices[hi],
      vertices[newHi]));
    lo = newLo;
    hi = newHi;
    remaining -= 2;
  }

  if (remaining == 1)
  {
    const auto last = (hi + 1) % n;
    patches.push_back(makePatch(
      face,
      pointRowCount,
      pointColumnCount,
      vertices[hi],
      vertices[last],
      vertices[lo],
      vertices[lo]));
  }

  return patches;
}

/*
 * The new pointRowCount x pointColumnCount control points form a grid of (pointRowCount -
 * 1) / 2 x (pointColumnCount - 1) / 2 new 3x3 sub-surfaces, tiled the same way
 * BezierPatch itself stores them: adjacent sub-surfaces share their common border row or
 * column of control points, so each grid point belongs to up to four sub-surfaces but
 * must only be computed once.
 *
 * A grid point's role, and therefore how it's computed, is determined by the parity of
 * its (row, col) indices in the new grid:
 * - both even: a sub-surface corner, fixed by directly evaluating the original surface
 *   (always exact, see the file comment above)
 * - exactly one odd: an edge midpoint, fit along the odd axis with the two corners on
 *   either side of it held fixed (fitEdgeControlPoint)
 * - both odd: a sub-surface center, fit in both directions at once with the surrounding 4
 *   corners and 4 edge midpoints held fixed (fitCenterControlPoint)
 *
 * Edge and center fits depend on already-computed corners (and, for centers, edges too),
 * so the grid is filled in four passes, one per role, in that dependency order: corners,
 * then horizontal edges (row even, col odd), then vertical edges (row odd, col even),
 * then centers. globalU/globalV convert a grid index to the patch's normalized [0, 1]
 * parameter space; oldRowSubSurfaceCount/oldColumnSubSurfaceCount are the original
 * patch's sub-surface counts, passed through to the fit functions so they know where to
 * split their integrals for exactness (see subSurfaceBoundariesWithin).
 */
BezierPatch resamplePatch(
  const BezierPatch& patch, const size_t pointRowCount, const size_t pointColumnCount)
{
  contract_assert(pointRowCount > 2 && pointRowCount % 2 == 1);
  contract_assert(pointColumnCount > 2 && pointColumnCount % 2 == 1);

  const auto oldRowSubSurfaceCount = patch.surfaceRowCount();
  const auto oldColumnSubSurfaceCount = patch.surfaceColumnCount();

  auto controlPoints = std::vector<BezierPatch::Point>(pointRowCount * pointColumnCount);
  const auto at = [&](const size_t row, const size_t col) -> BezierPatch::Point& {
    return controlPoints[row * pointColumnCount + col];
  };
  const auto globalU = [&](const size_t col) {
    return double(col) / double(pointColumnCount - 1);
  };
  const auto globalV = [&](const size_t row) {
    return double(row) / double(pointRowCount - 1);
  };

  // Pass 1: corners (row, col both even).
  for (size_t row = 0; row < pointRowCount; row += 2)
  {
    for (size_t col = 0; col < pointColumnCount; col += 2)
    {
      at(row, col) = patch.evaluateAt(globalU(col), globalV(row));
    }
  }

  // Pass 2: horizontal edge midpoints (row even, col odd), varying u at fixed v.
  for (size_t row = 0; row < pointRowCount; row += 2)
  {
    for (size_t col = 1; col < pointColumnCount; col += 2)
    {
      at(row, col) = fitEdgeControlPoint(
        patch,
        globalU(col - 1),
        globalU(col + 1),
        globalV(row),
        true,
        oldColumnSubSurfaceCount,
        at(row, col - 1),
        at(row, col + 1));
    }
  }

  // Pass 3: vertical edge midpoints (row odd, col even), varying v at fixed u.
  for (size_t row = 1; row < pointRowCount; row += 2)
  {
    for (size_t col = 0; col < pointColumnCount; col += 2)
    {
      at(row, col) = fitEdgeControlPoint(
        patch,
        globalV(row - 1),
        globalV(row + 1),
        globalU(col),
        false,
        oldRowSubSurfaceCount,
        at(row - 1, col),
        at(row + 1, col));
    }
  }

  // Pass 4: centers (row, col both odd).
  for (size_t row = 1; row < pointRowCount; row += 2)
  {
    for (size_t col = 1; col < pointColumnCount; col += 2)
    {
      at(row, col) = fitCenterControlPoint(
        patch,
        globalU(col - 1),
        globalU(col + 1),
        oldColumnSubSurfaceCount,
        globalV(row - 1),
        globalV(row + 1),
        oldRowSubSurfaceCount,
        at(row - 1, col - 1),
        at(row - 1, col + 1),
        at(row + 1, col - 1),
        at(row + 1, col + 1),
        at(row - 1, col),
        at(row + 1, col),
        at(row, col - 1),
        at(row, col + 1));
    }
  }

  return BezierPatch{
    pointRowCount, pointColumnCount, std::move(controlPoints), patch.materialName()};
}

} // namespace tb::mdl
