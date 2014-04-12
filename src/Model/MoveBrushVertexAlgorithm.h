/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__MoveBrushVertexAlgorithm__
#define __TrenchBroom__MoveBrushVertexAlgorithm__

#include "Model/BrushAlgorithm.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Macros.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <cassert>
#include <map>

namespace TrenchBroom {
    namespace Model {
        template <typename R>
        class MoveBrushVertexAlgorithm : public BrushAlgorithm<R> {
        private:
            class FaceManager {
            private:
                typedef std::map<BrushFace*, BrushFaceSet> CopyMap;
                CopyMap m_newFaces;
                BrushFaceSet m_droppedFaces;
            public:
                ~FaceManager() {
                    CopyMap::iterator mapIt, mapEnd;
                    for (mapIt = m_newFaces.begin(), mapEnd = m_newFaces.end(); mapIt != mapEnd; ++mapIt) {
                        BrushFaceSet& faces = mapIt->second;
                        BrushFaceSet::iterator faceIt, faceEnd;
                        for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt)
                            delete *faceIt;
                    }
                }
                
                void addFace(BrushFace* original, BrushFace* copy) {
                    assert(original != NULL);
                    assert(copy != NULL);
                    assert(original != copy);
                    m_newFaces[original].insert(copy);
                }
                
                void dropFace(BrushFaceGeometry* side) {
                    assert(side != NULL);
                    assert(side->face != NULL);
                    
                    CopyMap::iterator copyIt = m_newFaces.find(side->face);
                    if (copyIt != m_newFaces.end()) {
                        // the face is an original
                        BrushFaceSet& copies = copyIt->second;
                        assert(!copies.empty());
                        
                        BrushFaceSet::iterator faceIt = copies.begin();
                        BrushFace* copy = *faceIt;
                        copies.erase(faceIt);
                        
                        BrushFaceGeometry* copySide = copy->side();
                        copySide->face = side->face;
                        copySide->face->setSide(copySide);
                        
                        if (copies.empty())
                            m_newFaces.erase(copyIt);
                        
                        delete copy;
                    } else {
                        bool wasCopy = false;
                        CopyMap::iterator copyEnd;
                        for (copyIt = m_newFaces.begin(), copyEnd = m_newFaces.end(); copyIt != copyEnd; ++copyIt) {
                            BrushFaceSet& copies = copyIt->second;
                            assert(!copies.empty());
                            
                            if (copies.erase(side->face) > 0) {
                                wasCopy = true;
                                if (copies.empty())
                                    m_newFaces.erase(copyIt);
                                break;
                            }
                        }
                        if (!wasCopy)
                            m_droppedFaces.insert(side->face);
                    }
                    side->face = NULL;
                }
                
                void getFaces(BrushFaceList& newFaces, BrushFaceList& droppedFaces) {
                    newFaces.clear();
                    droppedFaces.clear();
                    
                    CopyMap::const_iterator it, end;
                    for (it = m_newFaces.begin(), end = m_newFaces.end(); it != end; ++it) {
                        const BrushFaceSet& actualNewFaces = it->second;
                        assert(!actualNewFaces.empty());
                        newFaces.insert(newFaces.end(), actualNewFaces.begin(), actualNewFaces.end());
                    }
                    droppedFaces.insert(droppedFaces.end(), m_droppedFaces.begin(), m_droppedFaces.end());
                    
                    m_newFaces.clear();
                    m_droppedFaces.clear();
                }
            };
        protected:
            FaceManager m_faceManager;

            struct MoveVertexResult {
                typedef enum {
                    VertexMoved,
                    VertexDeleted,
                    VertexUnchanged
                } Type;
                
                const Type type;
                BrushVertex* vertex;
                
                MoveVertexResult(const Type i_type, BrushVertex* i_vertex = NULL) :
                type(i_type),
                vertex(i_vertex) {
                    assert(type != VertexDeleted || vertex == NULL);
                }
            };
        protected:
            MoveBrushVertexAlgorithm(BrushGeometry& geometry) :
            BrushAlgorithm<R>(geometry) {}
            
