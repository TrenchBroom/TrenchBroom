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

#pragma once

#include "Ensure.h"
#include "Polyhedron.h"

#include "kdl/binary_relation.h"
#include "kdl/vector_set.h"
#include "kdl/vector_utils.h"

#include <limits>
#include <map>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
/**
 * This template is used to match the faces of two polyhedra. The two polyhedra are
 * expected to have the majority of their vertices in common as a result of a vertex move
 * / addition / removal operation.
 *
 * Two faces match if they have identical vertex positions or if they have an optimal
 * matching score. The matching score is based on a relation over the vertices of the left
 * and the vertices of the right polyhedron. The score of two faces is then the sum of all
 * pairs of related vertices (l,r), where l is a vertex of the left face L, and r is a
 * vertex of the right face R. Two vertices (l,r) are related if any of the following
 * conditions apply:
 *
 * 1. l and r have identical positions
 * 2. There is no vertex in the right polyhedron that corresponds to l, but there is a
 * vertex l' in the left polyhedron such that (l',r) are related, and l and l' are
 * adjacent in the left polyhedron.
 * 3. There is no vertex in the left polyhedron that corresponds to r, but there is a
 * vertex r' in the right polyhedron such that (l,r') are related, and r and r' are
 * adjacent in the right polyhedron.
 *
 * Case 2. corresponds to a vertex removal, that is, a vertex was removed from the left
 * polyhedron. Case 3. corresponds to a vertex addition, that is, a vertex was added to
 * the right polyhedron. If a vertex is moved, both cases apply since the move can be
 * regarded as a vertex removal and a subsequent addition.
 *
 * Using this relation over the vertices, the matcher will find the best matching face
 * from the left polyhedron for each face of the right polyhedron. If multiple faces of
 * the left polyhedron have a maximal matching score, the matcher selects a face such that
 * its normal is closest to the normal of the right face.
 */
template <typename P>
class PolyhedronMatcher
{
private:
  using V = vm::vec<typename P::FloatType, 3u>;
  using Vertex = typename P::Vertex;
  using VertexList = typename P::VertexList;
  using VertexSet = std::set<Vertex*>;
  using HalfEdge = typename P::HalfEdge;
  using Face = typename P::Face;
  using VMap = std::map<V, V>;

  using VertexRelation = kdl::binary_relation<Vertex*, Vertex*>;

  const P& m_left;
  const P& m_right;
  const VertexRelation m_vertexRelation;

public:
  PolyhedronMatcher(const P& left, const P& right)
    : m_left(left)
    , m_right(right)
    , m_vertexRelation(buildVertexRelation(m_left, m_right))
  {
  }

  PolyhedronMatcher(
    const P& left, const P& right, const std::vector<V>& vertices, const V& delta)
    : m_left(left)
    , m_right(right)
    , m_vertexRelation(buildVertexRelation(m_left, m_right, vertices, delta))
  {
  }

  PolyhedronMatcher(const P& left, const P& right, const VMap& vertexMap)
    : m_left(left)
    , m_right(right)
    , m_vertexRelation(buildVertexRelation(m_left, m_right, vertexMap))
  {
  }

public:
  /**
   * Apply the given callback function to each pair of matching faces. The algorithm
   * iterates over all faces of the right polyhedron and finds the best matching face of
   * the left polyhedron. Then it applies the given function to that pair of faces.
   *
   * @tparam Callback the type of the callback function
   * @param callback the callback function to apply to each pair of matching faces
   */
  template <typename Callback>
  void processRightFaces(const Callback& callback) const
  {
    auto* firstRightFace = m_right.faces().front();
    auto* currentRightFace = firstRightFace;
    do
    {
      auto* matchingLeftFace = findBestMatchingLeftFace(currentRightFace);
      callback(matchingLeftFace, currentRightFace);
      currentRightFace = currentRightFace->next();
    } while (currentRightFace != firstRightFace);
  }

