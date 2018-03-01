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

#ifndef Polyhedron_Matcher_h
#define Polyhedron_Matcher_h

#include "Polyhedron.h"
#include "Relation.h"

#include <limits>
#include <list>

template <typename P>
class PolyhedronMatcher {
private:
    typedef typename P::V V;
    typedef typename P::Vertex Vertex;
    typedef typename P::VertexList VertexList;
    typedef typename P::Vertex::Set VertexSet;
    typedef typename P::HalfEdge HalfEdge;
    typedef typename P::Face Face;
    
    typedef relation<Vertex*, Vertex*> VertexRelation;
    
    const P& m_left;
    const P& m_right;
    const VertexRelation m_vertexRelation;
public:
    PolyhedronMatcher(const P& left, const P& right) :
    m_left(left),
    m_right(right),
    m_vertexRelation(buildVertexRelation(m_left, m_right)) {}
    
    PolyhedronMatcher(const P& left, const P& right, const typename V::List& vertices, const V& delta) :
    m_left(left),
    m_right(right),
    m_vertexRelation(buildVertexRelation(m_left, m_right, vertices, delta)) {}

    PolyhedronMatcher(const P& left, const P& right, const typename V::Set& vertices, const V& delta) :
    m_left(left),
    m_right(right),
    m_vertexRelation(buildVertexRelation(m_left, m_right, vertices, delta)) {}
    
    PolyhedronMatcher(const P& left, const P& right, const typename V::Map& vertexMap) :
    m_left(left),
    m_right(right),
    m_vertexRelation(buildVertexRelation(m_left, m_right, vertexMap)) {}
public:
    template <typename Callback>
    void processRightFaces(const Callback& callback) const {
        auto* firstRightFace = m_right.faces().front();
        auto* currentRightFace = firstRightFace;
        do {
            auto* matchingLeftFace = findBestMatchingLeftFace(currentRightFace);
            callback(matchingLeftFace, currentRightFace);
            currentRightFace = currentRightFace->next();
        } while (currentRightFace != firstRightFace);
    }
    
    typedef std::list<Face*> MatchingFaces;
    
    Face* findBestMatchingLeftFace(Face* rightFace) const {
        const auto matchingFaces = findMatchingLeftFaces(rightFace);
        ensure(!matchingFaces.empty(), "No matching face found");

        // Among all matching faces, select one such its normal is the most similar to the given face's normal.
        auto it = std::begin(matchingFaces);
        
        auto* result = *it++;
        auto bestDot = rightFace->normal().dot(result->normal());
        
        while (it != std::end(matchingFaces) && bestDot < 1.0) {
            auto* currentFace = *it;
            const auto dot = rightFace->normal().dot(currentFace->normal());
            if (dot > bestDot) {
                result = currentFace;
                bestDot = dot;
            }
            ++it;
        }
        
        return result;
    }

    /*
     * Returns all left faces which have a maximal matching score with the given right face.
     */
    MatchingFaces findMatchingLeftFaces(Face* rightFace) const {
        MatchingFaces result;
        size_t bestMatchScore = 0;
        
        auto* firstLeftFace = m_left.faces().front();
        auto* currentLeftFace = firstLeftFace;
        do {
            const auto matchScore = computeMatchScore(currentLeftFace, rightFace);
            if (matchScore > bestMatchScore) {
                result.clear();
                result.push_back(currentLeftFace);
                bestMatchScore = matchScore;
            } else if (matchScore == bestMatchScore) {
                result.push_back(currentLeftFace);
            }
            currentLeftFace = currentLeftFace->next();
        } while (currentLeftFace != firstLeftFace);
        
        return result;
    }
private:
    /*
     * Computes the matching score between the given two faces using the data stored in the vertex relation.
     *
     * The matching score between the given faces is the number of all pairs of a vertex of the given left face
     * and a vertex of the given right face which are also in the vertex relation, unless the faces are identical.
     * In that case, this function returns a perfect match score.
     */
    size_t computeMatchScore(Face* leftFace, Face* rightFace) const {
        if (leftFace->vertexCount() == rightFace->vertexCount() && leftFace->hasVertexPositions(rightFace->vertexPositions())) {
            return std::numeric_limits<size_t>::max();
        }

        size_t result = 0;
        
        auto* firstLeftEdge = leftFace->boundary().front();
        auto* firstRightEdge = rightFace->boundary().front();

        auto* currentLeftEdge = firstLeftEdge;
        do {
            auto* leftVertex = currentLeftEdge->origin();

            auto* currentRightEdge = firstRightEdge;
            do {
                auto* rightVertex = currentRightEdge->origin();

                if (m_vertexRelation.contains(leftVertex, rightVertex))
                    ++result;

                currentRightEdge = currentRightEdge->next();
            } while (currentRightEdge != firstRightEdge);

            currentLeftEdge = currentLeftEdge->next();
        } while (currentLeftEdge != firstLeftEdge);
        
        return result;
    }
private:
    static VertexRelation buildVertexRelation(const P& left, const P& right) {
        VertexRelation result;
        
        auto* firstLeftVertex = left.vertices().front();
        auto* currentLeftVertex = firstLeftVertex;
        do {
            const auto& position = currentLeftVertex->position();
            auto* currentRightVertex = right.findVertexByPosition(position);
            if (currentRightVertex != nullptr)
                result.insert(currentLeftVertex, currentRightVertex);
            
            currentLeftVertex = currentLeftVertex->next();
        } while (currentLeftVertex != firstLeftVertex);

        return expandVertexRelation(left, right, result);
    }
    