            MoveVertexResult moveVertex(BrushGeometry& geometry, BrushVertex* vertex, const bool allowMerge, const Vec3& start, const Vec3& end) {
                assert(vertex != NULL);
                assert(start != end);
                assert(geometry.sanityCheck());
                
                FloatType lastFrac = 0.0;
                while (!vertex->position.equals(end, 0.0)) {
                    const Vec3 lastPosition = vertex->position;
                    BrushFaceGeometryList affectedSides = geometry.incidentSides(vertex);
                    
                    // Chop all sides incident to the moving vertex into triangles.
                    for (size_t i = 0; i < affectedSides.size(); ++i) {
                        BrushFaceGeometry* side = affectedSides[i];
                        if (side->vertices.size() > 3) {
                            const Plane3& boundary = side->face->boundary();
                            const FloatType dot = end.dot(boundary.normal) - boundary.distance;
                            
                            if (Math::neg(dot)) {
                                // Vertex will move the boundary: Chop off one triangle.
                                const size_t vertexIndex = VectorUtils::indexOf(side->vertices, vertex);
                                chopFace(geometry, side, vertexIndex);
                            } else {
                                // Vertex will move above or parallel to the boundary: Create a triangle fan.
                                for (size_t i = 1; i < side->vertices.size() - 1; ++i) {
                                    const size_t vertexIndex = VectorUtils::indexOf(side->vertices, vertex);
                                    chopFace(geometry, side, Math::succ(vertexIndex, side->vertices.size()));
                                }
                            }
                        }
                    }
                    affectedSides = geometry.incidentSides(vertex);
                    
                    // Now all sides incident to the vertex are triangles. We need to compute the next point to which the
                    // vertex can be moved without making the brush convex. For that, we consider each incident side
                    // and two of its neighbours: Its successor in the list of incident sides and its one neighbour that is
                    // not incident to the vertex.
                    
                    FloatType minFrac = 1.0;
                    for (size_t i = 0; i < affectedSides.size(); ++i) {
                        Plane3 plane;
                        FloatType startDot, endDot, frac;
                        
                        BrushFaceGeometry* side = affectedSides[i];
                        BrushFaceGeometry* next = affectedSides[Math::succ(i, affectedSides.size())];
                        
                        /*
                         First, we consider the plane made up by the points p1, p2 and p3 of side and next. If the movement
                         of the vertex were to go through this plane, the brush would become convex, which we must prevent.
                         
                         v----p1
                         |\ s |
                         | \  |
                         |  \ |
                         | n \|
                         p3----p2
                         
                         */
                        
                        const size_t sideIndex0 = VectorUtils::indexOf(side->vertices, vertex);
                        const size_t nextIndex0 = VectorUtils::indexOf(next->vertices, vertex);
                        assert(sideIndex0 < side->vertices.size());
                        assert(nextIndex0 < next->vertices.size());
                        
                        const size_t sideIndex1 = Math::succ(sideIndex0, side->vertices.size()); // index of next
                        const size_t sideIndex2 = Math::succ(sideIndex0, side->vertices.size(), 2); // index of next but one
                        const size_t nextIndex1 = Math::succ(nextIndex0, next->vertices.size(), 2); // index of next but one
                        
                        const Vec3& p1 = side->vertices[sideIndex1]->position;
                        const Vec3& p2 = side->vertices[sideIndex2]->position;
                        const Vec3& p3 = next->vertices[nextIndex1]->position;
                        if (!setPlanePoints(plane, p1, p2, p3)) {
                            // The points are colinear and we cannot determine the move distance - this is an error, but we
                            // gracefully stop the operation and return.
                            return cancel(geometry, vertex);
                        }
                        
                        startDot = start.dot(plane.normal) - plane.distance;
                        endDot = end.dot(plane.normal) - plane.distance;
                        
                        if (std::abs(startDot) >= 0.001 || std::abs(endDot) >= 0.001) {
                            if ((startDot > 0.0) != (endDot >  0.0)) {
                                frac = std::abs(startDot) < 0.001 ? 1.0 : std::abs(startDot) / (std::abs(startDot) + std::abs(endDot));
                                if (frac > lastFrac && frac < minFrac)
                                    minFrac = frac;
                            }
                        }
                        
                        /*
                         Second, we consider the boundary plane of the one neighbour to side which is not incident to the
                         moved vertex. This neighbour is not necessarily a triangle, but that does not matter.
                         
                                  -------
                                 /   n  |
                                /    e  |
                         v-----/     i  |
                         |\ s |      g  |
                         | \  |      h  |
                         |  \ |      b  |
                         |   \|      o  |
                         -----\      u  |
                               \     r  |
                                ---------
                         */
                        
                        const BrushEdge* neighbourEdge = side->edges[sideIndex1];
                        const BrushFaceGeometry* neighbourSide = neighbourEdge->left == side ? neighbourEdge->right : neighbourEdge->left;
                        const Vec3& b1 = neighbourSide->vertices[0]->position;
                        const Vec3& b2 = neighbourSide->vertices[1]->position;
                        const Vec3& b3 = neighbourSide->vertices[2]->position;
                        if (!setPlanePoints(plane, b1, b2, b3)) { // Don't use the side face's boundary plane here as it might not yet be updated!
                            // The points are colinear and we cannot determine the move distance - this is an error, but we
                            // gracefully stop the operation and return.
                            return cancel(geometry, vertex);
                        }
                        
                        startDot = start.dot(plane.normal) - plane.distance;
                        endDot = end.dot(plane.normal) - plane.distance;
                        
                        if (std::abs(startDot) >= 0.001 || std::abs(endDot) >= 0.001) {
                            if ((startDot > 0.0) != (endDot >  0.0)) {
                                frac = std::abs(startDot) < 0.001 ? 1.0 : std::abs(startDot) / (std::abs(startDot) + std::abs(endDot));
                                if (frac > lastFrac && frac < minFrac)
                                    minFrac = frac;
                            }
                        }
                    }
                    
                    assert(minFrac > lastFrac);
                    lastFrac = minFrac;
                    
                    // We can now safely move the vertex to this point without the brush becoming convex:
                    vertex->position = start + lastFrac * (end - start);
                    
                    // Now we check whether the vertex landed on another vertex. If so, we cancel the operation unless
                    // that vertex is adjacent to the moved vertex and allowMerge is true.
                    for (size_t i = 0; i < geometry.vertices.size(); ++i) {
                        BrushVertex* candidate = geometry.vertices[i];
                        if (vertex != candidate && vertex->position.equals(candidate->position)) {
                            BrushEdge* connectingEdge = NULL;
                            for (size_t j = 0; j < geometry.edges.size(); ++j) {
                                BrushEdge* edge = geometry.edges[j];
                                if (edge->connects(vertex, candidate))
                                    connectingEdge = edge;
                            }
                            
                            if (connectingEdge != NULL && allowMerge) {
                                // The vertex was dragged onto an adjacent vertex and we are allowed to merge them.
                                for (size_t j = 0; j < geometry.edges.size(); ++j) {
                                    BrushEdge* edge = geometry.edges[j];
                                    if (edge != connectingEdge && (edge->start == candidate || edge->end == candidate)) {
                                        if (edge->start == candidate)
                                            edge->start = vertex;
                                        else
                                            edge->end = vertex;
                                        
                                        std::replace(edge->left->vertices.begin(), edge->left->vertices.end(), candidate, vertex);
                                        std::replace(edge->right->vertices.begin(), edge->right->vertices.end(), candidate, vertex);
                                    }
                                }
                                
                                deleteDegenerateTriangle(geometry, connectingEdge->left, connectingEdge);
                                deleteDegenerateTriangle(geometry, connectingEdge->right, connectingEdge);
                                VectorUtils::eraseAndDelete(geometry.edges, connectingEdge);
                                VectorUtils::eraseAndDelete(geometry.vertices, candidate);
                            } else {
                                // The vertex was either dragged onto a non-adjacent vertex or we weren't allowed to
                                // merge it with an adjacent vertex, so undo the operation and return.
                                vertex->position = lastPosition;
                                return cancel(geometry, vertex);
                            }
                        }
                    }
                    
                    // If any of the incident sides has become colinear, we abort the operation.
                    affectedSides = geometry.incidentSides(vertex);
                    for (size_t i = 0; i < affectedSides.size(); ++i) {
                        BrushFaceGeometry* side = affectedSides[i];
                        if (side->isColinearTriangle() < side->edges.size()) {
                            vertex->position = lastPosition;
                            return cancel(geometry, vertex);
                        }
                    }
                    
                    // affectedSides = incidentSides(vertex);
                    // deleteCollinearTriangles(affectedSides, newFaces, droppedFaces);
                    
                    assert(geometry.sanityCheck());
                    
                    cleanup(geometry);
                    geometry.updateBounds();
                    
                    assert(geometry.sanityCheck());
                    
                    if (!VectorUtils::contains(geometry.vertices, vertex))
                        return MoveVertexResult(MoveVertexResult::VertexDeleted);
                }
                
                return MoveVertexResult(MoveVertexResult::VertexMoved, vertex);
            }
            