  using MatchingFaces = std::vector<Face*>;

private:
  /**
   * Find the best matching face from the left polyhedron for the given face of the right
   * polyhedron. The best match is determined using the matching score (see function
   * findMatchingLeftFaces). If multiple faces of the left polyhedron have a maximal
   * matching score with the given face of the right polyhedron, this function selects a
   * face based upon the dot products of the face normals.
   *
   * @param rightFace the face of the right polyhedron to find a match for
   * @return a best matching face of the left polyhedron
   */
  Face* findBestMatchingLeftFace(Face* rightFace) const
  {
    const auto matchingFaces = findMatchingLeftFaces(rightFace);
    ensure(!matchingFaces.empty(), "No matching face found");

    // Among all matching faces, select one such its normal is the most similar to the
    // given face's normal.
    auto it = std::begin(matchingFaces);

    auto* result = *it++;
    auto bestDot = dot(rightFace->normal(), result->normal());

    // exit early if we find a face with an identical normal
    while (it != std::end(matchingFaces) && bestDot < 1.0)
    {
      auto* currentFace = *it;
      const auto currentDot = dot(rightFace->normal(), currentFace->normal());
      if (currentDot > bestDot)
      {
        result = currentFace;
        bestDot = currentDot;
      }
      ++it;
    }

    return result;
  }

  /**
   * Find all faces of the left polyhedron that have a maximal matching score with the
   * given face of the right polyhedron.
   *
   * @param rightFace the face of the right polyhedron
   * @return the matching faces of the left polyhedron
   */
  MatchingFaces findMatchingLeftFaces(Face* rightFace) const
  {
    MatchingFaces result;
    size_t bestMatchScore = 0;

    auto* firstLeftFace = m_left.faces().front();
    auto* currentLeftFace = firstLeftFace;
    do
    {
      const auto matchScore = computeMatchScore(currentLeftFace, rightFace);
      if (matchScore > bestMatchScore)
      {
        result.clear();
        result.push_back(currentLeftFace);
        bestMatchScore = matchScore;
      }
      else if (matchScore == bestMatchScore)
      {
        result.push_back(currentLeftFace);
      }
      currentLeftFace = currentLeftFace->next();
    } while (currentLeftFace != firstLeftFace);

    return result;
  }

public:
  /**
   * Visits all pairs of vertices in the vertex relation where the left vertex is in the
   * given leftFace and the right vertex is in the given rightFace.
   *
   * @tparam L visitor type
   * @param leftFace a face of the left polyhedron
   * @param rightFace a face of the right polyhedron
   * @param lambda visitor to run on each pair of vertices
   */
  template <typename L>
  void visitMatchingVertexPairs(Face* leftFace, Face* rightFace, L&& lambda) const
  {
    auto* firstLeftEdge = leftFace->boundary().front();
    auto* firstRightEdge = rightFace->boundary().front();

    auto* currentLeftEdge = firstLeftEdge;
    do
    {
      auto* leftVertex = currentLeftEdge->origin();

      auto* currentRightEdge = firstRightEdge;
      do
      {
        auto* rightVertex = currentRightEdge->origin();

        if (m_vertexRelation.contains(leftVertex, rightVertex))
        {
          lambda(leftVertex, rightVertex);
        }

        currentRightEdge = currentRightEdge->next();
      } while (currentRightEdge != firstRightEdge);

      currentLeftEdge = currentLeftEdge->next();
    } while (currentLeftEdge != firstLeftEdge);
  }

private:
  /**
   * Computes the matching score between the given left and right faces the data stored in
   * the vertex relation.
   *
   * The matching score between the given faces is the number of all pairs of a vertex of
   * the given left face and a vertex of the given right face which are also in the vertex
   * relation, unless the faces are identical. In that case, this function returns a
   * perfect match score.
   *
   * @param leftFace a face of the left polyhedron
   * @param rightFace a face of the right polyhedron
   * @return the matching score
   */
  size_t computeMatchScore(Face* leftFace, Face* rightFace) const
  {
    if (
      leftFace->vertexCount() == rightFace->vertexCount()
      && leftFace->hasVertexPositions(rightFace->vertexPositions()))
    {
      return std::numeric_limits<size_t>::max();
    }

    size_t result = 0;
    visitMatchingVertexPairs(
      leftFace,
      rightFace,
      [&result](Vertex* /* leftVertex */, Vertex* /* rightVertex */) { ++result; });
    return result;
  }

private:
  /**
   * Build the vertex relation for the given left and right polyhedra.
   *
   * The relation is built by inserting every pair of vertices (l,r) such that l is a
   * vertex of the left polyhedron and r is a vertex of the given right polyhedron and
   * (l,r) have identical positions. Then, the relation is expanded by calling the
   * expandVertexRelation function.
   *
   * @param left the left polyhedron
   * @param right the right polyhedron
   * @return the vertex relation
   */
  static VertexRelation buildVertexRelation(const P& left, const P& right)
  {
    VertexRelation result;

    auto* firstLeftVertex = left.vertices().front();
    auto* currentLeftVertex = firstLeftVertex;
    do
    {
      const auto& position = currentLeftVertex->position();
      auto* currentRightVertex = right.findVertexByPosition(position);
      if (currentRightVertex != nullptr)
      {
        result.insert(currentLeftVertex, currentRightVertex);
      }

      currentLeftVertex = currentLeftVertex->next();
    } while (currentLeftVertex != firstLeftVertex);

    return expandVertexRelation(left, right, result);
  }

