/*
 Copyright (C) 2010 Kristian Duske

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

#include "Polyhedron.h"

#include "kd/contracts.h"

#include "vm/constants.h"
#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/segment.h"
#include "vm/util.h"

namespace tb::mdl
{

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::contains(const vm::vec<T, 3>& point, const T epsilon) const
{
  if (!polyhedron())
  {
    return false;
  }

  if (!bounds().contains(point))
  {
    return false;
  }

  for (const auto* face : m_faces)
  {
    const auto& plane = face->plane();
    if (plane.point_status(point, epsilon) == vm::plane_status::above)
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::contains(const Polyhedron& other) const
{
  if (!polyhedron())
  {
    return false;
  }

  if (!bounds().contains(other.bounds()))
  {
    return false;
  }

  for (const auto* vertex : other.vertices())
  {
    if (!contains(vertex->position(), vm::constants<T>::point_status_epsilon()))
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::intersects(const Polyhedron& other) const
{
  if (!bounds().intersects(other.bounds()))
  {
    return false;
  }

  if (empty() || other.empty())
  {
    return false;
  }

  if (point())
  {
    return other.point()     ? pointIntersectsPoint(*this, other)
           : other.edge()    ? pointIntersectsEdge(*this, other)
           : other.polygon() ? pointIntersectsPolygon(*this, other)
                             : pointIntersectsPolyhedron(*this, other);
  }
  if (edge())
  {
    return other.point()     ? edgeIntersectsPoint(*this, other)
           : other.edge()    ? edgeIntersectsEdge(*this, other)
           : other.polygon() ? edgeIntersectsPolygon(*this, other)
                             : edgeIntersectsPolyhedron(*this, other);
  }
  if (polygon())
  {
    return other.point()     ? polygonIntersectsPoint(*this, other)
           : other.edge()    ? polygonIntersectsEdge(*this, other)
           : other.polygon() ? polygonIntersectsPolygon(*this, other)
                             : polygonIntersectsPolyhedron(*this, other);
  }
  else
  {
    return other.point()     ? polyhedronIntersectsPoint(*this, other)
           : other.edge()    ? polyhedronIntersectsEdge(*this, other)
           : other.polygon() ? polyhedronIntersectsPolygon(*this, other)
                             : polyhedronIntersectsPolyhedron(*this, other);
  }
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::pointIntersectsPoint(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.point());
  contract_pre(rhs.point());

  const auto& lhsPos = lhs.m_vertices.front()->position();
  const auto& rhsPos = rhs.m_vertices.front()->position();
  return lhsPos == rhsPos;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::pointIntersectsEdge(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.point());
  contract_pre(rhs.edge());

  const auto& lhsPos = lhs.m_vertices.front()->position();
  const auto* rhsEdge = rhs.m_edges.front();
  const auto& rhsStart = rhsEdge->firstVertex()->position();
  const auto& rhsEnd = rhsEdge->secondVertex()->position();

  return vm::segment<T, 3>{rhsStart, rhsEnd}.contains(
    lhsPos, vm::constants<T>::almost_zero());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::pointIntersectsPolygon(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.point());
  contract_pre(rhs.polygon());

  const auto& lhsPos = lhs.m_vertices.front()->position();
  const auto* rhsFace = rhs.m_faces.front();
  const auto& rhsNormal = rhsFace->plane().normal;
  const auto& rhsBoundary = rhsFace->boundary();

  return vm::polygon_contains_point(
    lhsPos, rhsNormal, rhsBoundary.begin(), rhsBoundary.end(), GetVertexPosition{});
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::pointIntersectsPolyhedron(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.point());
  contract_pre(rhs.polyhedron());

  const auto& lhsPos = lhs.m_vertices.front()->position();
  return rhs.contains(lhsPos, vm::constants<T>::point_status_epsilon());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::edgeIntersectsPoint(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  return pointIntersectsEdge(rhs, lhs);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::edgeIntersectsEdge(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.edge());
  contract_pre(rhs.edge());

  const auto* lhsEdge = lhs.m_edges.front();
  const auto& lhsStart = lhsEdge->firstVertex()->position();
  const auto& lhsEnd = lhsEdge->secondVertex()->position();

  const auto* rhsEdge = rhs.m_edges.front();
  if (rhsEdge->hasPosition(lhsStart) || rhsEdge->hasPosition(lhsEnd))
  {
    return true;
  }

  const auto lhsRay = vm::ray<T, 3>{lhsStart, vm::normalize(lhsEnd - lhsStart)};
  const auto dist = vm::squared_distance(lhsRay, rhsEdge->segment());
  const auto rayLen = vm::distance_to_projected_point(lhsRay, lhsEnd);

  if (dist.parallel)
  {
    if (dist.is_colinear())
    {
      const auto& rhsStart = rhsEdge->firstVertex()->position();
      const auto& rhsEnd = rhsEdge->secondVertex()->position();

      const auto rhsStartDist = vm::distance_to_projected_point(lhsRay, rhsStart);
      const auto rhsEndDist = vm::distance_to_projected_point(lhsRay, rhsEnd);

      return (
        vm::contains(rhsStartDist, 0.0, rayLen) ||   // lhs constains rhs start
        vm::contains(rhsEndDist, 0.0, rayLen) ||     // lhs contains rhs end
        (rhsStartDist > 0.0) != (rhsEndDist > 0.0)); // rhs contains lhs
    }
    return false;
  }

  constexpr auto epsilon2 =
    vm::constants<T>::almost_zero() * vm::constants<T>::almost_zero();
  return dist.distance < epsilon2 && dist.position1 <= rayLen;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::edgeIntersectsPolygon(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.edge());
  contract_pre(rhs.polygon());

  const auto* lhsEdge = lhs.m_edges.front();
  const auto* rhsFace = rhs.m_faces.front();

  return edgeIntersectsFace(lhsEdge, rhsFace);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::edgeIntersectsPolyhedron(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.edge());
  contract_pre(rhs.polyhedron());

  const auto* lhsEdge = lhs.m_edges.front();
  const auto& lhsStart = lhsEdge->firstVertex()->position();
  const auto& lhsEnd = lhsEdge->secondVertex()->position();

  const auto lhsRay = vm::ray<T, 3>{lhsStart, normalize(lhsEnd - lhsStart)};
  const auto rayLen = vm::dot(lhsEnd - lhsStart, lhsRay.direction);

  auto frontHit = false;
  auto backHit = false;

  for (const auto* rhsFace : rhs.faces())
  {
    if (const auto result = rhsFace->intersectWithRay(lhsRay))
    {
      if (result->front())
      {
        if (result->distance() <= rayLen)
        {
          return true;
        }
        frontHit = true;
      }
      else if (result->back())
      {
        if (result->distance() <= rayLen)
        {
          return true;
        }
        backHit = true;
      }
    }
  }

  return backHit && !frontHit;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::edgeIntersectsFace(const Edge* lhsEdge, const Face* rhsFace)
{
  const auto& lhsStart = lhsEdge->firstVertex()->position();
  const auto& lhsEnd = lhsEdge->secondVertex()->position();
  const auto lhsRay = vm::ray<T, 3>{lhsStart, normalize(lhsEnd - lhsStart)};

  if (const auto dist = rhsFace->intersectWithRay(lhsRay, vm::side::both))
  {
    const auto rayLen = vm::dot(lhsEnd - lhsStart, lhsRay.direction);
    return dist <= rayLen;
  }

  const auto& edgeDir = lhsRay.direction;
  const auto faceNorm = rhsFace->normal();
  if (vm::is_zero(vm::dot(faceNorm, edgeDir), vm::constants<T>::almost_zero()))
  {
    // ray and face are parallel, intersect with edges
    constexpr auto MaxDistance =
      vm::constants<T>::almost_zero() * vm::constants<T>::almost_zero();

    for (const auto* rhsEdge : rhsFace->boundary())
    {
      const auto& start = rhsEdge->origin()->position();
      const auto& end = rhsEdge->destination()->position();
      if (vm::distance(lhsRay, vm::segment<T, 3>{start, end}).distance <= MaxDistance)
      {
        return true;
      }
    }
  }
  return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polygonIntersectsPoint(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  return pointIntersectsPolygon(rhs, lhs);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polygonIntersectsEdge(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  return edgeIntersectsPolygon(rhs, lhs);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polygonIntersectsPolygon(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.polygon());
  contract_pre(rhs.polygon());

  auto* lhsFace = lhs.faces().front();
  auto* rhsFace = rhs.faces().front();

  return faceIntersectsFace(lhsFace, rhsFace);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polygonIntersectsPolyhedron(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.polygon());
  contract_pre(rhs.polyhedron());

  auto* lhsFace = lhs.faces().front();
  for (const auto* rhsFace : rhs.faces())
  {
    if (faceIntersectsFace(lhsFace, rhsFace))
    {
      return true;
    }
  }

  auto* vertex = lhs.vertices().front();
  return rhs.contains(vertex->position(), vm::constants<T>::point_status_epsilon());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::faceIntersectsFace(const Face* lhsFace, const Face* rhsFace)
{
  const auto& lhsBoundary = lhsFace->boundary();
  const auto& rhsBoundary = rhsFace->boundary();

  for (const auto* lhsEdge : lhsBoundary)
  {
    if (edgeIntersectsFace(lhsEdge->edge(), rhsFace))
    {
      return true;
    }
  }

  const auto* lhsVertex = lhsBoundary.front()->origin();
  const auto* rhsVertex = rhsBoundary.front()->origin();

  return (
    vm::polygon_contains_point(
      lhsVertex->position(), rhsBoundary.begin(), rhsBoundary.end(), GetVertexPosition{})
    || vm::polygon_contains_point(
      rhsVertex->position(),
      lhsBoundary.begin(),
      lhsBoundary.end(),
      GetVertexPosition{}));
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polyhedronIntersectsPoint(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  return pointIntersectsPolyhedron(rhs, lhs);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polyhedronIntersectsEdge(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  return edgeIntersectsPolyhedron(rhs, lhs);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polyhedronIntersectsPolygon(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  return polygonIntersectsPolyhedron(rhs, lhs);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polyhedronIntersectsPolyhedron(
  const Polyhedron& lhs, const Polyhedron& rhs)
{
  contract_pre(lhs.polyhedron());
  contract_pre(rhs.polyhedron());

  // separating axis theorem
  // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf

  if (separate(lhs.m_faces, rhs.vertices()))
  {
    return false;
  }
  if (separate(rhs.faces(), lhs.m_vertices))
  {
    return false;
  }

  for (const auto* lhsEdge : lhs.edges())
  {
    const auto lhsEdgeVec = lhsEdge->vector();
    const auto& lhsEdgeOrigin = lhsEdge->firstVertex()->position();

    for (const auto* rhsEdge : rhs.edges())
    {
      const auto rhsEdgeVec = rhsEdge->vector();
      const auto direction = vm::cross(lhsEdgeVec, rhsEdgeVec);

      if (!vm::is_zero(direction, vm::constants<T>::almost_zero()))
      {
        const auto plane = vm::plane<T, 3>(lhsEdgeOrigin, direction);

        const auto lhsStatus = pointStatus(plane, lhs.vertices());
        if (lhsStatus != vm::plane_status::inside)
        {
          const auto rhsStatus = pointStatus(plane, rhs.vertices());
          if (rhsStatus != vm::plane_status::inside)
          {
            if (lhsStatus != rhsStatus)
            {
              return false;
            }
          }
        }
      }
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::separate(const FaceList& faces, const VertexList& vertices)
{
  for (const auto* face : faces)
  {
    const auto& plane = face->plane();
    if (pointStatus(plane, vertices) == vm::plane_status::above)
    {
      return true;
    }
  }

  return false;
}

template <typename T, typename FP, typename VP>
vm::plane_status Polyhedron<T, FP, VP>::pointStatus(
  const vm::plane<T, 3>& plane, const VertexList& vertices)
{
  std::size_t above = 0u;
  std::size_t below = 0u;

  for (const auto* vertex : vertices)
  {
    const auto status = plane.point_status(vertex->position());
    if (status == vm::plane_status::above)
    {
      ++above;
    }
    else if (status == vm::plane_status::below)
    {
      ++below;
    }
    if (above > 0u && below > 0u)
    {
      return vm::plane_status::inside;
    }
  }
  return above > 0u ? vm::plane_status::above : vm::plane_status::below;
}

} // namespace tb::mdl