            void updateFacePoints(BrushGeometry& geometry) {
                // This method must ONLY be called at the end of a vertex operation, just before
                // the geometry is rebuilt anyway
                
                // TODO: Originally, this method attempted to find optimal integer plane points. For now, I have have
                // opted to only update the face points according to the vertex positions, leaving the rest to the user
                // because the issue browser should indicate if the plane points have become fractional.
                for (size_t i = 0; i < geometry.sides.size(); ++i) {
                    BrushFaceGeometry* side = geometry.sides[i];
                    BrushFace* face = side->face;
                    assert(face != NULL);
                    
                    try {
                        face->updatePointsFromVertices();
                    } catch (GeometryException&) {
                        // the side has gone out of whack...
                        m_faceManager.dropFace(side);
                    }
                }
            }
            
            void updateNewAndDroppedFaces() {
                m_faceManager.getFaces(BrushAlgorithm<R>::m_addedFaces,
                                       BrushAlgorithm<R>::m_removedFaces);
            }
        private:
            MoveVertexResult cancel(BrushGeometry& geometry, BrushVertex* vertex) {
                cleanup(geometry);
                return MoveVertexResult(MoveVertexResult::VertexUnchanged, vertex);
            }
            
            void cleanup(BrushGeometry& geometry) {
                mergeSides(geometry);
                mergeEdges(geometry);
            }
            
