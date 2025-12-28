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
#include "mdl/Polyhedron.h"

#include "kd/contracts.h"
#include "kd/vector_utils.h"

#include "vm/bbox.h"
#include "vm/constants.h"
#include "vm/plane.h"
#include "vm/segment.h"
#include "vm/util.h"

#include <list>
#include <unordered_set>
#include <vector>

namespace tb::mdl
{

namespace detail
{
template <typename T>
T computePlaneEpsilon(const std::vector<vm::vec<T, 3>>& points)
{
  auto builder = typename vm::bbox<T, 3>::builder{};
  builder.add(points.begin(), points.end());
  const auto size = builder.bounds().size();

  const auto defaultEpsilon = vm::constants<T>::point_status_epsilon();
  const auto computedEpsilon =
    vm::get_max_component(size) / T(10) * vm::constants<T>::point_status_epsilon();
  return std::max(computedEpsilon, defaultEpsilon);
}
} // namespace detail

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::addPoints(std::vector<vm::vec<T, 3>> points)
{
  if (!points.empty())
  {
    points = kdl::vec_sort_and_remove_duplicates(std::move(points));

    const auto planeEpsilon = detail::computePlaneEpsilon(points);
    for (const auto& point : points)
    {
      addPoint(point, planeEpsilon);
    }
  }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addPoint(
  const vm::vec<T, 3>& position, const T planeEpsilon)
{
  assert(checkInvariant());

  // quick test to discard vertices which would yield short edges
  for (const auto* v : m_vertices)
  {
    if (vm::distance(position, v->position()) < MinEdgeLength)
    {
      return nullptr;
    }
  }

  Vertex* result = nullptr;
  switch (vertexCount())
  {
  case 0:
    result = addFirstPoint(position);
    m_bounds.min = m_bounds.max = position;
    break;
  case 1:
    result = addSecondPoint(position);
    m_bounds = vm::merge(m_bounds, position);
    break;
  case 2:
    result = addThirdPoint(position);
    m_bounds = vm::merge(m_bounds, position);
    break;
  default:
    result = addFurtherPoint(position, planeEpsilon);
    if (result)
    {
      m_bounds = vm::merge(m_bounds, position);
    }
    break;
  }
  assert(checkInvariant());
  return result;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addFirstPoint(
  const vm::vec<T, 3>& position)
{
  contract_pre(empty());

  auto* newVertex = new Vertex{position};
  m_vertices.push_back(newVertex);
  return newVertex;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addSecondPoint(
  const vm::vec<T, 3>& position)
{
  contract_pre(point());

  auto* onlyVertex = *m_vertices.begin();
  if (position != onlyVertex->position())
  {
    auto* newVertex = new Vertex{position};
    m_vertices.push_back(newVertex);

    auto* halfEdge1 = new HalfEdge{onlyVertex};
    auto* halfEdge2 = new HalfEdge{newVertex};
    auto* edge = new Edge{halfEdge1, halfEdge2};
    m_edges.push_back(edge);
    return newVertex;
  }
  return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addThirdPoint(
  const vm::vec<T, 3>& position)
{
  contract_pre(edge());

  auto* v1 = m_vertices.front();
  auto* v2 = v1->next();

  return vm::is_colinear(v1->position(), v2->position(), position)
           ? addColinearThirdPoint(position)
           : addNonColinearThirdPoint(position);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addColinearThirdPoint(
  const vm::vec<T, 3>& position)
{
  contract_pre(edge());

  auto* v1 = m_vertices.front();
  auto* v2 = v1->next();
  contract_assert(vm::is_colinear(v1->position(), v2->position(), position));

  if (vm::segment<T, 3>(v1->position(), v2->position())
        .contains(position, vm::constants<T>::almost_zero()))
  {
    return nullptr;
  }

  if (vm::segment<T, 3>(position, v2->position())
        .contains(v1->position(), vm::constants<T>::almost_zero()))
  {
    v1->setPosition(position);
    return v1;
  }

  contract_assert((vm::segment<T, 3>(position, v1->position())
                     .contains(v2->position(), vm::constants<T>::almost_zero())));
  v2->setPosition(position);
  return v2;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addNonColinearThirdPoint(
  const vm::vec<T, 3>& position)
{
  contract_pre(edge());

  auto* v1 = m_vertices.front();
  auto* v2 = v1->next();
  contract_assert(!vm::is_colinear(v1->position(), v2->position(), position));

  auto* h1 = v1->leaving();
  auto* h2 = v2->leaving();
  contract_assert(h1->next() == h1);
  contract_assert(h1->previous() == h1);
  contract_assert(h2->next() == h2);
  contract_assert(h2->previous() == h2);

  if (const auto plane = vm::from_points(v2->position(), v1->position(), position))
  {
    auto* v3 = new Vertex{position};
    auto* h3 = new HalfEdge{v3};

    auto* e1 = m_edges.front();
    e1->makeFirstEdge(h1);
    e1->unsetSecondEdge();

    auto boundary = HalfEdgeList{};
    boundary.push_back(h1);
    boundary.push_back(h2);
    boundary.push_back(h3);

    auto* face = new Face{std::move(boundary), *plane};

    auto* e2 = new Edge{h2};
    auto* e3 = new Edge{h3};

    m_vertices.push_back(v3);
    m_edges.push_back(e2);
    m_edges.push_back(e3);
    m_faces.push_back(face);

    return v3;
  }
  return nullptr;
}


template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addFurtherPoint(
  const vm::vec<T, 3>& position, const T planeEpsilon)
{
  contract_pre(faceCount() > 0u);

  return faceCount() == 1 ? addFurtherPointToPolygon(position, planeEpsilon)
                          : addFurtherPointToPolyhedron(position, planeEpsilon);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addFurtherPointToPolygon(
  const vm::vec<T, 3>& position, const T planeEpsilon)
{
  auto* face = m_faces.front();
  const auto status = face->pointStatus(position, planeEpsilon);
  switch (status)
  {
  case vm::plane_status::inside:
    return addPointToPolygon(position, planeEpsilon);
  case vm::plane_status::above:
    face->flip();
    switchFallthrough();
  case vm::plane_status::below:
    return makePolyhedron(position, planeEpsilon);
  }
  // will never be reached
  return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::addPointToPolygon(
  const vm::vec<T, 3>& position, const T planeEpsilon)
{
  contract_pre(polygon());

  auto* face = m_faces.front();
  const auto& facePlane = face->plane();

  HalfEdge* firstVisibleEdge = nullptr;
  HalfEdge* lastVisibleEdge = nullptr;

  for (auto* curEdge : face->boundary())
  {
    auto* prevEdge = curEdge->previous();
    auto* nextEdge = curEdge->next();
    const auto prevStatus =
      prevEdge->pointStatus(facePlane.normal, position, planeEpsilon);
    const auto curStatus = curEdge->pointStatus(facePlane.normal, position, planeEpsilon);
    const auto nextStatus =
      nextEdge->pointStatus(facePlane.normal, position, planeEpsilon);

    // If the current edge contains the point, it will not be added anyway.
    const auto curSegment = vm::segment<T, 3>{
      curEdge->origin()->position(), curEdge->destination()->position()};
    if (
      curStatus == vm::plane_status::inside
      && curSegment.contains(position, vm::constants<T>::almost_zero()))
    {
      return nullptr;
    }

    if (prevStatus == vm::plane_status::below && curStatus != vm::plane_status::below)
    {
      firstVisibleEdge = curEdge;
    }

    if (curStatus != vm::plane_status::below && nextStatus == vm::plane_status::below)
    {
      lastVisibleEdge = curEdge;
    }

    if (firstVisibleEdge && lastVisibleEdge)
    {
      break;
    }
  }

  // Is the point contained in the polygon?
  if (!firstVisibleEdge || !lastVisibleEdge)
  {
    return nullptr;
  }

  // Now we know which edges are visible from the point. These will have to be replaced
  // with two new edges.
  auto* newVertex = new Vertex{position};
  auto* h1 = new HalfEdge{firstVisibleEdge->origin()};
  auto* h2 = new HalfEdge{newVertex};

  face->insertIntoBoundaryAfter(lastVisibleEdge, HalfEdgeList{h1});
  face->insertIntoBoundaryAfter(h1, HalfEdgeList{h2});
  auto visibleEdges = face->removeFromBoundary(firstVisibleEdge, lastVisibleEdge);

  h1->setAsLeaving();

  auto* e1 = new Edge{h1};
  auto* e2 = new Edge{h2};

  // delete the visible vertices and edges.
  // the visible half edges are deleted when visibleEdges goes out of scope
  for (auto* curEdge : visibleEdges)
  {
    auto* edge = curEdge->edge();
    m_edges.remove(edge);

    if (curEdge != visibleEdges.front())
    {
      auto* vertex = curEdge->origin();
      m_vertices.remove(vertex);
    }
  }

  m_edges.push_back(e1);
  m_edges.push_back(e2);
  m_vertices.push_back(newVertex);

  return newVertex;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::makePolyhedron(
  const vm::vec<T, 3>& position, const T planeEpsilon)
{
  contract_pre(polygon());

  auto seam = Seam{};
  auto* face = m_faces.front();
  const auto& boundary = face->boundary();

  // The seam must be CCW, so we have to iterate in reverse order in this case.
  for (auto it = boundary.rbegin(), end = boundary.rend(); it != end; ++it)
  {
    seam.push_back((*it)->edge());
  }

  if (auto cone = weaveCone(seam, position))
  {
    auto* top = cone->vertices.front();

    sealWithCone(std::move(*cone), seam);
    if (mergeCoplanarIncidentFaces(top, planeEpsilon))
    {
      return top;
    }
  }

  return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::
  addFurtherPointToPolyhedron(const vm::vec<T, 3>& position, const T planeEpsilon)
{
  contract_pre(polyhedron());

  auto seam = createSeamForHorizon(position, planeEpsilon);

  // If no correct seam could be created, we assume that the vertex was inside the
  // polyhedron. If the seam has multiple loops, this indicates that the point to be added
  // is very close to another vertex and no correct seam can be computed due to
  // imprecision. In that case, we just assume that the vertex is inside the polyhedron
  // and skip it.
  if (!seam || seam->empty())
  {
    return nullptr;
  }

  contract_assert(seam->size() >= 3);

  // Under certain circumstances, it is not possible to weave a cap onto the seam because
  // it would create a face with colinear points. In this case, we assume the vertex was
  // inside the polyhedron and skip it.
  if (!checkSeamForWeaving(*seam, position))
  {
    return nullptr;
  }

  if (auto cone = weaveCone(*seam, position))
  {
    auto* top = cone->vertices.front();

    split(*seam);
    sealWithCone(std::move(*cone), *seam);
    if (mergeCoplanarIncidentFaces(top, planeEpsilon))
    {
      return top;
    }
  }

  return nullptr;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T, FP, VP>::Seam
{
private:
  using List = std::list<Edge*>;
  List m_edges;

public:
  using iterator = typename List::iterator;
  using const_iterator = typename List::const_iterator;

public:
  /**
   * Appends the given edge to the end of this seam.
   *
   * If this seam is not empty, then the given edge must not be identical to the last edge
   * of this seam, and its first vertex must be identical to the last edge's second
   * vertex.
   *
   * @param edge the edge to append, must not be null
   */
  void push_back(Edge* edge)
  {
    contract_pre(edge != nullptr);
    contract_pre(empty() || edge != last());
    contract_pre(checkEdge(edge));

    m_edges.push_back(edge);
  }

  /**
   * Replaces the range [first, end) of this seam with the given edge.
   *
   * @param first start of the range of edges to replace
   * @param end end end of the range of edges to replace
   * @param replacement the replacemenet edge, must not be null
   */
  void replace(
    typename List::iterator first, typename List::iterator end, Edge* replacement)
  {
    m_edges.erase(first, end);
    m_edges.insert(end, replacement);
    assert(check());
  }

  /**
   * Indicates whether this seam is empty.
   *
   * @return true if this seam is empty and false otherwise
   */
  bool empty() const { return m_edges.empty(); }

  /**
   * Returns the number of edges in this seam.
   */
  std::size_t size() const { return m_edges.size(); }

  /**
   * Returns the first edge of this seam.
   *
   * Assumes that this seam is not empty.
   */
  Edge* first() const
  {
    contract_pre(!empty());

    return m_edges.front();
  }

  /**
   * Returns the second edge of this seam.
   *
   * Assumes that this seam contains at least two edges.
   */
  Edge* second() const
  {
    contract_pre(size() > 1u);

    return *std::next(m_edges.begin());
  }

  /**
   * Returns the last edge of this seam.
   *
   * Assumes that this seam is not empty.
   */
  Edge* last() const
  {
    contract_pre(!empty());

    return m_edges.back();
  }

  /**
   * Returns an iterator pointing to the first edge in this seam, or an end iterator if
   * this seam is empty.
   */
  iterator begin() { return m_edges.begin(); }

  /**
   * Returns an iterator pointing to the end of this seam.
   */
  iterator end() { return m_edges.end(); }

  /**
   * Returns a const iterator pointing to the first edge in this seam, or an end iterator
   * if this seam is empty.
   */
  const_iterator begin() const { return m_edges.begin(); }

  /**
   * Returns a const iterator pointing to the end of this seam.
   */
  const_iterator end() const { return m_edges.end(); }

  /**
   * Removes all edges from this seam.
   */
  void clear() { m_edges.clear(); }

  /**
   * Returns the vertices of the seam in counter clockwise order.
   */
  std::vector<Vertex*> vertices() const
  {
    auto result = std::vector<Vertex*>{};
    result.reserve(size());
    for (auto* edge : m_edges)
    {
      result.push_back(edge->firstVertex());
    }
    return result;
  }

  /**
   * Checks whether this seam is a consecutive list of edges connected with their
   * vertices.
   */
  bool hasMultipleLoops() const
  {
    contract_pre(size() > 2);

    auto visitedVertices = std::unordered_set<Vertex*>{};
    for (const auto* edge : m_edges)
    {
      if (!visitedVertices.insert(edge->secondVertex()).second)
      {
        return true;
      }
    }
    return false;
  }

private:
  /**
   * Checks whether the given edge is connected to last edge of the current seam, or more
   * precisely, whether the second vertex of the given edge is identical to the first
   * vertex of the last edge of this seam.
   *
   * @param edge the edge to check
   * @return true if this seam is empty or if the given edge shares a vertex with the last
   * edge of this seam, and false otherwise
   */
  bool checkEdge(Edge* edge) const
  {
    if (m_edges.empty())
    {
      return true;
    }

    auto* last = m_edges.back();
    return last->firstVertex() == edge->secondVertex();
  }

  /**
   * Checks whether the edges of this seam share their vertices, that is, for each edge,
   * its second vertex is identical to its predecessors first vertex.
   *
   * @return true if the edges of this seam share their vertices and false otherwise
   */
  bool check() const
  {
    contract_pre(size() > 2);

    const auto* last = m_edges.back();
    for (const auto* edge : m_edges)
    {
      if (last->firstVertex() != edge->secondVertex())
      {
        return false;
      }

      last = edge;
    }
    return true;
  }
};

template <typename T, typename FP, typename VP>
std::optional<typename Polyhedron<T, FP, VP>::Seam> Polyhedron<T, FP, VP>::
  createSeamForHorizon(const vm::vec<T, 3>& position, const T planeEpsilon)
{
  Face* initialVisibleFace = nullptr;
  for (auto* face : m_faces)
  {
    if (face->plane().point_status(position, planeEpsilon) != vm::plane_status::below)
    {
      initialVisibleFace = face;
      break;
    }
  }

  if (!initialVisibleFace)
  {
    return std::nullopt;
  }

  auto seam = Seam{};

  auto visitedFaces = std::unordered_set<Face*>{initialVisibleFace};
  visitFace(
    position, initialVisibleFace->boundary().front(), visitedFaces, seam, planeEpsilon);

  return seam;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::visitFace(
  const vm::vec<T, 3>& position,
  HalfEdge* initialBoundaryEdge,
  std::unordered_set<Face*>& visitedFaces,
  Seam& seam,
  const T planeEpsilon)
{
  auto* currentBoundaryEdge = initialBoundaryEdge;
  do
  {
    auto* neighbour = currentBoundaryEdge->twin()->face();
    if (
      neighbour->plane().point_status(position, planeEpsilon) != vm::plane_status::below)
    {
      if (visitedFaces.insert(neighbour).second)
      {
        visitFace(
          position, currentBoundaryEdge->twin(), visitedFaces, seam, planeEpsilon);
      }
    }
    else
    {
      auto* edge = currentBoundaryEdge->edge();
      edge->makeSecondEdge(currentBoundaryEdge);
      seam.push_back(edge);
    }

    currentBoundaryEdge = currentBoundaryEdge->next();
  } while (currentBoundaryEdge != initialBoundaryEdge);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::split(const Seam& seam)
{
  contract_pre(seam.size() >= 3);
  assert(!seam.hasMultipleLoops());

  // First, unset the second half edge of every seam edge.
  // Thereby remember the second half edge of the first seam edge.
  // Note that all seam edges are oriented such that their second half edge belongs
  // to the portion of the polyhedron that must be removed.
  auto* first = seam.first()->secondEdge();
  for (auto* edge : seam)
  {
    // Set the first edge as the leaving edge. Since the first one will remain
    // in the polyhedron, we can use this as an indicator whether or not to
    // delete a vertex in the call to deleteFaces.
    edge->setFirstAsLeaving();
    edge->unsetSecondEdge();
  }

  // Now we must delete all the faces, edges, and vertices which are above the seam.
  // Since we opened the seam, that is, we unset the 2nd half edge of each seam edge,
  // which belongs to the portion of the polyhedron that will be deleted, the deletion
  // will not touch the faces that should remain in the polyhedron. Additionally, the
  // seam edges will also not be deleted.
  // The first half edge we remembered above is our entry point into that portion of the
  // polyhedron. We must remember which faces we have already visited to stop the
  // recursion.
  auto visitedFaces = std::unordered_set<Face*>{};

  // Will automatically delete the vertices when it falls out of scope
  auto verticesToDelete = VertexList{};
  deleteFaces(first, visitedFaces, verticesToDelete);
}

template <typename T, typename FP, typename VP>
template <typename FaceSet>
void Polyhedron<T, FP, VP>::deleteFaces(
  HalfEdge* first, FaceSet& visitedFaces, VertexList& verticesToDelete)
{
  auto* face = first->face();

  // Have we already visited this face?
  if (!visitedFaces.insert(face).second)
  {
    return;
  }

  auto* current = first;
  do
  {
    if (auto* edge = current->edge())
    {
      // This indicates that the current half edge was not part of the seam before
      // the seam was opened, i.e., it may have a neighbour that should also be deleted.

      // If the current edge has a neighbour, we can go ahead and delete it.
      // Once the function returns, the neighbour is definitely deleted unless
      // we are in a recursive call where that neighbour is being deleted by one
      // of our callers. In that case, the call to deleteFaces returned immediately.
      if (edge->fullySpecified())
      {
        deleteFaces(edge->twin(current), visitedFaces, verticesToDelete);
      }

      if (edge->fullySpecified())
      {
        // This indicates that we are in a recursive call and that the neighbour across
        // the current edge is going to be deleted by one of our callers. We open the
        // edge and unset it so that it is not considered again later.
        edge->makeSecondEdge(current);
        edge->unsetSecondEdge();
      }
      else
      {
        // This indicates that the neighbour across the current edges has already been
        // deleted or that it will be deleted by one of our callers. This means that we
        // can safely unset the edge and delete it.
        current->unsetEdge();
        m_edges.remove(edge);
      }
    }

    if (auto* origin = current->origin(); origin->leaving() == current)
    {
      // We expect that the vertices on the seam have had a remaining edge
      // set as their leaving edge before the call to this function.
      verticesToDelete.splice_back(
        m_vertices, VertexList::iter(origin), std::next(VertexList::iter(origin)), 1u);
    }
    current = current->next();
  } while (current != first);

  m_faces.remove(face);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Face* Polyhedron<T, FP, VP>::sealWithSinglePolygon(
  const Seam& seam, const vm::plane<T, 3>& plane)
{
  contract_pre(seam.size() >= 3);
  contract_pre(!empty() && !point() && !edge() && !polygon());
  assert(!seam.hasMultipleLoops());

  auto boundary = HalfEdgeList{};
  for (auto* seamEdge : seam)
  {
    contract_assert(!seamEdge->fullySpecified());

    auto* origin = seamEdge->secondVertex();
    auto* boundaryEdge = new HalfEdge{origin};
    boundary.push_back(boundaryEdge);
    seamEdge->setSecondEdge(boundaryEdge);
  }

  auto* face = new Face{std::move(boundary), plane};
  m_faces.push_back(face);
  return face;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkSeamForWeaving(
  const Seam& seam, const vm::vec<T, 3>& position) const
{
  contract_pre(seam.size() >= 3);
  contract_pre(!empty() && !point() && !edge());
  assert(!seam.hasMultipleLoops());

  for (auto* edge : seam)
  {
    auto* v1 = edge->secondVertex();
    auto* v2 = edge->firstVertex();

    if (!vm::plane_normal(position, v1->position(), v2->position()))
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
std::optional<typename Polyhedron<T, FP, VP>::WeaveConeResult> Polyhedron<T, FP, VP>::
  weaveCone(const Seam& seam, const vm::vec<T, 3>& position)
{
  contract_pre(seam.size() >= 3);
  assert(!seam.hasMultipleLoops());

  auto vertices = VertexList{};
  auto edges = EdgeList{};
  auto faces = FaceList{};
  HalfEdge* firstSeamEdge = nullptr;

  auto* top = new Vertex{position};
  vertices.push_back(top);

  HalfEdge* first = nullptr;
  HalfEdge* last = nullptr;

  for (auto* edge : seam)
  {
    auto* v1 = edge->secondVertex();
    auto* v2 = edge->firstVertex();

    auto* h1 = new HalfEdge{top};
    auto* h2 = new HalfEdge{v1};
    auto* h3 = new HalfEdge{v2};
    auto* h = h3;

    auto boundary = HalfEdgeList{};
    boundary.push_back(h1);
    boundary.push_back(h2);
    boundary.push_back(h3);

    if (!firstSeamEdge)
    {
      firstSeamEdge = h2;
    }

    const auto plane = vm::from_points(v1->position(), position, v2->position());
    if (!plane)
    {
      return std::nullopt;
    }

    faces.push_back(new Face{std::move(boundary), *plane});

    if (last)
    {
      edges.push_back(new Edge{h1, last});
    }

    if (!first)
    {
      first = h1;
    }

    last = h;
  }

  contract_assert(first->face() != last->face());
  edges.push_back(new Edge(first, last));

  return WeaveConeResult{
    std::move(vertices), std::move(edges), std::move(faces), firstSeamEdge};
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::sealWithCone(WeaveConeResult cone, const Seam& seam)
{
  auto* currentConeEdge = cone.firstSeamEdge;
  for (auto* edge : seam)
  {
    edge->setSecondEdge(currentConeEdge);
    currentConeEdge = currentConeEdge->next()->twin()->next();
  }

  m_vertices.append(cone.vertices);
  m_edges.append(cone.edges);
  m_faces.append(cone.faces);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::mergeCoplanarIncidentFaces(
  Vertex* vertex, const T planeEpsilon)
{
  contract_pre(vertex != nullptr);
  assert(checkInvariant());

  const auto checkAndMergeIncidentNeighbours = [&](auto* leaving) -> HalfEdge* {
    auto* next = leaving->nextIncident();
    auto* edge = leaving->edge();
    auto* firstFace = edge->firstFace();
    auto* secondFace = edge->secondFace();

    if (firstFace->coplanar(secondFace, planeEpsilon))
    {
      // If the vertex has only three incident edges, then it will be removed when the
      // faces are merged.
      const auto hasThreeIncidentEdges =
        leaving == leaving->nextIncident()->nextIncident()->nextIncident();

      if (hasThreeIncidentEdges && m_faces.size() == 4u)
      {
        // Merging any faces will fail because it will turn this polyhedron into a
        // polygon. We treat this case by removing all incident faces and edges, and the
        // given vertex.

        // Build a seam consisting of the edges of the remaining polygon.
        auto seam = Seam{};
        auto* remainingFace = leaving->next()->twin()->face();
        auto& boundary = remainingFace->boundary();

        // The seam must be CCW, so we have to iterate in reverse order in this case.
        for (auto it = boundary.rbegin(), end = boundary.rend(); it != end; ++it)
        {
          auto* halfEdge = *it;
          auto* seamEdge = halfEdge->edge();
          seamEdge->makeFirstEdge(halfEdge);
          seam.push_back(seamEdge);
        }

        // Split this polyhedron.
        split(seam);
        return nullptr;
      }

      // Choose the face to retain. We prefer the face that produces minimal error, i.e.
      // the face such such that the maximum distance of the other face's vertices to the
      // face plane is minimal.
      if (
        firstFace->maximumVertexDistance(secondFace->plane())
        < secondFace->maximumVertexDistance(firstFace->plane()))
      {
        // firstFace->plane() will introduce a smaller error, so merge secondFace into
        // firstFace
        assertResult(mergeNeighbours(edge->firstEdge()));
      }
      else
      {
        // secondFace->plane() will introduce a smaller error, so merge firstFace into
        // secondFace
        assertResult(mergeNeighbours(edge->secondEdge()));
      }

      if (hasThreeIncidentEdges)
      {
        return nullptr;
      }
    }

    return next;
  };

  auto* firstLeaving = vertex->leaving();
  contract_assert(
    firstLeaving != firstLeaving->nextIncident()
    && firstLeaving != firstLeaving->nextIncident()->nextIncident());

  // First we merge all incident coplanar faces.
  // First we treat all incident edges except for firstLeaving, which must stay intact
  // because we test it in the loop condition
  auto* currentLeaving = firstLeaving->nextIncident();
  while (currentLeaving != firstLeaving)
  {
    currentLeaving = checkAndMergeIncidentNeighbours(currentLeaving);
    if (!currentLeaving)
    {
      // all faces were coplanar
      return false;
    }
  };

  // we have at least three incident edges left
  contract_assert(
    firstLeaving != firstLeaving->nextIncident()
    && firstLeaving != firstLeaving->nextIncident()->nextIncident());

  // treat firstLeaving now
  if (!checkAndMergeIncidentNeighbours(firstLeaving))
  {
    // all faces were coplanar
    return false;
  }

  return true;
}

} // namespace tb::mdl