    static VertexRelation buildVertexRelation(const P& left, const P& right, const typename V::List& vertices, const V& delta) {
        return buildVertexRelation(left, right, typename V::Set(std::begin(vertices), std::end(vertices)), delta);
    }
    
    static VertexRelation buildVertexRelation(const P& left, const P& right, const typename V::Set& vertices, const V& delta) {
        typename V::Map vertexMap;
        
        auto* firstVertex = left.vertices().front();
        auto* currentVertex = firstVertex;
        do {
            const auto& position = currentVertex->position();
            if (vertices.count(position) == 0) {
                if (right.hasVertex(position))
                    vertexMap.insert(std::make_pair(position, position));
            } else {
                assert(right.hasVertex(position + delta));
                vertexMap.insert(std::make_pair(position, position + delta));
            }
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        
        return buildVertexRelation(left, right, vertexMap);
    }
    
    static VertexRelation buildVertexRelation(const P& left, const P& right, const typename V::Map& vertexMap) {
        VertexRelation result;
        
        for (const auto& entry : vertexMap) {
            const auto& leftPosition = entry.first;
            const auto& rightPosition = entry.second;
            
            auto* leftVertex = left.findVertexByPosition(leftPosition);
            auto* rightVertex = right.findVertexByPosition(rightPosition);
            
            assert(leftVertex != nullptr);
            assert(rightVertex != nullptr);
            result.insert(leftVertex, rightVertex);
        }

        return expandVertexRelation(left, right, result);
    }
    
    static VertexRelation expandVertexRelation(const P& left, const P& right, const VertexRelation& initialRelation) {
        auto result = initialRelation;
        result.insert(addedVertexRelation(right, initialRelation));
        result.insert(removedVertexRelation(left, initialRelation));
        return result;
    }
    
    static VertexRelation addedVertexRelation(const P& right, const VertexRelation& initialRelation) {
        const auto addedVertices = findAddedVertices(right, initialRelation);

        auto result = initialRelation;
        size_t previousSize;
        do {
            previousSize = result.size();
            for (auto* addedVertex : addedVertices) {
                // consider all adjacent vertices
                auto* firstEdge = addedVertex->leaving();
                auto* currentEdge = firstEdge;
                do {
                    auto* neighbour = currentEdge->destination();
                    result.insert(result.left_range(neighbour), addedVertex);
                    currentEdge = currentEdge->nextIncident();
                } while (currentEdge != firstEdge);
            }
        } while (result.size() > previousSize);
        
        return result;
    }
    
    static VertexRelation removedVertexRelation(const P& left, const VertexRelation& initialRelation) {
        const auto removedVertices = findRemovedVertices(left, initialRelation);

        auto result = initialRelation;
        size_t previousSize;
        do {
            previousSize = result.size();
            for (auto* removedVertex : removedVertices) {
                // consider all adjacent vertices
                auto* firstEdge = removedVertex->leaving();
                auto* currentEdge = firstEdge;
                do {
                    auto* neighbour = currentEdge->destination();
                    result.insert(removedVertex, result.right_range(neighbour));
                    currentEdge = currentEdge->nextIncident();
                } while (currentEdge != firstEdge);
            }
        } while (result.size() > previousSize);
        
        return result;
    }
    
    static VertexSet findAddedVertices(const P& right, const VertexRelation& vertexRelation) {
        VertexSet result;
        
        const auto& rightVertices = right.vertices();
        auto* firstVertex = rightVertices.front();
        auto* currentVertex = firstVertex;
        do {
            if (vertexRelation.count_left(currentVertex) == 0)
                result.insert(currentVertex);
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        
        return result;
    }
    
    static VertexSet findRemovedVertices(const P& left, const VertexRelation& vertexRelation) {
        VertexSet result;
        
        const auto& leftVertices = left.vertices();
        auto* firstVertex = leftVertices.front();
        auto* currentVertex = firstVertex;
        do {
            if (vertexRelation.count_right(currentVertex) == 0)
                result.insert(currentVertex);
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        
        return result;
    }
};

#endif /* Polyhedron_Matcher_h */