            void chopFace(BrushGeometry& geometry, BrushFaceGeometry* side, const size_t vertexIndex) {
                BrushFaceGeometry* newSide = NULL;
                BrushEdge* newEdge = NULL;
                side->chop(vertexIndex, newSide, newEdge);
                assert(newSide != NULL);
                assert(newEdge != NULL);
                
                geometry.edges.push_back(newEdge);
                geometry.sides.push_back(newSide);
                m_faceManager.addFace(side->face, newSide->face);
            }
            
            void mergeSides(BrushGeometry& geometry) {
                for (size_t i = 0; i < geometry.sides.size(); ++i) {
                    BrushFaceGeometry* side = geometry.sides[i];
                    
                    Plane3 sideBoundary;
                    setPlanePoints(sideBoundary,
                                   side->vertices[0]->position,
                                   side->vertices[1]->position,
                                   side->vertices[2]->position);
                    
                    for (size_t j = 0; j < side->edges.size(); ++j) {
                        BrushEdge* edge = side->edges[j];
                        BrushFaceGeometry* neighbour = edge->left != side ? edge->left : edge->right;
                        
                        Plane3 neighbourBoundary;
                        setPlanePoints(neighbourBoundary,
                                       neighbour->vertices[0]->position,
                                       neighbour->vertices[1]->position,
                                       neighbour->vertices[2]->position);
                        
                        if (sideBoundary.equals(neighbourBoundary, Math::C::ColinearEpsilon)) {
                            mergeNeighbours(geometry, side, j);
                            --i;
                            break;
                        }
                    }
                }
            }
            
