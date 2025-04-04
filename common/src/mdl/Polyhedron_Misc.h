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

#include "kdl/range_utils.h"

#include "vm/bbox.h"
#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/scalar.h"
#include "vm/util.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace tb::mdl
{

template <typename T, typename FP, typename VP>
const vm::vec<T, 3>& Polyhedron<T, FP, VP>::GetVertexPosition::operator()(
  const Vertex* vertex) const
{
  return vertex->position();
}

template <typename T, typename FP, typename VP>
const vm::vec<T, 3>& Polyhedron<T, FP, VP>::GetVertexPosition::operator()(
  const HalfEdge* halfEdge) const
{
  return halfEdge->origin()->position();
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::CopyCallback::~CopyCallback() = default;

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::CopyCallback::vertexWasCopied(
  const Vertex* /* original */, Vertex* /* copy */) const
{
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::CopyCallback::faceWasCopied(
  const Face* /* original */, Face* /* copy */) const
{
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron()
{
  updateBounds();
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron(std::initializer_list<vm::vec<T, 3>> positions)
{
  addPoints(std::vector<vm::vec<T, 3>>(std::begin(positions), std::end(positions)));
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron(const vm::bbox<T, 3>& bounds)
  : m_bounds(bounds)
{
  if (m_bounds.min == m_bounds.max)
  {
    addPoint(m_bounds.min, vm::constants<T>::point_status_epsilon());
    return;
  }

  // Explicitly create the polyhedron for better performance when building brushes.

  const auto p1 = vm::vec<T, 3>{m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()};
  const auto p2 = vm::vec<T, 3>{m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()};
  const auto p3 = vm::vec<T, 3>{m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()};
  const auto p4 = vm::vec<T, 3>{m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()};
  const auto p5 = vm::vec<T, 3>{m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()};
  const auto p6 = vm::vec<T, 3>{m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()};
  const auto p7 = vm::vec<T, 3>{m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()};
  const auto p8 = vm::vec<T, 3>{m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()};

  auto* v1 = new Vertex{p1};
  auto* v2 = new Vertex{p2};
  auto* v3 = new Vertex{p3};
  auto* v4 = new Vertex{p4};
  auto* v5 = new Vertex{p5};
  auto* v6 = new Vertex{p6};
  auto* v7 = new Vertex{p7};
  auto* v8 = new Vertex{p8};

  m_vertices = VertexList{v1, v2, v3, v4, v5, v6, v7, v8};

  // Front face
  auto* f1h1 = new HalfEdge{v1};
  auto* f1h2 = new HalfEdge{v5};
  auto* f1h3 = new HalfEdge{v6};
  auto* f1h4 = new HalfEdge{v2};
  m_faces.push_back(new Face{HalfEdgeList{f1h1, f1h2, f1h3, f1h4}, {p1, {0, -1, 0}}});

  // Left face
  auto* f2h1 = new HalfEdge{v1};
  auto* f2h2 = new HalfEdge{v2};
  auto* f2h3 = new HalfEdge{v4};
  auto* f2h4 = new HalfEdge{v3};
  m_faces.push_back(new Face{HalfEdgeList{f2h1, f2h2, f2h3, f2h4}, {p1, {-1, 0, 0}}});

  // Bottom face
  auto* f3h1 = new HalfEdge{v1};
  auto* f3h2 = new HalfEdge{v3};
  auto* f3h3 = new HalfEdge{v7};
  auto* f3h4 = new HalfEdge{v5};
  m_faces.push_back(new Face{HalfEdgeList{f3h1, f3h2, f3h3, f3h4}, {p1, {0, 0, -1}}});

  // Top face
  auto* f4h1 = new HalfEdge{v2};
  auto* f4h2 = new HalfEdge{v6};
  auto* f4h3 = new HalfEdge{v8};
  auto* f4h4 = new HalfEdge{v4};
  m_faces.push_back(new Face{HalfEdgeList{f4h1, f4h2, f4h3, f4h4}, {p8, {0, 0, 1}}});

  // Back face
  auto* f5h1 = new HalfEdge{v3};
  auto* f5h2 = new HalfEdge{v4};
  auto* f5h3 = new HalfEdge{v8};
  auto* f5h4 = new HalfEdge{v7};
  m_faces.push_back(new Face{HalfEdgeList{f5h1, f5h2, f5h3, f5h4}, {p8, {0, 1, 0}}});

  // Right face
  auto* f6h1 = new HalfEdge{v5};
  auto* f6h2 = new HalfEdge{v7};
  auto* f6h3 = new HalfEdge{v8};
  auto* f6h4 = new HalfEdge{v6};
  m_faces.push_back(new Face{HalfEdgeList{f6h1, f6h2, f6h3, f6h4}, {p8, {1, 0, 0}}});

  m_edges.push_back(new Edge{f1h4, f2h1}); // v1, v2
  m_edges.push_back(new Edge{f2h4, f3h1}); // v1, v3
  m_edges.push_back(new Edge{f1h1, f3h4}); // v1, v5
  m_edges.push_back(new Edge{f2h2, f4h4}); // v2, v4
  m_edges.push_back(new Edge{f4h1, f1h3}); // v2, v6
  m_edges.push_back(new Edge{f2h3, f5h1}); // v3, v4
  m_edges.push_back(new Edge{f3h2, f5h4}); // v3, v7
  m_edges.push_back(new Edge{f4h3, f5h2}); // v4, v8
  m_edges.push_back(new Edge{f1h2, f6h4}); // v5, v6
  m_edges.push_back(new Edge{f6h1, f3h3}); // v5, v7
  m_edges.push_back(new Edge{f6h3, f4h2}); // v6, v8
  m_edges.push_back(new Edge{f6h2, f5h3}); // v7, v8
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron(std::vector<vm::vec<T, 3>> positions)
{
  addPoints(std::move(positions));
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron(const Polyhedron<T, FP, VP>& other)
{
  auto copy = Copy{other.faces(), other.edges(), other.vertices(), *this, CopyCallback{}};
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron(
  const Polyhedron<T, FP, VP>& other, const CopyCallback& callback)
{
  auto copy = Copy{other.faces(), other.edges(), other.vertices(), *this, callback};
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>::Polyhedron(Polyhedron<T, FP, VP>&& other) noexcept
  : m_vertices{std::move(other.m_vertices)}
  , m_edges{std::move(other.m_edges)}
  , m_faces{std::move(other.m_faces)}
  , m_bounds{std::move(other.m_bounds)}
{
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>& Polyhedron<T, FP, VP>::operator=(
  const Polyhedron<T, FP, VP>& other)
{
  auto copy = Polyhedron<T, FP, VP>{other};
  swap(*this, copy);
  return *this;
}

template <typename T, typename FP, typename VP>
Polyhedron<T, FP, VP>& Polyhedron<T, FP, VP>::operator=(Polyhedron<T, FP, VP>&& other) =
  default;

/**
 * Copies a polyhedron.
 */
template <typename T, typename FP, typename VP>
class Polyhedron<T, FP, VP>::Copy
{
private:
  using VertexMap = std::unordered_map<const Vertex*, Vertex*>;
  using VertexMapEntry = typename VertexMap::value_type;

  using HalfEdgeMap = std::unordered_map<const HalfEdge*, HalfEdge*>;
  using HalfEdgeMapEntry = typename HalfEdgeMap::value_type;

  /**
   * Maps the vertices of the original to their copies.
   */
  VertexMap m_vertexMap;

  /**
   * Maps the half edges of the original to their copies.
   */
  HalfEdgeMap m_halfEdgeMap;

  /**
   * The copied vertices.
   */
  VertexList m_vertices;

  /**
   * The copied edges.
   */
  EdgeList m_edges;

  /**
   * The copied faces.
   */
  FaceList m_faces;

  /**
   * The polyhedron which should become a copy.
   */
  Polyhedron& m_destination;

public:
  /**
   * Copies a polyhedron with the given faces, edges and vertices into the given
   * destination polyhedron. The callback can be used to set up the face and vertex
   * payloads.
   *
   * @param originalFaces the faces to copy
   * @param originalEdges the edges to copy
   * @param originalVertices the vertices to copy
   * @param destination the destination polyhedron that will become a copy
   * @param callback the callback to call for every created face or vertex             *
   */
  Copy(
    const FaceList& originalFaces,
    const EdgeList& originalEdges,
    const VertexList& originalVertices,
    Polyhedron& destination,
    const CopyCallback& callback)
    : m_destination{destination}
  {
    copyVertices(originalVertices, callback);
    copyFaces(originalFaces, callback);
    copyEdges(originalEdges);
    swapContents();
  }

private:
  void copyVertices(const VertexList& originalVertices, const CopyCallback& callback)
  {
    for (const auto* currentVertex : originalVertices)
    {
      auto* copy = new Vertex{currentVertex->position()};
      callback.vertexWasCopied(currentVertex, copy);
      assert(m_vertexMap.count(currentVertex) == 0u);
      m_vertexMap.emplace(currentVertex, copy);
      m_vertices.push_back(copy);
      currentVertex = currentVertex->next();
    }
  }

  void copyFaces(const FaceList& originalFaces, const CopyCallback& callback)
  {
    for (const auto* currentFace : originalFaces)
    {
      copyFace(currentFace, callback);
    }
  }

  void copyFace(const Face* originalFace, const CopyCallback& callback)
  {
    auto myBoundary = HalfEdgeList{};
    for (const auto* currentHalfEdge : originalFace->boundary())
    {
      myBoundary.push_back(copyHalfEdge(currentHalfEdge));
    }

    auto* copy = new Face{std::move(myBoundary), originalFace->plane()};
    callback.faceWasCopied(originalFace, copy);
    m_faces.push_back(copy);
  }

  HalfEdge* copyHalfEdge(const HalfEdge* original)
  {
    const auto* originalOrigin = original->origin();

    auto* myOrigin = findVertex(originalOrigin);
    auto* copy = new HalfEdge{myOrigin};
    assert(m_halfEdgeMap.count(original) == 0u);
    m_halfEdgeMap.emplace(original, copy);
    return copy;
  }

  Vertex* findVertex(const Vertex* original)
  {
    auto it = m_vertexMap.find(original);
    assert(it != m_vertexMap.end());
    return it->second;
  }

  void copyEdges(const EdgeList& originalEdges)
  {
    for (const auto* currentEdge : originalEdges)
    {
      m_edges.push_back(copyEdge(currentEdge));
    }
  }

  Edge* copyEdge(const Edge* original)
  {
    auto* myFirst = findOrCopyHalfEdge(original->firstEdge());
    if (!original->fullySpecified())
    {
      return new Edge{myFirst};
    }

    auto* mySecond = findOrCopyHalfEdge(original->secondEdge());
    return new Edge{myFirst, mySecond};
  }

  HalfEdge* findOrCopyHalfEdge(const HalfEdge* original)
  {
    if (auto it = m_halfEdgeMap.find(original); it != m_halfEdgeMap.end())
    {
      return it->second;
    }

    const auto* originalOrigin = original->origin();
    auto* myOrigin = findVertex(originalOrigin);
    auto* copy = new HalfEdge{myOrigin};
    m_halfEdgeMap.emplace(original, copy);
    return copy;
  }

  void swapContents()
  {
    using std::swap;
    swap(m_vertices, m_destination.m_vertices);
    swap(m_edges, m_destination.m_edges);
    swap(m_faces, m_destination.m_faces);
    m_destination.updateBounds();
  }
};

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::operator==(const Polyhedron& other) const
{
  if (vertexCount() != other.vertexCount())
  {
    return false;
  }
  if (edgeCount() != other.edgeCount())
  {
    return false;
  }
  if (faceCount() != other.faceCount())
  {
    return false;
  }

  for (const auto* current : m_vertices)
  {
    if (!other.hasVertex(current->position(), 0.0))
    {
      return false;
    }
  }

  for (const auto* current : m_edges)
  {
    if (!other.hasEdge(
          current->firstVertex()->position(), current->secondVertex()->position(), 0.0))
    {
      return false;
    }
  }

  for (const auto* current : m_faces)
  {
    if (!other.hasFace(current->vertexPositions(), 0.0))
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::operator!=(const Polyhedron& other) const
{
  return !(*this == other);
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T, FP, VP>::vertexCount() const
{
  return m_vertices.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T, FP, VP>::VertexList& Polyhedron<T, FP, VP>::vertices() const
{
  return m_vertices;
}

template <typename T, typename FP, typename VP>
std::vector<vm::vec<T, 3>> Polyhedron<T, FP, VP>::vertexPositions() const
{
  auto result = std::vector<vm::vec<T, 3>>{};
  result.reserve(vertexCount());
  for (const auto* vertex : m_vertices)
  {
    result.push_back(vertex->position());
  }
  return result;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T, FP, VP>::edgeCount() const
{
  return m_edges.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T, FP, VP>::EdgeList& Polyhedron<T, FP, VP>::edges() const
{
  return m_edges;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::hasEdge(
  const vm::vec<T, 3>& pos1, const vm::vec<T, 3>& pos2, const T epsilon) const
{
  return findEdgeByPositions(pos1, pos2, epsilon) != nullptr;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T, FP, VP>::faceCount() const
{
  return m_faces.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T, FP, VP>::FaceList& Polyhedron<T, FP, VP>::faces() const
{
  return m_faces;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::FaceList& Polyhedron<T, FP, VP>::faces()
{
  return m_faces;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::hasFace(
  const std::vector<vm::vec<T, 3>>& positions, const T epsilon) const
{
  return findFaceByPositions(positions, epsilon) != nullptr;
}

template <typename T, typename FP, typename VP>
const vm::bbox<T, 3>& Polyhedron<T, FP, VP>::bounds() const
{
  return m_bounds;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::empty() const
{
  return vertexCount() == 0;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::point() const
{
  return vertexCount() == 1;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::edge() const
{
  return vertexCount() == 2;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polygon() const
{
  return faceCount() == 1;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::polyhedron() const
{
  return faceCount() > 3;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::closed() const
{
  return vertexCount() + faceCount() == edgeCount() + 2;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::clear()
{
  m_faces.clear();
  m_edges.clear();
  m_vertices.clear();
  updateBounds();
}

template <typename T, typename FP, typename VP>
std::optional<typename Polyhedron<T, FP, VP>::FaceHit> Polyhedron<T, FP, VP>::pickFace(
  const vm::ray<T, 3>& ray) const
{
  const auto side = polygon() ? vm::side::both : vm::side::front;
  auto* firstFace = m_faces.front();
  auto* currentFace = firstFace;
  do
  {
    if (const auto distance = currentFace->intersectWithRay(ray, side))
    {
      return FaceHit{*currentFace, *distance};
    }
    currentFace = currentFace->next();
  } while (currentFace != firstFace);

  return std::nullopt;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::hasVertex(
  const vm::vec<T, 3>& position, const T epsilon) const
{
  return findVertexByPosition(position, epsilon) != nullptr;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::hasAnyVertex(
  const std::vector<vm::vec<T, 3>>& positions, const T epsilon) const
{
  for (const auto& position : positions)
  {
    if (hasVertex(position, epsilon))
    {
      return true;
    }
  }
  return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::hasAllVertices(
  const std::vector<vm::vec<T, 3>>& positions, const T epsilon) const
{
  if (positions.size() != vertexCount())
  {
    return false;
  }
  for (const auto& position : positions)
  {
    if (!hasVertex(position, epsilon))
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::findVertexByPosition(
  const vm::vec<T, 3>& position, const T epsilon) const
{
  if (m_vertices.empty())
  {
    return nullptr;
  }

  auto* firstVertex = m_vertices.front();
  auto* currentVertex = firstVertex;
  do
  {
    if (vm::is_equal(position, currentVertex->position(), epsilon))
    {
      return currentVertex;
    }
    currentVertex = currentVertex->next();
  } while (currentVertex != firstVertex);
  return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Vertex* Polyhedron<T, FP, VP>::findClosestVertex(
  const vm::vec<T, 3>& position, const T maxDistance) const
{
  if (m_vertices.empty())
  {
    return nullptr;
  }

  auto closestDistance2 = maxDistance * maxDistance;
  Vertex* closestVertex = nullptr;

  auto* firstVertex = m_vertices.front();
  auto* currentVertex = firstVertex;
  do
  {
    const auto currentDistance2 =
      vm::squared_distance(position, currentVertex->position());
    if (currentDistance2 < closestDistance2)
    {
      closestDistance2 = currentDistance2;
      closestVertex = currentVertex;
    }
    currentVertex = currentVertex->next();
  } while (currentVertex != firstVertex);
  return closestVertex;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Edge* Polyhedron<T, FP, VP>::findEdgeByPositions(
  const vm::vec<T, 3>& pos1, const vm::vec<T, 3>& pos2, const T epsilon) const
{
  if (m_edges.empty())
  {
    return nullptr;
  }

  auto* firstEdge = m_edges.front();
  auto* currentEdge = firstEdge;
  do
  {
    if (currentEdge->hasPositions(pos1, pos2, epsilon))
    {
      return currentEdge;
    }
    currentEdge = currentEdge->next();
  } while (currentEdge != firstEdge);
  return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Edge* Polyhedron<T, FP, VP>::findClosestEdge(
  const vm::vec<T, 3>& pos1, const vm::vec<T, 3>& pos2, const T maxDistance) const
{
  if (m_edges.empty())
  {
    return nullptr;
  }

  auto closestDistance = maxDistance;
  Edge* closestEdge = nullptr;

  auto* firstEdge = m_edges.front();
  auto* currentEdge = firstEdge;
  do
  {
    const auto currentDistance = currentEdge->distanceTo(pos1, pos2);
    if (currentDistance < closestDistance)
    {
      closestDistance = currentDistance;
      closestEdge = currentEdge;
    }
    currentEdge = currentEdge->next();
  } while (currentEdge != firstEdge);
  return closestEdge;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Face* Polyhedron<T, FP, VP>::findFaceByPositions(
  const std::vector<vm::vec<T, 3>>& positions, const T epsilon) const
{
  auto* firstFace = m_faces.front();
  auto* currentFace = firstFace;
  do
  {
    if (currentFace->hasVertexPositions(positions, epsilon))
    {
      return currentFace;
    }
    currentFace = currentFace->next();
  } while (currentFace != firstFace);
  return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Face* Polyhedron<T, FP, VP>::findClosestFace(
  const std::vector<vm::vec<T, 3>>& positions, const T maxDistance)
{
  auto closestDistance = maxDistance;
  Face* closestFace = nullptr;

  auto* firstFace = m_faces.front();
  auto* currentFace = firstFace;
  do
  {
    const auto currentDistance = currentFace->distanceTo(positions);
    if (currentDistance < closestDistance)
    {
      closestDistance = currentDistance;
      closestFace = currentFace;
    }
    currentFace = currentFace->next();
  } while (currentFace != firstFace);
  return closestFace;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::updateBounds()
{
  auto builder = typename vm::bbox<T, 3>::builder();
  builder.add(m_vertices.begin(), m_vertices.end(), GetVertexPosition{});

  if (!builder.initialized())
  {
    m_bounds.min = m_bounds.max = vm::vec<T, 3>::nan();
  }
  else
  {
    m_bounds = builder.bounds();
  }
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::correctVertexPositions(const size_t decimals, const T epsilon)
{
  for (auto* vertex : m_vertices)
  {
    vertex->correctPosition(decimals, epsilon);
  }
  updateBounds();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::healEdges(const T minLength)
{
  const auto minLength2 = minLength * minLength;

  const auto findShortEdge = [&]() -> typename Polyhedron<T, FP, VP>::Edge* {
    for (auto edge : m_edges)
    {
      if (vm::squared_length(edge->vector()) < minLength2)
      {
        return edge;
      }
    }
    return nullptr;
  };

  for (auto* edge = findShortEdge(); edge != nullptr && polyhedron();
       edge = findShortEdge())
  {
    if (removeEdge(edge) == nullptr)
    {
      return false;
    }
  }

  assert(!polyhedron() || checkEdgeLengths(minLength));

  updateBounds();

  return polyhedron();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T, FP, VP>::Edge* Polyhedron<T, FP, VP>::removeEdge(Edge* edge)
{
  /*
    | f1 | n1
    v1-e-v2
    | f2 | n2

    Let e be the edge to remove. If f1 is a triangle, we merge f1 into n1. Then, if
    f2 is a triangle, we merge that into n2.
    This can have two outcomes:

    - v2 becomes redundant and is removed to repair the topological error. In that case,
      e is also removed and we are done.
    - v2 remains, and we need to remove e manually. To do that, we transfer all edges
      from v2 to v1, so e becomes a loop, and we can safely remove it after.

    Note that n1 and n2 can be identical. If that is the case, then v2 is immediately
    removed. We also need to be aware that removing v2 may remove e, so we cannot access
    e again.
  */

  auto* validEdge = edge->next();
  auto* v1 = edge->firstVertex();
  auto* v2 = edge->secondVertex();
  if (v1 == v2)
  {
    // This should happen, but rarely it does. For now, we signal an error and abort.
    return nullptr;
  }

  const auto v2WasRemoved = [&]() {
    return std::find(m_vertices.begin(), m_vertices.end(), v2) == m_vertices.end();
  };

  // merge f1 into n1:
  if (
    edge->firstFace()->vertexCount() == 3u
    && !mergeNeighbours(edge->firstEdge()->next()->twin(), validEdge))
  {
    return nullptr;
  }

  // merge f2 into n2 if necessary:
  if (!v2WasRemoved())
  {
    if (
      edge->secondFace()->vertexCount() == 3u
      && !mergeNeighbours(edge->secondEdge()->previous()->twin(), validEdge))
    {
      return nullptr;
    }

    if (!v2WasRemoved())
    {
      // Transfer all edges from v2 to v1.
      // This results in e being a loop and v2 to be orphaned.
      while (v2->leaving())
      {
        auto* leaving = v2->leaving();
        auto* newLeaving = leaving->previous()->twin();
        leaving->setOrigin(v1);
        if (newLeaving->origin() == v2)
        {
          v2->setLeaving(newLeaving);
        }
        else
        {
          v2->setLeaving(nullptr);
        }
      }

      // Remove the edge's first edge from its first face
      auto* f1 = edge->firstFace();
      auto* h1 = edge->firstEdge();
      auto* n = h1->next();
      v1->setLeaving(h1->previous()->twin());
      f1->removeFromBoundary(h1);
      n->setOrigin(v1);

      // Remove the edges's second edge from its second face
      auto* f2 = edge->secondFace();
      auto* h2 = edge->secondEdge();
      f2->removeFromBoundary(h2);

      // Finally, remove v2 and e
      m_vertices.remove(v2);
      m_edges.remove(edge);
    }
  }

  return validEdge;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::removeDegenerateFace(Face* face)
{
  assert(face != nullptr);
  assert(face->vertexCount() == 2u);

  // The boundary of the face to remove consists of two half edges:
  auto* halfEdge1 = face->boundary().front();
  auto* halfEdge2 = halfEdge1->next();
  assert(halfEdge2->next() == halfEdge1);
  assert(halfEdge1->previous() == halfEdge2);

  // The face has two vertices:
  auto* vertex1 = halfEdge1->origin();
  auto* vertex2 = halfEdge2->origin();

  // Make sure we don't delete the vertices' leaving edges:
  vertex1->setLeaving(halfEdge2->twin());
  vertex2->setLeaving(halfEdge1->twin());

  assert(vertex1->leaving() != halfEdge1);
  assert(vertex1->leaving() != halfEdge2);
  assert(vertex2->leaving() != halfEdge1);
  assert(vertex2->leaving() != halfEdge2);

  // These two edges will be merged into one:
  auto* edge1 = halfEdge1->edge();
  auto* edge2 = halfEdge2->edge();

  // The twins of the two half edges of the degenerate face will become twins now.
  auto* halfEdge1Twin = halfEdge1->twin();
  auto* halfEdge2Twin = halfEdge2->twin();

  // We will keep edge1 and delete edge2.
  // Make sure that halfEdge1's twin is the first edge of edge1:
  edge1->makeFirstEdge(halfEdge1Twin);

  // Now replace halfEdge2 by new halfEdge2Twin:
  assert(halfEdge2Twin->edge() == edge2);
  halfEdge2Twin->unsetEdge();
  edge1->unsetSecondEdge(); // unsets halfEdge1, leaving halfEdge1Twin as the first half
                            // edge of edge1
  edge1->setSecondEdge(halfEdge2Twin); // replace halfEdge1 with halfEdge2Twin

  // Now edge1 should be correct:
  assert(edge1->firstEdge() == halfEdge1Twin);
  assert(edge1->secondEdge() == halfEdge2Twin);

  // Delete the now obsolete edge.
  // The constructor doesn't do anything, so no further cleanup is necessary.
  m_edges.remove(edge2);

  // Delete the degenerate face. This also deletes its boundary of halfEdge1 and
  // halfEdge2.
  m_faces.remove(face);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::mergeNeighbours(HalfEdge* borderFirst, Edge*& validEdge)
{
  auto* face = borderFirst->face();
  auto* neighbour = borderFirst->twin()->face();

  // find the entire border between the two faces
  while (borderFirst->previous()->face() == face
         && borderFirst->previous()->twin()->face() == neighbour)
  {
    borderFirst = borderFirst->previous();
  }

  auto* twinLast = borderFirst->twin();
  auto* borderLast = borderFirst;

  while (borderLast->next()->face() == face
         && borderLast->next()->twin()->face() == neighbour)
  {
    borderLast = borderLast->next();
  }

  auto* twinFirst = borderLast->twin();

  auto* borderFirstOrigin = borderFirst->origin();
  auto* twinFirstOrigin = twinFirst->origin();

  // make sure we don't remove any leaving edges
  borderFirstOrigin->setLeaving(twinLast->next());
  twinFirstOrigin->setLeaving(borderLast->next());

  auto* remainingFirst = twinLast->next();
  auto* remainingLast = twinFirst->previous();

  auto edgesToRemove = neighbour->removeFromBoundary(twinFirst, twinLast);
  auto remainingEdges = neighbour->removeFromBoundary(remainingFirst, remainingLast);
  assert(neighbour->boundary().empty());

  // the replaced edges are deleted
  face->replaceBoundary(borderFirst, borderLast, std::move(remainingEdges));

  // now delete any remaining vertices and edges
  // edgesToRemove are deleted when the container falls out of scope
  auto* firstEdge = edgesToRemove.front();
  auto* curEdge = firstEdge;
  do
  {
    auto* edge = curEdge->edge();
    auto* next = curEdge->next();
    auto* origin = curEdge->origin();

    if (edge == validEdge)
    {
      validEdge = validEdge->next();
    }

    m_edges.remove(edge);

    // don't delete the origin of the first twin edge!
    if (curEdge != twinFirst)
    {
      m_vertices.remove(origin);
    }

    curEdge = next;
  } while (curEdge != firstEdge);

  m_faces.remove(neighbour);

  // Fix topological errors
  const auto fixTopologicalErrors = [&](auto* vertex) {
    if (!polyhedron())
    {
      return false;
    }

    if (vertex->hasTwoIncidentEdges())
    {
      // vertex has become redundant, so we need to remove it.
      auto* face1 = vertex->leaving()->face();
      auto* face2 = vertex->leaving()->twin()->face();

      if (face1->vertexCount() == 3u || face2->vertexCount() == 3u)
      {
        // If either face is a triangle, then the other face has become convex. We merge
        // the two faces.
        auto* borderEdge = vertex->leaving();
        if (borderEdge->face() != face)
        {
          // We want to retain the original face, so we make sure that we pass the correct
          // half edge to mergeNeighbours.
          borderEdge = borderEdge->twin();
        }
        return mergeNeighbours(borderEdge, validEdge);
      }
      else
      {
        assert(face1->vertexCount() > 3u && face2->vertexCount() > 3u);
        if (validEdge == vertex->leaving()->edge())
        {
          validEdge = validEdge->next();
        }
        mergeIncidentEdges(vertex);
      }
    }

    return polyhedron();
  };

  return fixTopologicalErrors(borderFirstOrigin) && fixTopologicalErrors(twinFirstOrigin);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::mergeNeighbours(HalfEdge* borderFirst)
{
  Edge* e = nullptr;
  return mergeNeighbours(borderFirst, e);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T, FP, VP>::mergeIncidentEdges(Vertex* vertex)
{
  assert(vertex != nullptr);

  /*
                   face1

       *-arriving->   *  -leaving->*
    prev<----------vertex<---------next

                   face2
   */

  auto* leaving = vertex->leaving();
  assert(leaving != nullptr);

  // vertex has exactly two incident edges
  assert(leaving != leaving->nextIncident());
  assert(leaving == leaving->nextIncident()->nextIncident());

  // different faces on each side of the leaving edge
  assert(leaving->face() != leaving->twin()->face());

  // only two incident faces in total
  assert(leaving->face() == leaving->previous()->face());
  assert(leaving->twin()->face() == leaving->twin()->next()->face());

  auto* face1 = leaving->face();
  auto* face2 = leaving->twin()->face();

  // each incident face has more than three vertices
  assert(face1->vertexCount() > 3u);
  assert(face2->vertexCount() > 3u);

  auto* arriving = leaving->previous();
  auto* next = leaving->destination();

  auto* edgeToRemove = leaving->edge();

  face2->removeFromBoundary(leaving->twin(), leaving->twin());
  face1->removeFromBoundary(leaving, leaving);

  arriving->twin()->setOrigin(next);
  next->setLeaving(arriving->twin());

  m_edges.remove(edgeToRemove);
  m_vertices.remove(vertex);
}

template <typename T, typename FP, typename VP>
std::string Polyhedron<T, FP, VP>::exportObj() const
{
  auto faces = std::vector<const Face*>{};
  for (const auto* face : m_faces)
  {
    faces.push_back(face);
  }
  return exportObjSelectedFaces(faces);
}

template <typename T, typename FP, typename VP>
std::string Polyhedron<T, FP, VP>::exportObjSelectedFaces(
  const std::vector<const Face*>& faces) const
{
  auto ss = std::stringstream{};
  auto vertices = std::vector<const Vertex*>{};

  for (const auto* current : m_vertices)
  {
    vertices.push_back(current);
  }

  // write the vertices
  for (const auto* v : vertices)
  {
    // vec operator<< prints the vector space delimited
    ss << "v " << v->position() << "\n";
  }

  // write the faces
  for (const auto* face : faces)
  {
    ss << "f ";
    for (const auto* halfEdge : face->boundary())
    {
      const auto* vertex = halfEdge->origin();
      auto indexOptional = kdl::index_of(vertices, vertex);
      assert(indexOptional.has_value());

      // .obj indices are 1-based
      ss << (*indexOptional + 1) << " ";
    }
    ss << "\n";
  }

  return ss.str();
}

} // namespace tb::mdl
