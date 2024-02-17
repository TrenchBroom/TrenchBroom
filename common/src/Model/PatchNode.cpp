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

#include "PatchNode.h"

#include "Macros.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/LayerNode.h"
#include "Model/LinkedGroupUtils.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"
#include "Model/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"
#include "kdl/zip_iterator.h"

#include "vm/bbox_io.h"
#include "vm/intersection.h"
#include "vm/vec_io.h"

#include <cassert>
#include <ostream>
#include <string>

namespace TrenchBroom
{
namespace Model
{
constexpr static size_t DefaultSubdivisionsPerSurface = 3u;

kdl_reflect_impl(PatchGrid::Point);

const PatchGrid::Point& PatchGrid::point(const size_t row, const size_t col) const
{
  const auto index = row * pointColumnCount + col;
  assert(index < points.size());
  return points[index];
}

size_t PatchGrid::quadRowCount() const
{
  return pointRowCount - 1u;
}

size_t PatchGrid::quadColumnCount() const
{
  return pointColumnCount - 1u;
}

kdl_reflect_impl(PatchGrid);

/**
 * Compute the normals for the given patch grid points.
 *
 * The normals are computed by averaging the normals of the quadrants incident to each
 * point.
 *
 *      *
 *    A | B
 *  *---*---* row
 *    C | D
 *      *
 *     col
 *
 * For a point at row, col, we compute the normals of the incident quadrants A, B, C, D,
 * take their average, and normalize the result. Not every grid point has four incident
 * quadrants (e.g. the corner points have only one). If the grid points of two opposing
 * sides of the grid coincide, we treat them as one grid point and average their normals.
 */
std::vector<vm::vec3> computeGridNormals(
  const std::vector<BezierPatch::Point> patchGrid,
  const size_t pointRowCount,
  const size_t pointColumnCount)
{
  /* Returns the index of a grid point with the given coordinates. */
  const auto index = [&](const size_t row, const size_t col) {
    return row * pointColumnCount + col;
  };

  /* Returns the grid point with the given coordinates. */
  const auto gridPoint = [&](const size_t row, const size_t col) {
    return patchGrid[index(row, col)].xyz();
  };

  enum class RowOffset
  {
    Above,
    Below
  };
  enum class ColOffset
  {
    Left,
    Right
  };

  /* Returns the of the quadrant next to the grid point with the given coordinates.
   *
   *      *
   *    A | B
   *  *---*---* row
   *    C | D
   *      *
   *     col
   *
   * Quadrant A is above and left of the grid point at row, col, quadrant B is above and
   * right of the grid point, and so on. We determine the incident grid points necessary
   * to compute the normals (via cross product). The returned normal is not normalized.
   */
  const auto normalForQuadrant = [&](
                                   const size_t row,
                                   const size_t col,
                                   const RowOffset rowOffset,
                                   const ColOffset colOffset) {
    const auto point = gridPoint(row, col);
    switch (rowOffset)
    {
    case RowOffset::Above: {
      assert(row > 0u);
      const auto above = gridPoint(row - 1u, col);
      switch (colOffset)
      {
      case ColOffset::Left: {
        assert(col > 0u);
        const auto left = gridPoint(row, col - 1u);
        return vm::cross(above - point, left - point);
      }
      case ColOffset::Right: {
        assert(col < pointColumnCount - 1u);
        const auto right = gridPoint(row, col + 1u);
        return vm::cross(right - point, above - point);
      }
        switchDefault();
      }
    }
    case RowOffset::Below: {
      assert(row < pointRowCount - 1u);
      const auto below = gridPoint(row + 1u, col);
      switch (colOffset)
      {
      case ColOffset::Left: {
        assert(col > 0u);
        const auto left = gridPoint(row, col - 1u);
        return vm::cross(left - point, below - point);
      }
      case ColOffset::Right: {
        assert(col < pointColumnCount - 1u);
        const auto right = gridPoint(row, col + 1u);
        return vm::cross(below - point, right - point);
      }
        switchDefault();
      }
    }
      switchDefault();
    };
  };

  const size_t t = 0u;                    // top row
  const size_t b = pointRowCount - 1u;    // bottom row
  const size_t l = 0u;                    // left column
  const size_t r = pointColumnCount - 1u; // right column

  auto normals = std::vector<vm::vec3>{};
  normals.resize(patchGrid.size());

  // corner normals
  normals[index(t, l)] = normalForQuadrant(t, l, RowOffset::Below, ColOffset::Right);
  normals[index(t, r)] = normalForQuadrant(t, r, RowOffset::Below, ColOffset::Left);
  normals[index(b, l)] = normalForQuadrant(b, l, RowOffset::Above, ColOffset::Right);
  normals[index(b, r)] = normalForQuadrant(b, r, RowOffset::Above, ColOffset::Left);

  // top and bottom row normals, excluding corners
  for (size_t col = 1u; col < r; ++col)
  {
    normals[index(t, col)] =
      (normalForQuadrant(t, col, RowOffset::Below, ColOffset::Left)
       + normalForQuadrant(t, col, RowOffset::Below, ColOffset::Right))
      / static_cast<FloatType>(2);
    normals[index(b, col)] =
      (normalForQuadrant(b, col, RowOffset::Above, ColOffset::Left)
       + normalForQuadrant(b, col, RowOffset::Above, ColOffset::Right))
      / static_cast<FloatType>(2);
  }

  // left and right column normals, excluding corners
  for (size_t row = 1u; row < b; ++row)
  {
    normals[index(row, l)] =
      (normalForQuadrant(row, l, RowOffset::Above, ColOffset::Right)
       + normalForQuadrant(row, l, RowOffset::Below, ColOffset::Right))
      / static_cast<FloatType>(2);
    normals[index(row, r)] =
      (normalForQuadrant(row, r, RowOffset::Above, ColOffset::Left)
       + normalForQuadrant(row, r, RowOffset::Below, ColOffset::Left))
      / static_cast<FloatType>(2);
  }

  // inner point normals
  for (size_t row = 1u; row < b; ++row)
  {
    for (size_t col = 1u; col < r; ++col)
    {
      normals[index(row, col)] =
        (normalForQuadrant(row, col, RowOffset::Above, ColOffset::Left)
         + normalForQuadrant(row, col, RowOffset::Above, ColOffset::Right)
         + normalForQuadrant(row, col, RowOffset::Below, ColOffset::Left)
         + normalForQuadrant(row, col, RowOffset::Below, ColOffset::Right))
        / static_cast<FloatType>(4);
    }
  }

  // Two grid points on opposing sides of the grid coincide if their distance is less than
  // this. This is from Q3 Radiant's source code.
  constexpr auto GridPointEpsilon = static_cast<FloatType>(1);

  // check opposing sides of the grid, if their corresponding points coincide, combine the
  // normals
  for (size_t row = 0u; row < pointRowCount; ++row)
  {
    if (
      vm::squared_distance(gridPoint(row, l), gridPoint(row, r))
      < GridPointEpsilon * GridPointEpsilon)
    {
      const auto combinedNormal =
        (normals[index(row, l)] + normals[index(row, r)]) / static_cast<FloatType>(2);
      normals[index(row, l)] = normals[index(row, r)] = combinedNormal;
    }
  }
  for (size_t col = 0u; col < pointColumnCount; ++col)
  {
    if (
      vm::squared_distance(gridPoint(t, col), gridPoint(b, col))
      < GridPointEpsilon * GridPointEpsilon)
    {
      const auto combinedNormal =
        (normals[index(t, col)] + normals[index(b, col)]) / static_cast<FloatType>(2);
      normals[index(t, col)] = normals[index(b, col)] = combinedNormal;
    }
  }

  // normalize
  for (auto& normal : normals)
  {
    normal = vm::normalize(normal);
  }

  return normals;
}

PatchGrid makePatchGrid(const BezierPatch& patch, const size_t subdivisionsPerSurface)
{
  const size_t gridPointRowCount =
    patch.surfaceRowCount() * (size_t(1) << subdivisionsPerSurface) + 1u;
  const size_t gridPointColumnCount =
    patch.surfaceColumnCount() * (size_t(1) << subdivisionsPerSurface) + 1u;

  const auto patchGrid = patch.evaluate(subdivisionsPerSurface);
  const auto normals =
    computeGridNormals(patchGrid, gridPointRowCount, gridPointColumnCount);
  assert(patchGrid.size() == normals.size());

  auto points = std::vector<PatchGrid::Point>{};
  auto boundsBuilder = vm::bbox3::builder{};
  for (const auto [point, normal] : kdl::make_zip_range(patchGrid, normals))
  {
    const auto position = vm::slice<3>(point, 0);
    const auto texCoords = vm::slice<2>(point, 3);
    points.push_back(PatchGrid::Point{position, texCoords, normal});
    boundsBuilder.add(position);
  }

  return {
    gridPointRowCount, gridPointColumnCount, std::move(points), boundsBuilder.bounds()};
}

const HitType::Type PatchNode::PatchHitType = HitType::freeType();

PatchNode::PatchNode(BezierPatch patch)
  : m_patch{std::move(patch)}
  , m_grid{makePatchGrid(m_patch, DefaultSubdivisionsPerSurface)}
{
}

const EntityNodeBase* PatchNode::entity() const
{
  return visitParent(
           kdl::overload(
             [](const WorldNode* world) -> const EntityNodeBase* { return world; },
             [](const EntityNode* entity) -> const EntityNodeBase* { return entity; },
             [](auto&& thisLambda, const LayerNode* layer) -> const EntityNodeBase* {
               return layer->visitParent(thisLambda).value_or(nullptr);
             },
             [](auto&& thisLambda, const GroupNode* group) -> const EntityNodeBase* {
               return group->visitParent(thisLambda).value_or(nullptr);
             },
             [](auto&& thisLambda, const BrushNode* brush) -> const EntityNodeBase* {
               return brush->visitParent(thisLambda).value_or(nullptr);
             },
             [](auto&& thisLambda, const PatchNode* patch) -> const EntityNodeBase* {
               return patch->visitParent(thisLambda).value_or(nullptr);
             }))
    .value_or(nullptr);
}

EntityNodeBase* PatchNode::entity()
{
  return const_cast<EntityNodeBase*>(const_cast<const PatchNode*>(this)->entity());
}

const BezierPatch& PatchNode::patch() const
{
  return m_patch;
}

BezierPatch PatchNode::setPatch(BezierPatch patch)
{
  const auto nodeChange = NotifyNodeChange{*this};
  const auto boundsChange = NotifyPhysicalBoundsChange{*this};

  auto previousPatch = std::exchange(m_patch, std::move(patch));
  m_grid = makePatchGrid(m_patch, DefaultSubdivisionsPerSurface);
  return previousPatch;
}

void PatchNode::setTexture(Assets::Texture* texture)
{
  m_patch.setTexture(texture);
}

const PatchGrid& PatchNode::grid() const
{
  return m_grid;
}

const std::string& PatchNode::doGetName() const
{
  static const auto name = std::string{"patch"};
  return name;
}

const vm::bbox3& PatchNode::doGetLogicalBounds() const
{
  return m_patch.bounds();
}

const vm::bbox3& PatchNode::doGetPhysicalBounds() const
{
  return m_grid.bounds;
}

FloatType PatchNode::doGetProjectedArea(const vm::axis::type axis) const
{
  // computing the projected area of a patch is expensive, so we just use the bounds
  const vm::vec3 size = physicalBounds().size();
  switch (axis)
  {
  case vm::axis::x:
    return size.y() * size.z();
  case vm::axis::y:
    return size.x() * size.z();
  case vm::axis::z:
    return size.x() * size.y();
  default:
    return 0.0;
  }
}

Node* PatchNode::doClone(const vm::bbox3&, const SetLinkId setLinkIds) const
{
  auto result = std::make_unique<PatchNode>(m_patch);
  result->cloneLinkId(*this, setLinkIds);
  return result.release();
}

bool PatchNode::doCanAddChild(const Node*) const
{
  return false;
}

bool PatchNode::doCanRemoveChild(const Node*) const
{
  return false;
}

bool PatchNode::doRemoveIfEmpty() const
{
  return false;
}

bool PatchNode::doShouldAddToSpacialIndex() const
{
  return true;
}

bool PatchNode::doSelectable() const
{
  return true;
}

void PatchNode::doPick(
  const EditorContext& editorContext, const vm::ray3& pickRay, PickResult& pickResult)
{
  if (!editorContext.visible(this))
  {
    return;
  }
  const auto pickTriangle = [&](const auto& p0, const auto& p1, const auto& p2) {
    if (const auto distance = vm::intersect_ray_triangle(pickRay, p0, p1, p2);
        !vm::is_nan(distance))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, distance);
      pickResult.addHit(Hit(PatchHitType, distance, hitPoint, this));
      return true;
    }
    return false;
  };