            void mergeNeighbours(BrushGeometry& geometry, BrushFaceGeometry* side, const size_t edgeIndex) {
                BrushVertex* vertex = NULL;
                BrushEdge* edge = side->edges[edgeIndex];
                BrushFaceGeometry* neighbour = edge->left != side ? edge->left : edge->right;
                
                size_t sideEdgeIndex = edgeIndex;
                size_t neighbourEdgeIndex = VectorUtils::indexOf(neighbour->edges, edge);
                assert(neighbourEdgeIndex < neighbour->edges.size());
                
                do {
                    sideEdgeIndex = Math::succ(sideEdgeIndex, side->edges.size());
                    neighbourEdgeIndex = Math::pred(neighbourEdgeIndex, neighbour->edges.size());
                } while (side->edges[sideEdgeIndex] == neighbour->edges[neighbourEdgeIndex]);
                
                // now sideEdgeIndex points to the last edge (in CW order) of side that should not be deleted
                // and neighbourEdgeIndex points to the first edge (in CW order) of neighbour that should not be deleted
                
                int count = -1;
                do {
                    sideEdgeIndex = Math::pred(sideEdgeIndex, side->edges.size());
                    neighbourEdgeIndex = Math::succ(neighbourEdgeIndex, neighbour->edges.size());
                    ++count;
                } while (side->edges[sideEdgeIndex] == neighbour->edges[neighbourEdgeIndex]);
                
                // now sideEdgeIndex points to the first edge (in CW order) of side that should not be deleted
                // now neighbourEdgeIndex points to the last edge (in CW order) of neighbour that should not be deleted
                // and count is the number of shared edges between side and neighbour
                
                assert(count >= 0);
                const size_t totalVertexCount = side->edges.size() + neighbour->edges.size() - static_cast<size_t>(2 * count);
                _unused(totalVertexCount);
                
                // shift the two sides so that their shared edges are at the end of both's edge lists
                side->shift(Math::succ(sideEdgeIndex, side->edges.size(), static_cast<size_t>(count + 1)));
                neighbour->shift(neighbourEdgeIndex);
                
                side->edges.resize(side->edges.size() - static_cast<size_t>(count));
                side->vertices.resize(side->vertices.size() - static_cast<size_t>(count));
                
                for (size_t i = 0; i < neighbour->edges.size() - static_cast<size_t>(count); ++i) {
                    edge = neighbour->edges[i];
                    vertex = neighbour->vertices[i];
                    if (edge->left == neighbour)
                        edge->left = side;
                    else
                        edge->right = side;
                    side->edges.push_back(edge);
                    side->vertices.push_back(vertex);
                }
                
                for (size_t i = neighbour->edges.size() - static_cast<size_t>(count); i < neighbour->edges.size(); ++i) {
                    bool success = VectorUtils::eraseAndDelete(geometry.edges, neighbour->edges[i]);
                    _unused(success);
                    assert(success);
                    if (i > neighbour->edges.size() - static_cast<size_t>(count)) {
                        success = VectorUtils::eraseAndDelete(geometry.vertices, neighbour->vertices[i]);
                        assert(success);
                    }
                }
                
                for (size_t i = 0; i < side->edges.size(); ++i) {
                    edge = side->edges[i];
                    if (edge->left == side)
                        assert(edge->right != neighbour);
                    else
                        assert(edge->left != neighbour);
                }
                
                m_faceManager.dropFace(neighbour);
                bool success = VectorUtils::eraseAndDelete(geometry.sides, neighbour);
                _unused(success);
                assert(success);
                
                assert(side->vertices.size() == totalVertexCount);
                assert(side->edges.size() == totalVertexCount);
            }
            
