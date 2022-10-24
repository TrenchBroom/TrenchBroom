/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Polyhedron.h"

namespace TrenchBroom
{
namespace Model
{
template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkInvariant() const
{
  /*
   if (!checkConvex())
   return false;
   */
  if (!checkComponentCounts())
    return false;
  if (!checkEulerCharacteristic())
    return false;
  if (!checkVertices())
    return false;
  if (!checkFaceBoundaries())
    return false;
  if (!checkFaceNeighbours())
    return false;
  if (!checkOverlappingFaces())
    return false;
  if (!checkVertexLeavingEdges())
    return false;
  if (!checkClosed())
    return false;
  if (!checkNoDegenerateFaces())
    return false;
  if (!checkEdges())
    return false;
  /* This check leads to false positive with almost coplanar faces.
   if (polyhedron() && !checkNoCoplanarFaces())
   return false;
   */
  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkComponentCounts() const
{
  if (vertexCount() == 0u && edgeCount() == 0u && faceCount() == 0u)
    return true; // empty
  if (vertexCount() == 1u && edgeCount() == 0u && faceCount() == 0u)
    return true; // point
  if (vertexCount() == 2u && edgeCount() == 1u && faceCount() == 0u)
    return true; // edge
  if (vertexCount() >= 3u && edgeCount() >= 3u && faceCount() == 1u)
    return true; // polygon
  if (vertexCount() >= 4u && edgeCount() >= 6u && faceCount() >= 4u)
    return true; // polyhedron
  return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkEulerCharacteristic() const
{
  if (!polyhedron())
    return true;

  // See https://en.m.wikipedia.org/wiki/Euler_characteristic
  return vertexCount() + faceCount() - edgeCount() == 2;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkVertices() const
{
  const auto countIncidentEdges = [](const Vertex* vertex) -> size_t {
    if (vertex->leaving() == nullptr)
    {
      return 0u;
    }

    size_t count = 0u;
    HalfEdge* halfEdge = vertex->leaving();
    do
    {
      ++count;
      halfEdge = halfEdge->nextIncident();
    } while (halfEdge != vertex->leaving());
    return count;
  };

  if (polyhedron())
  {
    for (const Vertex* vertex : m_vertices)
    {
      if (countIncidentEdges(vertex) < 3u)
      {
        return false;
      }
    }
  }
  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkOverlappingFaces() const
{
  if (!polyhedron())
  {
    return true;
  }

  for (auto it1 = std::begin(m_faces), end = std::end(m_faces); it1 != end; ++it1)
  {
    for (auto it2 = std::next(it1); it2 != end; ++it2)
    {
      const std::size_t sharedVertexCount = (*it1)->countSharedVertices(*it2);
      if (sharedVertexCount > 2u)
      {
        return false;
      }
    }
  }
  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkFaceBoundaries() const
{
  if (m_faces.empty())
  {
    return true;
  }

  for (const Face* face : m_faces)
  {
    for (const HalfEdge* edge : face->boundary())
    {
      if (edge->face() != face)
      {
        return false;
      }
      if (edge->edge() == nullptr)
      {
        return false;
      }
      if (!m_edges.contains(edge->edge()))
      {
        return false;
      }
      if (!m_vertices.contains(edge->origin()))
      {
        return false;
      }
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkFaceNeighbours() const
{
  if (!polyhedron())
  {
    return true;
  }

  for (const Face* face : m_faces)
  {
    for (const HalfEdge* edge : face->boundary())
    {
      HalfEdge* twin = edge->twin();
      if (twin == nullptr)
      {
        return false;
      }
      if (twin->face() == nullptr)
      {
        return false;
      }
      if (!m_faces.contains(twin->face()))
      {
        return false;
      }
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkConvex() const
{
  if (!polyhedron())
  {
    return true;
  }

  for (const Face* face : m_faces)
  {
    for (const Vertex* vertex : m_vertices)
    {
      if (
        face->pointStatus(vertex->position(), vm::constants<T>::point_status_epsilon())
        == vm::plane_status::above)
      {
        return false;
      }
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkClosed() const
{
  if (!polyhedron())
  {
    return true;
  }

  for (const Edge* edge : m_edges)
  {
    if (!edge->fullySpecified())
    {
      return false;
    }
    else if (!m_faces.contains(edge->firstFace()))
    {
      return false;
    }
    else if (!m_faces.contains(edge->secondFace()))
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkNoCoplanarFaces() const
{
  if (!polyhedron())
    return true;

  for (const Edge* edge : m_edges)
  {
    const Face* firstFace = edge->firstFace();
    const Face* secondFace = edge->secondFace();

    if (firstFace == secondFace)
    {
      return false;
    }
    if (firstFace->coplanar(secondFace, vm::constants<T>::point_status_epsilon()))
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkNoDegenerateFaces() const
{
  if (!polyhedron())
  {
    return true;
  }

  for (const Face* face : m_faces)
  {
    if (face->vertexCount() < 3u)
    {
      return false;
    }

    for (const HalfEdge* halfEdge : face->boundary())
    {
      const Edge* edge = halfEdge->edge();
      if (edge == nullptr || !edge->fullySpecified())
      {
        return false;
      }
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkVertexLeavingEdges() const
{
  if (empty() || point())
  {
    return true;
  }

  for (const Vertex* vertex : m_vertices)
  {
    const HalfEdge* leaving = vertex->leaving();
    if (leaving == nullptr)
    {
      return false;
    }
    if (leaving->origin() != vertex)
    {
      return false;
    }
    if (!point())
    {
      const Edge* edge = leaving->edge();
      if (edge == nullptr)
      {
        return false;
      }
      if (!m_edges.contains(edge))
      {
        return false;
      }
      if (polyhedron() && !edge->fullySpecified())
      {
        return false;
      }
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkEdges() const
{
  if (!polyhedron())
  {
    return true;
  }

  for (const Edge* currentEdge : m_edges)
  {
    if (!currentEdge->fullySpecified())
    {
      return false;
    }
    Face* firstFace = currentEdge->firstFace();
    if (firstFace == nullptr)
    {
      return false;
    }
    if (!m_faces.contains(firstFace))
    {
      return false;
    }

    Face* secondFace = currentEdge->secondFace();
    if (secondFace == nullptr)
    {
      return false;
    }
    if (!m_faces.contains(secondFace))
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkEdgeLengths(const T minLength) const
{
  if (m_edges.empty())
    return true;

  const T minLength2 = minLength * minLength;
  for (const Edge* edge : m_edges)
  {
    const T length2 = vm::squared_length(edge->vector());
    if (length2 < minLength2)
    {
      return false;
    }
  }

  return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T, FP, VP>::checkLeavingEdges(const Vertex* v) const
{
  assert(v != nullptr);
  const HalfEdge* firstEdge = v->leaving();
  assert(firstEdge != nullptr);
  const HalfEdge* curEdge = firstEdge;

  do
  {
    const HalfEdge* nextEdge = curEdge->nextIncident();
    do
    {
      if (curEdge->destination() == nextEdge->destination())
      {
        return false;
      }
      nextEdge = nextEdge->nextIncident();
    } while (nextEdge != firstEdge);

    curEdge = curEdge->nextIncident();
  } while (curEdge->nextIncident() != firstEdge);

  return true;
}
} // namespace Model
} // namespace TrenchBroom
