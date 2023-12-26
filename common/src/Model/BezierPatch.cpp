/*
 Copyright (C) 2021 Kristian Duske

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

#include "BezierPatch.h"

#include "Assets/Texture.h"
#include "Ensure.h"
#include "Uuid.h"

#include <kdl/reflection_impl.h>

#include <vecmath/bbox_io.h>
#include <vecmath/bezier_surface.h>
#include <vecmath/vec_io.h>

#include <cassert>

namespace TrenchBroom::Model
{

kdl_reflect_impl(BezierPatch);

namespace
{
vm::bbox3 computeBounds(const std::vector<BezierPatch::Point>& points)
{
  auto builder = vm::bbox3::builder{};
  for (const auto& point : points)
  {
    builder.add(point.xyz());
  }
  return builder.bounds();
}
} // namespace

BezierPatch::BezierPatch(
  const size_t pointRowCount,
  const size_t pointColumnCount,
  std::vector<Point> controlPoints,
  std::string textureName)
  : m_pointRowCount{pointRowCount}
  , m_pointColumnCount{pointColumnCount}
  , m_controlPoints{std::move(controlPoints)}
  , m_bounds(computeBounds(m_controlPoints))
  , m_linkId{generateUuid()}
  , m_textureName{std::move(textureName)}
{
  ensure(
    m_pointRowCount > 2 && m_pointColumnCount > 2,
    "Bezier patch must have at least 3*3 control points");
  ensure(
    m_pointRowCount % 2 == 1 && m_pointColumnCount % 2 == 1,
    "Bezier patch must have odd number of control points per column and per row");
  ensure(
    m_controlPoints.size() == m_pointRowCount * m_pointColumnCount,
    "Invalid Bezier patch control points");
}

BezierPatch::~BezierPatch() = default;

BezierPatch::BezierPatch(const BezierPatch& other) = default;
BezierPatch::BezierPatch(BezierPatch&& other) noexcept = default;

BezierPatch& BezierPatch::operator=(const BezierPatch& other) = default;
BezierPatch& BezierPatch::operator=(BezierPatch&& other) noexcept = default;

const std::string& BezierPatch::linkId() const
{
  return m_linkId;
}

void BezierPatch::setLinkId(std::string linkId)
{
  m_linkId = std::move(linkId);
}

size_t BezierPatch::pointRowCount() const
{
  return m_pointRowCount;
}

size_t BezierPatch::pointColumnCount() const
{
  return m_pointColumnCount;
}

size_t BezierPatch::quadRowCount() const
{
  return m_pointRowCount - 1u;
}

size_t BezierPatch::quadColumnCount() const
{
  return m_pointColumnCount - 1u;
}

size_t BezierPatch::surfaceRowCount() const
{
  return quadRowCount() / 2u;
}

size_t BezierPatch::surfaceColumnCount() const
{
  return quadColumnCount() / 2u;
}

const std::vector<BezierPatch::Point>& BezierPatch::controlPoints() const
{
  return m_controlPoints;
}

const BezierPatch::Point& BezierPatch::controlPoint(
  const size_t row, const size_t col) const
{
  assert(row < m_pointRowCount);
  assert(col < m_pointColumnCount);
  return m_controlPoints[row * m_pointColumnCount + col];
}

void BezierPatch::setControlPoint(const size_t row, const size_t col, Point controlPoint)
{
  assert(row < m_pointRowCount);
  assert(col < m_pointColumnCount);
  m_controlPoints[row * m_pointColumnCount + col] = std::move(controlPoint);
  m_bounds = computeBounds(m_controlPoints);
}

const vm::bbox3& BezierPatch::bounds() const
{
  return m_bounds;
}

const std::string& BezierPatch::textureName() const
{
  return m_textureName;
}

void BezierPatch::setTextureName(std::string textureName)
{
  m_textureName = std::move(textureName);
}

const Assets::Texture* BezierPatch::texture() const
{
  return m_textureReference.get();
}

bool BezierPatch::setTexture(Assets::Texture* texture)
{
  if (texture == this->texture())
  {
    return false;
  }

  m_textureReference = Assets::AssetReference{texture};
  return true;
}

void BezierPatch::transform(const vm::mat4x4& transformation)
{
  auto builder = vm::bbox3::builder{};
  for (auto& controlPoint : m_controlPoints)
  {
    controlPoint =
      Point{transformation * controlPoint.xyz(), controlPoint[3], controlPoint[4]};
    builder.add(controlPoint.xyz());
  }
  m_bounds = builder.bounds();
}

using SurfaceControlPoints = std::array<std::array<BezierPatch::Point, 3u>, 3u>;
static SurfaceControlPoints collectSurfaceControlPoints(
  const std::vector<BezierPatch::Point>& controlPoints,
  const size_t pointColumnCount,
  const size_t surfaceRow,
  const size_t surfaceCol)
{
  // at which column and row do we need to start collecting control points for the
  // surface?
  const size_t rowOffset = 2u * surfaceRow;
  const size_t colOffset = 2u * surfaceCol;

  // collect 3*3 control points
  auto result = SurfaceControlPoints{};
  for (size_t row = 0; row < 3u; ++row)
  {
    for (size_t col = 0; col < 3u; ++col)
    {
      result[row][col] =
        controlPoints[(row + rowOffset) * pointColumnCount + col + colOffset];
    }
  }
  return result;
}

static std::vector<SurfaceControlPoints> collectAllSurfaceControlPoints(
  const std::vector<BezierPatch::Point>& controlPoints,
  const size_t pointRowCount,
  const size_t pointColumnCount)
{
  // determine how many 3*3 surfaces the patch has in each direction
  const size_t surfaceRowCount = (pointRowCount - 1u) / 2u;
  const size_t surfaceColumnCount = (pointColumnCount - 1u) / 2u;

  // collect the control points for each surface
  auto result = std::vector<SurfaceControlPoints>{};
  result.reserve(surfaceRowCount * surfaceColumnCount);

  for (size_t surfaceRow = 0u; surfaceRow < surfaceRowCount; ++surfaceRow)
  {
    for (size_t surfaceCol = 0u; surfaceCol < surfaceColumnCount; ++surfaceCol)
    {
      result.push_back(collectSurfaceControlPoints(
        controlPoints, pointColumnCount, surfaceRow, surfaceCol));
    }
  }
  return result;
}

template <typename O>
void evaluateSurface(
  const SurfaceControlPoints& surfaceControlPoints,
  const size_t subdivisionsPerSurface,
  const bool isLastCol,
  const bool isLastRow,
  O out)
{
  const auto maxRow = isLastRow ? subdivisionsPerSurface + 1u : subdivisionsPerSurface;
  const auto maxCol = isLastCol ? subdivisionsPerSurface + 1u : subdivisionsPerSurface;

  for (size_t row = 0u; row < maxRow; ++row)
  {
    const auto v =
      static_cast<FloatType>(row) / static_cast<FloatType>(subdivisionsPerSurface);
    for (size_t col = 0u; col < maxCol; ++col)
    {
      const auto u =
        static_cast<FloatType>(col) / static_cast<FloatType>(subdivisionsPerSurface);
      out = vm::evaluate_quadratic_bezier_surface(surfaceControlPoints, u, v);
    }
  }
}

std::vector<BezierPatch::Point> BezierPatch::evaluate(
  const size_t subdivisionsPerSurface) const
{
  // collect the control points for each surface in this patch
  const auto allSurfaceControlPoints =
    collectAllSurfaceControlPoints(m_controlPoints, m_pointRowCount, m_pointColumnCount);

  const auto quadsPerSurfaceSide = (1u << subdivisionsPerSurface);

  // determine dimensions of the resulting point grid
  const size_t gridPointRowCount = surfaceRowCount() * quadsPerSurfaceSide + 1u;
  const size_t gridPointColumnCount = surfaceColumnCount() * quadsPerSurfaceSide + 1u;

  auto grid = std::vector<BezierPatch::Point>{};
  grid.reserve(gridPointRowCount * gridPointColumnCount);

  /*
  Next we sample the surfaces to compute each point in the grid.

  Consider the following example of a Bezier patch consisting of 4 surfaces A, B, C, D. In
  the diagram, an asterisk (*) represents a point on the grid, and o represents a point on
  the grid which is shared by adjacent surfaces. Each surface is subdivided into 3*3
  parts, which yields 4*4=16 grid points per surface.

  We compute the grid row by row, so in each iteration, we need to determine which surface
  should be sampled for the grid point. For the shared points, we could sample either
  surface, but we decided (arbitrarily) that for a shared point, we will sample the
  previous surface. In the diagram, the surface column / row index indicates which surface
  will be sampled for each grid point. Suppose we want to compute the grid point at column
  3, row 2. This is a shared point of surfaces A and B, and per our rule, we will sample
  surface A.

  This also affects how we compute the u and v values which we use to sample each surface.
  Note that for shared grid points, either u or v or both are always 1. This is necessary
  because we are still sampling the preceeding surface for the shared grid points.

            0   1/4  2/4  3/4   1   1/4  2/4  3/4   1 -- value of u
            0    0    0    0    0    1    1    1    1 -- surface column index
            0    1    2    3    4    5    6    7    8 -- grid column index
  0    0  0 *----*----*----*----o----*----*----*----*
            |                   |                   |
  1/4  0  1 *    *    *    *    o    *    *    *    *
            |       A           |       B           |
  2/4  0  2 *    *    *    *    o    *    *    *    *
            |                   |                   |
  3/4  0  3 *    *    *    *    o    *    *    *    *
            |                   |                   |
  1    0  4 o----o----o----o----o----o----o----o----o
            |                   |                   |
  1/4  1  5 *    *    *    *    o    *    *    *    *
            |       C           |       D           |
  2/4  1  6 *    *    *    *    o    *    *    *    *
            |                   |                   |
  3/4  1  7 *    *    *    *    o    *    *    *    *
            |                   |                   |
  1    1  8 *----*----*----*----o----*----*----*----*
  |    |  |
  |    |  grid row index
  |    |
  |    surface row index
  |
  value of v
  */

  for (size_t gridRow = 0u; gridRow < gridPointRowCount; ++gridRow)
  {
    const size_t surfaceRow =
      (gridRow > 0u ? gridRow - 1u : gridRow) / quadsPerSurfaceSide;
    const FloatType v = static_cast<FloatType>(gridRow - surfaceRow * quadsPerSurfaceSide)
                        / static_cast<FloatType>(quadsPerSurfaceSide);

    for (size_t gridCol = 0u; gridCol < gridPointColumnCount; ++gridCol)
    {
      const size_t surfaceCol =
        (gridCol > 0u ? gridCol - 1u : gridCol) / quadsPerSurfaceSide;
      const FloatType u =
        static_cast<FloatType>(gridCol - surfaceCol * quadsPerSurfaceSide)
        / static_cast<FloatType>(quadsPerSurfaceSide);

      const auto& surfaceControlPoints =
        allSurfaceControlPoints[surfaceRow * surfaceColumnCount() + surfaceCol];
      auto point = vm::evaluate_quadratic_bezier_surface(surfaceControlPoints, u, v);
      grid.push_back(std::move(point));
    }
  }

  return grid;
}

} // namespace TrenchBroom::Model