  /**
   * Builds the vertex relation for a pair of polyhedra such that the right polyhedron is
   * the result of moving the given vertices of the left polyhedron by the given delta.
   *
   * The function accounts for the moved vertices when attempting to find a vertex of the
   * left polyhedron in the right polyhedron. If a vertex v is in the given set of moved
   * vertices, then the algorithm attempts to find it at its new position in the right
   * polyhedron.
   *
   * @param left the left polyhedron
   * @param right the right polyhedron
   * @param vertices the vertices that have been moved
   * @param delta the move delta
   * @return the vertex relation
   */
  static VertexRelation buildVertexRelation(
    const P& left, const P& right, const std::vector<V>& vertices, const V& delta)
  {
    VMap vertexMap;
    const auto vertexSet = kdl::vector_set<V>::create(vertices);

    auto* firstVertex = left.vertices().front();
    auto* currentVertex = firstVertex;
    do
    {
      const auto& position = currentVertex->position();
      // vertices are expected to be exact positions of vertices in left, whereas the
      // vertex positions searched for in right allow an epsilon of
      // vm::Constants<T>::almost_zero()
      if (vertexSet.count(position) > 0u)
      {
        if (right.hasVertex(position))
        {
          vertexMap.insert(std::make_pair(position, position));
        }
      }
      else
      {
        assert(right.hasVertex(position + delta));
        vertexMap.insert(std::make_pair(position, position + delta));
      }
      currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);

    return buildVertexRelation(left, right, vertexMap);
  }

  /**
   * Helper function to build a vertex relation using the given set of corresponding
   * vertices.
   *
   * @param left the left polyhedron
   * @param right the right polyhedron
   * @param vertexMap a set of corresponding vertices for which to build the relation
   * @return the vertex relation
   */
  static VertexRelation buildVertexRelation(
    const P& left, const P& right, const VMap& vertexMap)
  {
    VertexRelation result;

    for (const auto& [leftPosition, rightPosition] : vertexMap)
    {
      auto* leftVertex = left.findVertexByPosition(leftPosition);
      auto* rightVertex = right.findVertexByPosition(rightPosition);

      assert(leftVertex != nullptr);
      assert(rightVertex != nullptr);
      result.insert(leftVertex, rightVertex);
    }

    return expandVertexRelation(left, right, result);
  }

  /**
   * Expand the given vertex relation of vertices of the given left and right polyhedra.
   * Expanding a vertex relation is based on the given initial relation, and expands the
   * relation by those vertices present only in the right polyhedron and by those vertices
   * present only in the left polyhedron.
   *
   * @param left the left polyhedron
   * @param right the right polyhedron
   * @param initialRelation the initial vertex relation
   * @return the expanded vertex relation
   */
  static VertexRelation expandVertexRelation(
    const P& left, const P& right, const VertexRelation& initialRelation)
  {
    auto result = initialRelation;
    result.insert(addedVertexRelation(right, initialRelation));
    result.insert(removedVertexRelation(left, initialRelation));
    return result;
  }

