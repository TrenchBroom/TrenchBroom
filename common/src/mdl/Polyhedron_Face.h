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

#include "Macros.h"
#include "Polyhedron.h"

#include "kdl/optional_utils.h"

#include "vm/constants.h"
#include "vm/intersection.h"
#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/scalar.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <unordered_set>

namespace tb::mdl
{

template <typename T, typename FP, typename VP>
kdl::intrusive_circular_link<Polyhedron_Face<T, FP, VP>>& Polyhedron_GetFaceLink<
  T,
  FP,
  VP>::operator()(Polyhedron_Face<T, FP, VP>* face) const
{
  return face->m_link;
}

template <typename T, typename FP, typename VP>
const kdl::intrusive_circular_link<Polyhedron_Face<T, FP, VP>>& Polyhedron_GetFaceLink<
  T,
  FP,
  VP>::operator()(const Polyhedron_Face<T, FP, VP>* face) const
{
  return face->m_link;
}

template <typename T, typename FP, typename VP>
Polyhedron_Face<T, FP, VP>::Polyhedron_Face(
  HalfEdgeList&& boundary, const vm::plane<T, 3>& plane)
  : m_boundary{std::move(boundary)}
  , m_plane{plane}
  , m_payload{FP::defaultValue()}
  ,
#ifdef _MSC_VER
// MSVC throws a warning because we're passing this to the FaceLink constructor, but it's
// okay because we just store the pointer there.
#pragma warning(push)
#pragma warning(disable : 4355)
  m_link{this}
#pragma warning(pop)
#else
  m_link{this}
#endif
{
  assert(m_boundary.size() >= 3);
  countAndSetFace(m_boundary.front(), m_boundary.back(), this);
}

template <typename T, typename FP, typename VP>
const typename Polyhedron_Face<T, FP, VP>::HalfEdgeList& Polyhedron_Face<T, FP, VP>::
  boundary() const
{
  return m_boundary;
}

template <typename T, typename FP, typename VP>
typename Polyhedron_Face<T, FP, VP>::HalfEdgeList& Polyhedron_Face<T, FP, VP>::boundary()
{
  return m_boundary;
}

template <typename T, typename FP, typename VP>
const vm::plane<T, 3>& Polyhedron_Face<T, FP, VP>::plane() const
{
  return m_plane;
}

template <typename T, typename FP, typename VP>
void Polyhedron_Face<T, FP, VP>::setPlane(const vm::plane<T, 3>& plane)
{
  m_plane = plane;
}

template <typename T, typename FP, typename VP>
typename Polyhedron_Face<T, FP, VP>::Face* Polyhedron_Face<T, FP, VP>::next() const
{
  return m_link.next();
}

template <typename T, typename FP, typename VP>
typename Polyhedron_Face<T, FP, VP>::Face* Polyhedron_Face<T, FP, VP>::previous() const
{
  return m_link.previous();
}

template <typename T, typename FP, typename VP>
typename FP::Type Polyhedron_Face<T, FP, VP>::payload() const
{
  return m_payload;
}

template <typename T, typename FP, typename VP>
void Polyhedron_Face<T, FP, VP>::setPayload(typename FP::Type payload)
{
  m_payload = payload;
}

template <typename T, typename FP, typename VP>
std::size_t Polyhedron_Face<T, FP, VP>::vertexCount() const
{
  return m_boundary.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron_Face<T, FP, VP>::HalfEdge* Polyhedron_Face<T, FP, VP>::
  findHalfEdge(const vm::vec<T, 3>& origin, const T epsilon) const
{
  for (const auto* halfEdge : m_boundary)
  {
    if (vm::is_equal(halfEdge->origin()->position(), origin, epsilon))
    {
      return halfEdge;
    }
  }
  return nullptr;
}

template <typename T, typename FP, typename VP>
const typename Polyhedron_Face<T, FP, VP>::Edge* Polyhedron_Face<T, FP, VP>::findEdge(
  const vm::vec<T, 3>& first, const vm::vec<T, 3>& second, const T epsilon) const
{
  if (const auto* halfEdge = findHalfEdge(first, epsilon))
  {
    if (vm::is_equal(halfEdge->destination()->position(), second, epsilon))
    {
      return halfEdge->edge();
    }

    halfEdge = halfEdge->previous();
    if (vm::is_equal(halfEdge->origin()->position(), second, epsilon))
    {
      return halfEdge->edge();
    }
  }

  return nullptr;
}

template <typename T, typename FP, typename VP>
vm::vec<T, 3> Polyhedron_Face<T, FP, VP>::origin() const
{
  const auto* edge = m_boundary.front();
  return edge->origin()->position();
}

template <typename T, typename FP, typename VP>
std::vector<vm::vec<T, 3>> Polyhedron_Face<T, FP, VP>::vertexPositions() const
{
  auto result = std::vector<vm::vec<T, 3>>{};
  result.reserve(vertexCount());
  for (const auto* halfEdge : m_boundary)
  {
    result.push_back(halfEdge->origin()->position());
  }
  return result;
}

template <typename T, typename FP, typename VP>
bool Polyhedron_Face<T, FP, VP>::hasVertexPosition(
  const vm::vec<T, 3>& position, const T epsilon) const
{
  for (const auto* halfEdge : m_boundary)
  {
    if (vm::is_equal(halfEdge->origin()->position(), position, epsilon))
    {
      return true;
    }
  }
  return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron_Face<T, FP, VP>::hasVertexPositions(
  const std::vector<vm::vec<T, 3>>& positions, const T epsilon) const
{
  if (positions.size() != vertexCount())
  {
    return false;
  }

  for (const auto* halfEdge : m_boundary)
  {
    if (halfEdge->hasOrigins(positions, epsilon))
    {
      return true;
    }
  }

  return false;
}

template <typename T, typename FP, typename VP>
T Polyhedron_Face<T, FP, VP>::distanceTo(
  const std::vector<vm::vec<T, 3>>& positions, const T maxDistance) const
{
  if (positions.size() != vertexCount())
  {
    return maxDistance;
  }

  auto closestDistance = maxDistance;

  // Find the boundary edge with the origin closest to the first position.
  const HalfEdge* startEdge = nullptr;
  for (const auto* halfEdge : m_boundary)
  {
    const auto currentDistance =
      vm::distance(halfEdge->origin()->position(), positions.front());
    if (currentDistance < closestDistance)
    {
      closestDistance = currentDistance;
      startEdge = halfEdge;
    }
  }

  // No vertex is within maxDistance of the first of the given positions.
  if (!startEdge)
  {
    return maxDistance;
  }

  // now find the maximum distance of all points
  const auto* firstEdge = startEdge;
  const auto* currentEdge = firstEdge->next();
  auto posIt = std::next(std::begin(positions));
  do
  {
    const auto& position = *posIt;
    ++posIt;

    closestDistance =
      vm::max(closestDistance, vm::distance(currentEdge->origin()->position(), position));
    currentEdge = currentEdge->next();
  } while (currentEdge != firstEdge);
  return closestDistance;
}

template <typename T, typename FP, typename VP>
vm::vec<T, 3> Polyhedron_Face<T, FP, VP>::normal() const
{
  for (const auto* halfEdge : m_boundary)
  {
    const auto& p1 = halfEdge->origin()->position();
    const auto& p2 = halfEdge->next()->origin()->position();
    const auto& p3 = halfEdge->next()->next()->origin()->position();
    const auto normal = vm::cross(p2 - p1, p3 - p1);
    if (!vm::is_zero(normal, vm::constants<T>::almost_zero()))
    {
      return vm::normalize(normal);
    }
  }

  return vm::vec<T, 3>{0, 0, 0};
}

template <typename T, typename FP, typename VP>
vm::vec<T, 3> Polyhedron_Face<T, FP, VP>::center() const
{
  return vm::average(m_boundary.begin(), m_boundary.end(), [](const auto* e) {
    return e->origin()->position();
  });
}

template <typename T, typename FP, typename VP>
std::optional<T> Polyhedron_Face<T, FP, VP>::intersectWithRay(
  const vm::ray<T, 3>& ray, const vm::side side) const
{
  return intersectWithRay(ray) | kdl::optional_transform([&](const auto result) {
           switch (side)
           {
           case vm::side::front:
             return result.front() ? std::optional{result.distance()} : std::nullopt;
           case vm::side::back:
             return result.back() ? std::optional{result.distance()} : std::nullopt;
           case vm::side::both:
             return std::optional{result.distance()};
             switchDefault();
           }
         });
}

template <typename T, typename FP, typename VP>
vm::plane_status Polyhedron_Face<T, FP, VP>::pointStatus(
  const vm::vec<T, 3>& point, const T epsilon) const
{
  const auto norm = normal();
  const auto distance = vm::dot(point - origin(), norm);
  return distance > epsilon    ? vm::plane_status::above
         : distance < -epsilon ? vm::plane_status::below
                               : vm::plane_status::inside;
}

template <typename T, typename FP, typename VP>
bool Polyhedron_Face<T, FP, VP>::coplanar(const Face* other, const T epsilon) const
{
  assert(other != nullptr);

  // Test if the normals are colinear by checking their enclosed angle.
  if (T(1) - vm::dot(normal(), other->normal()) >= vm::constants<T>::colinear_epsilon())
  {
    return false;
  }

  const auto myPlane =
    vm::plane<T, 3>{m_boundary.front()->origin()->position(), normal()};
  if (!other->verticesOnPlane(myPlane, epsilon))
  {
    return false;
  }

  const auto otherPlane =
    vm::plane<T, 3>{other->boundary().front()->origin()->position(), other->normal()};
  return verticesOnPlane(otherPlane, epsilon);
}

template <typename T, typename FP, typename VP>
bool Polyhedron_Face<T, FP, VP>::verticesOnPlane(
  const vm::plane<T, 3>& plane, const T epsilon) const
{
  for (const auto* halfEdge : m_boundary)
  {
    const auto* vertex = halfEdge->origin();
    if (plane.point_status(vertex->position(), epsilon) != vm::plane_status::inside)
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
T Polyhedron_Face<T, FP, VP>::maximumVertexDistance(const vm::plane<T, 3>& plane) const
{
  auto maximumDistance = T(0);
  for (const auto* halfEdge : m_boundary)
  {
    const auto* vertex = halfEdge->origin();
    maximumDistance = vm::max(plane.point_distance(vertex->position()), maximumDistance);
  }

  return maximumDistance;
}

template <typename T, typename FP, typename VP>
void Polyhedron_Face<T, FP, VP>::flip()
{
  m_boundary.reverse();
  m_plane = m_plane.flip();
}

template <typename T, typename FP, typename VP>
template <typename H>
void Polyhedron_Face<T, FP, VP>::insertIntoBoundaryAfter(HalfEdge* after, H&& edges)
{
  assert(after != nullptr);
  assert(after->face() == this);

  countAndSetFace(edges.front(), edges.back(), this);
  m_boundary.insert(HalfEdgeList::iter(after->next()), std::forward<H>(edges));
}

template <typename T, typename FP, typename VP>
typename Polyhedron_Face<T, FP, VP>::HalfEdgeList Polyhedron_Face<T, FP, VP>::
  removeFromBoundary(HalfEdge* from, HalfEdge* to)
{
  assert(from != nullptr);
  assert(to != nullptr);
  assert(from->face() == this);
  assert(to->face() == this);

  const auto removeCount = countAndUnsetFace(from, to);
  return m_boundary.remove(
    HalfEdgeList::iter(from), std::next(HalfEdgeList::iter(to)), removeCount);
}

template <typename T, typename FP, typename VP>
typename Polyhedron_Face<T, FP, VP>::HalfEdgeList Polyhedron_Face<T, FP, VP>::
  removeFromBoundary(HalfEdge* edge)
{
  return removeFromBoundary(edge, edge);
}

template <typename T, typename FP, typename VP>
template <typename H>
typename Polyhedron_Face<T, FP, VP>::HalfEdgeList Polyhedron_Face<T, FP, VP>::
  replaceBoundary(HalfEdge* from, HalfEdge* to, H&& with)
{
  assert(from != nullptr);
  assert(to != nullptr);
  assert(from->face() == this);
  assert(to->face() == this);

  const auto removeCount = countAndUnsetFace(from, to);
  countAndSetFace(with.front(), with.back(), this);
  return m_boundary.splice_replace(
    HalfEdgeList::iter(from),
    std::next(HalfEdgeList::iter(to)),
    removeCount,
    std::forward<H>(with));
}

template <typename T, typename FP, typename VP>
std::size_t Polyhedron_Face<T, FP, VP>::countAndSetFace(
  HalfEdge* from, HalfEdge* to, Face* face)
{
  std::size_t count = 0u;
  auto* cur = from;
  do
  {
    cur->setFace(face);
    cur = cur->next();
    ++count;
  } while (cur != to->next());
  return count;
}

template <typename T, typename FP, typename VP>
std::size_t Polyhedron_Face<T, FP, VP>::countAndUnsetFace(HalfEdge* from, HalfEdge* to)
{
  std::size_t count = 0u;
  auto* cur = from;
  do
  {
    cur->unsetFace();
    cur = cur->next();
    ++count;
  } while (cur != to->next());
  return count;
}

template <typename T, typename FP, typename VP>
std::size_t Polyhedron_Face<T, FP, VP>::countSharedVertices(const Face* other) const
{
  assert(other != nullptr);
  assert(other != this);

  auto myVertices = std::unordered_set<Vertex*>();
  for (auto* halfEdge : m_boundary)
  {
    myVertices.insert(halfEdge->origin());
  }

  std::size_t sharedVertexCount = 0u;
  for (auto* halfEdge : other->m_boundary)
  {
    if (myVertices.count(halfEdge->origin()) > 0u)
    {
      ++sharedVertexCount;
    }
  }

  return sharedVertexCount;
}

template <typename T, typename FP, typename VP>
class Polyhedron_Face<T, FP, VP>::RayIntersection
{
private:
  enum class Type
  {
    Front = 1,
    Back = 2,
  };

  Type m_type;
  T m_distance;

  RayIntersection(const Type type, const T distance)
    : m_type{type}
    , m_distance{distance}
  {
    assert(!vm::is_nan(m_distance));
  }

public:
  static RayIntersection Front(const T distance)
  {
    return RayIntersection(Type::Front, distance);
  }

  static RayIntersection Back(const T distance)
  {
    return RayIntersection(Type::Back, distance);
  }

  bool front() const { return m_type == Type::Front; }

  bool back() const { return m_type == Type::Back; }

  T distance() const { return m_distance; }
};

template <typename T, typename FP, typename VP>
typename std::optional<typename Polyhedron_Face<T, FP, VP>::RayIntersection>
Polyhedron_Face<T, FP, VP>::intersectWithRay(const vm::ray<T, 3>& ray) const
{
  const auto plane = vm::plane<T, 3>{origin(), normal()};
  const auto cos = vm::dot(plane.normal, ray.direction);

  if (vm::is_zero(cos, vm::constants<T>::almost_zero()))
  {
    return std::nullopt;
  }

  return vm::intersect_ray_polygon(
           ray,
           plane,
           m_boundary.begin(),
           m_boundary.end(),
           [](const HalfEdge* e) { return e->origin()->position(); })
         | kdl::optional_transform([&](const auto d) {
             return cos < T(0) ? RayIntersection::Front(d) : RayIntersection::Back(d);
           });
}

} // namespace tb::mdl