  for (size_t row = 0u; row < m_grid.pointRowCount - 1u; ++row)
  {
    for (size_t col = 0u; col < m_grid.pointColumnCount - 1u; ++col)
    {
      const auto v0 = m_grid.point(row, col).position;
      const auto v1 = m_grid.point(row, col + 1u).position;
      const auto v2 = m_grid.point(row + 1u, col + 1u).position;
      const auto v3 = m_grid.point(row + 1u, col).position;

      if (pickTriangle(v0, v1, v2) || pickTriangle(v2, v3, v0))
      {
        return;
      }
    }
  }
}

void PatchNode::doFindNodesContaining(const vm::vec3&, std::vector<Node*>&) {}

void PatchNode::doAccept(NodeVisitor& visitor)
{
  visitor.visit(this);
}

void PatchNode::doAccept(ConstNodeVisitor& visitor) const
{
  visitor.visit(this);
}

Node* PatchNode::doGetContainer()
{
  return parent();
}

LayerNode* PatchNode::doGetContainingLayer()
{
  return findContainingLayer(this);
}

GroupNode* PatchNode::doGetContainingGroup()
{
  return findContainingGroup(this);
}

void PatchNode::doAcceptTagVisitor(TagVisitor& visitor)
{
  visitor.visit(*this);
}

void PatchNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const
{
  visitor.visit(*this);
}
} // namespace Model
} // namespace TrenchBroom