  /**
   * Returns a vertex relation that includes only vertices present in the given right
   * polyhedron, but not in the given vertex relation. Such a vertex is related to the
   * vertices which are related to its neighbours as follows.
   *
   * Let r be a vertex present in the given polyhedron that has no related vertices in the
   * given relation. Let r' be adjacent to r in the given polyhedron and let r' be related
   * to l in the given relation. l is a vertex of the left polyhedron under treatment.
   * Then the pair (l,r) is added to the resulting relation.
   *
   * Note that the resulting relation does not include the given initial relation.
   *
   * @param right the polyhedron with additional vertices
   * @param initialRelation the initial vertex relation
   * @return a vertex relation containing only the newly discovered related vertices
   */
  static VertexRelation addedVertexRelation(
    const P& right, const VertexRelation& initialRelation)
  {
    const auto addedVertices = findAddedVertices(right, initialRelation);

    auto result = initialRelation;
    size_t previousSize;
    do
    {
      previousSize = result.size();
      for (auto* addedVertex : addedVertices)
      {
        // consider all adjacent vertices
        auto* firstEdge = addedVertex->leaving();
        auto* currentEdge = firstEdge;
        do
        {
          auto* neighbour = currentEdge->destination();
          result.insert(result.left_range(neighbour), addedVertex);
          currentEdge = currentEdge->nextIncident();
        } while (currentEdge != firstEdge);
      }
    } while (result.size() > previousSize);

    return result;
  }

  /**
   * Returns a vertex relation that includes only vertices present in the given left
   * polyhedron, but not in the given vertex relation. Such a vertex is related to the
   * vertices which are related to its neighbours as follows.
   *
   * Let l be a vertex present in the given polyhedron that has no related vertices in the
   * given relation. Let l' be adjacent to l in the given polyhedron and let l' be related
   * to r in the given relation. r is a vertex of the left polyhedron under treatment.
   * Then the pair (l,r) is added to the resulting relation.
   *
   * Note that the resulting relation does not include the given initial relation.
   *
   * @param left the polyhedron with removed vertices
   * @param initialRelation the initial vertex relation
   * @return a vertex relation containing only the newly discovered related vertices
   */
  static VertexRelation removedVertexRelation(
    const P& left, const VertexRelation& initialRelation)
  {
    const auto removedVertices = findRemovedVertices(left, initialRelation);

    auto result = initialRelation;
    size_t previousSize;
    do
    {
      previousSize = result.size();
      for (auto* removedVertex : removedVertices)
      {
        // consider all adjacent vertices
        auto* firstEdge = removedVertex->leaving();
        auto* currentEdge = firstEdge;
        do
        {
          auto* neighbour = currentEdge->destination();
          result.insert(removedVertex, result.right_range(neighbour));
          currentEdge = currentEdge->nextIncident();
        } while (currentEdge != firstEdge);
      }
    } while (result.size() > previousSize);

    return result;
  }

  /**
   * Returns a set containing only those vertices of the given polyhedron for which there
   * is no entry in the given vertex relation. These are vertices which have been added to
   * the given polyhedron.
   *
   * @param right the polyhedron
   * @param vertexRelation the vertex relation
   * @return the added vertices
   */
  static VertexSet findAddedVertices(const P& right, const VertexRelation& vertexRelation)
  {
    VertexSet result;

    const auto& rightVertices = right.vertices();
    auto* firstVertex = rightVertices.front();
    auto* currentVertex = firstVertex;
    do
    {
      if (vertexRelation.count_left(currentVertex) == 0)
        result.insert(currentVertex);
      currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);

    return result;
  }

  /**
   * Returns a set containing only those vertices of the given polyhedron for there is no
   * entry in the given vertex relation. These are vertices which have been removed from
   * the given polyhedron.
   *
   * @param left the polyhedron
   * @param vertexRelation the vertex relation
   * @return the removed vertices
   */
  static VertexSet findRemovedVertices(
    const P& left, const VertexRelation& vertexRelation)
  {
    VertexSet result;

    const auto& leftVertices = left.vertices();
    auto* firstVertex = leftVertices.front();
    auto* currentVertex = firstVertex;
    do
    {
      if (vertexRelation.count_right(currentVertex) == 0)
        result.insert(currentVertex);
      currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);

    return result;
  }
};
} // namespace Model
} // namespace TrenchBroom
