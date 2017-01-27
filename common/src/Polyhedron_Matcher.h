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
        Face* firstRightFace = m_right.faces().front();
        Face* currentRightFace = firstRightFace;
        do {
            Face* matchingLeftFace = findBestMatchingLeftFace(currentRightFace);
            callback(matchingLeftFace, currentRightFace);
            currentRightFace = currentRightFace->next();
        } while (currentRightFace != firstRightFace);
    }
    
    typedef std::list<Face*> MatchingFaces;
    
    Face* findBestMatchingLeftFace(Face* rightFace) const {
        const MatchingFaces matchingFaces = findMatchingLeftFaces(rightFace);
        ensure(!matchingFaces.empty(), "No matching face found");
        
        typename MatchingFaces::const_iterator it;
        it = std::begin(matchingFaces);
        
        Face* result = *it++;
        FloatType bestDot = rightFace->normal().dot(result->normal());
        
        while (it != std::end(matchingFaces)) {
            Face* currentFace = *it;
            const FloatType dot = rightFace->normal().dot(currentFace->normal());
            if (dot < bestDot) {
                result = currentFace;
                bestDot = dot;
            }
            ++it;
        }
        
        return result;
    }
    
    MatchingFaces findMatchingLeftFaces(Face* rightFace) const {
        MatchingFaces result;
        size_t bestMatchScore = 0;
        
        Face* firstLeftFace = m_left.faces().front();
        Face* currentLeftFace = firstLeftFace;
        do {
            const size_t matchScore = computeMatchScore(currentLeftFace, rightFace);
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
    size_t computeMatchScore(Face* leftFace, Face* rightFace) const {
        size_t result = 0;
        
        HalfEdge* firstEdge = leftFace->boundary().front();
        HalfEdge* currentEdge = firstEdge;
        do {
            Vertex* leftVertex = currentEdge->origin();
            typename VertexRelation::const_right_range right = m_vertexRelation.right_range(leftVertex);
            
            typename VertexRelation::const_right_iterator it = right.first;
            while (it != right.second) {
                Vertex* rightVertex = *it;
                if (rightVertex->incident(rightFace))
                    ++result;
                ++it;
            }
            currentEdge = currentEdge->next();
        } while (currentEdge != firstEdge);
        
        return result;
    }
private:
    static VertexRelation buildVertexRelation(const P& left, const P& right) {
        VertexRelation result;
        
        Vertex* firstLeftVertex = left.vertices().front();
        Vertex* currentLeftVertex = firstLeftVertex;
        do {
            const V& position = currentLeftVertex->position();
            Vertex* currentRightVertex = right.findVertexByPosition(position);
            if (currentRightVertex != NULL)
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
        
        Vertex* firstVertex = left.vertices().front();
        Vertex* currentVertex = firstVertex;
        do {
            const V& position = currentVertex->position();
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
            const V& leftPosition = entry.first;
            const V& rightPosition = entry.second;
            
            Vertex* leftVertex = left.findVertexByPosition(leftPosition);
            Vertex* rightVertex = right.findVertexByPosition(rightPosition);
            
            assert(leftVertex != NULL);
            assert(rightVertex != NULL);
            result.insert(leftVertex, rightVertex);
        }

        return expandVertexRelation(left, right, result);
    }
    
    static VertexRelation expandVertexRelation(const P& left, const P& right, const VertexRelation& initialRelation) {
        VertexRelation result = initialRelation;
        result.insert(addedVertexRelation(right, initialRelation));
        result.insert(removedVertexRelation(left, initialRelation));
        return result;
    }
    
    static VertexRelation addedVertexRelation(const P& right, const VertexRelation& initialRelation) {
        const VertexSet addedVertices = findAddedVertices(right, initialRelation);

        VertexRelation result = initialRelation;
        size_t previousSize;
        do {
            previousSize = result.size();
            for (Vertex* addedVertex : addedVertices) {
                // consider all adjacent vertices
                HalfEdge* firstEdge = addedVertex->leaving();
                HalfEdge* currentEdge = firstEdge;
                do {
                    Vertex* neighbour = currentEdge->destination();
                    result.insert(result.left_range(neighbour), addedVertex);
                    currentEdge = currentEdge->nextIncident();
                } while (currentEdge != firstEdge);
            }
        } while (result.size() > previousSize);
        
        return result;
    }
    
    static VertexRelation removedVertexRelation(const P& left, const VertexRelation& initialRelation) {
        const VertexSet removedVertices = findRemovedVertices(left, initialRelation);

        VertexRelation result = initialRelation;
        size_t previousSize;
        do {
            previousSize = result.size();
            for (Vertex* removedVertex : removedVertices) {
                // consider all adjacent vertices
                HalfEdge* firstEdge = removedVertex->leaving();
                HalfEdge* currentEdge = firstEdge;
                do {
                    Vertex* neighbour = currentEdge->destination();
                    result.insert(removedVertex, result.right_range(neighbour));
                    currentEdge = currentEdge->nextIncident();
                } while (currentEdge != firstEdge);
            }
        } while (result.size() > previousSize);
        
        return result;
    }
    
    static VertexSet findAddedVertices(const P& right, const VertexRelation& vertexRelation) {
        VertexSet result;
        
        const VertexList& rightVertices = right.vertices();
        Vertex* firstVertex = rightVertices.front();
        Vertex* currentVertex = firstVertex;
        do {
            if (vertexRelation.count_left(currentVertex) == 0)
                result.insert(currentVertex);
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        
        return result;
    }
    
    static VertexSet findRemovedVertices(const P& left, const VertexRelation& vertexRelation) {
        VertexSet result;
        
        const VertexList& leftVertices = left.vertices();
        Vertex* firstVertex = leftVertices.front();
        Vertex* currentVertex = firstVertex;
        do {
            if (vertexRelation.count_right(currentVertex) == 0)
                result.insert(currentVertex);
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        
        return result;
    }
};

#endif /* Polyhedron_Matcher_h */