            void mergeEdges(BrushGeometry& geometry) {
                for (size_t i = 0; i < geometry.edges.size(); i++) {
                    BrushEdge* edge = geometry.edges[i];
                    const Vec3 edgeVector = edge->vector();
                    for (size_t j = i + 1; j < geometry.edges.size(); j++) {
                        BrushEdge* candidate = geometry.edges[j];
                        if (edge->isIncidentWith(candidate)) {
                            const Vec3 candidateVector = candidate->vector();
                            if (edgeVector.parallelTo(candidateVector, Math::C::ColinearEpsilon)) {
                                if (edge->end == candidate->end)
                                    candidate->flip();
                                if (edge->end == candidate->start &&
                                    edge->start != candidate->end &&
                                    edge->left == candidate->left &&
                                    edge->right == candidate->right) {
                                    
                                    assert(edge->start != candidate->end);
                                    assert(edge->left->vertices.size() > 3);
                                    assert(edge->right->vertices.size() > 3);
                                    
                                    BrushFaceGeometry* leftSide = edge->left;
                                    BrushFaceGeometry* rightSide = edge->right;
                                    
                                    assert(leftSide != rightSide);
                                    
                                    BrushEdge* newEdge = new BrushEdge(edge->start, candidate->end, leftSide, rightSide);
                                    geometry.edges.push_back(newEdge);
                                    
                                    const size_t leftIndex = VectorUtils::indexOf(leftSide->edges, candidate);
                                    const size_t rightIndex = VectorUtils::indexOf(rightSide->edges, candidate);
                                    const size_t leftCount = leftSide->edges.size();
                                    const size_t rightCount = rightSide->edges.size();
                                    
                                    leftSide->replaceEdgesWithEdge(leftIndex,
                                                                   Math::succ(leftIndex, leftCount, 2),
                                                                   newEdge);
                                    rightSide->replaceEdgesWithEdge(Math::pred(rightIndex, rightCount),
                                                                    Math::succ(rightIndex, rightCount),
                                                                    newEdge);
                                    
                                    VectorUtils::eraseAndDelete(geometry.vertices, candidate->start);
                                    VectorUtils::eraseAndDelete(geometry.edges, candidate);
                                    VectorUtils::eraseAndDelete(geometry.edges, edge);
                                    
                                    break;
                                }
                                
                                if (edge->start == candidate->start)
                                    candidate->flip();
                                if (edge->start == candidate->end &&
                                    edge->end != candidate->start &&
                                    edge->left == candidate->left &&
                                    edge->right == candidate->right) {
                                    
                                    assert(edge->end != candidate->start);
                                    assert(edge->left->vertices.size() > 3);
                                    assert(edge->right->vertices.size() > 3);
                                    
                                    BrushFaceGeometry* leftSide = edge->left;
                                    BrushFaceGeometry* rightSide = edge->right;
                                    
                                    assert(leftSide != rightSide);
                                    
                                    BrushEdge* newEdge = new BrushEdge(candidate->start, edge->end);
                                    newEdge->left = leftSide;
                                    newEdge->right = rightSide;
                                    geometry.edges.push_back(newEdge);
                                    
                                    const size_t leftIndex = VectorUtils::indexOf(leftSide->edges, candidate);
                                    const size_t rightIndex = VectorUtils::indexOf(rightSide->edges, candidate);
                                    const size_t leftCount = leftSide->edges.size();
                                    const size_t rightCount = rightSide->edges.size();
                                    
                                    leftSide->replaceEdgesWithEdge(Math::pred(leftIndex, leftCount),
                                                                   Math::succ(leftIndex, leftCount),
                                                                   newEdge);
                                    rightSide->replaceEdgesWithEdge(rightIndex,
                                                                    Math::succ(rightIndex, rightCount, 2),
                                                                    newEdge);
                                    
                                    VectorUtils::eraseAndDelete(geometry.vertices, candidate->end);
                                    VectorUtils::eraseAndDelete(geometry.edges, candidate);
                                    VectorUtils::eraseAndDelete(geometry.edges, edge);
                                    
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            void deleteDegenerateTriangle(BrushGeometry& geometry, BrushFaceGeometry* side, BrushEdge* edge) {
                assert(side != NULL);
                assert(edge != NULL);
                assert(side->edges.size() == 3);
                
                side->shift(VectorUtils::indexOf(side->edges, edge));
                
                BrushEdge* keepEdge = side->edges[1];
                BrushEdge* dropEdge = side->edges[2];
                BrushFaceGeometry* neighbour = dropEdge->left == side ? dropEdge->right : dropEdge->left;
                
                if (keepEdge->left == side)
                    keepEdge->left = neighbour;
                else
                    keepEdge->right = neighbour;
                
                const size_t deleteIndex = VectorUtils::indexOf(neighbour->edges, dropEdge);
                const size_t nextIndex = Math::succ(deleteIndex, neighbour->edges.size());
                neighbour->replaceEdgesWithEdge(deleteIndex, nextIndex, keepEdge);
                
                m_faceManager.dropFace(side);
                VectorUtils::eraseAndDelete(geometry.sides, side);
                VectorUtils::eraseAndDelete(geometry.edges, dropEdge);
            }
        };
    }
}

#endif /* defined(__TrenchBroom__MoveBrushVertexAlgorithm__) */
