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

#include "BrushFaceGeometry.h"

#include "Exceptions.h"
#include "Macros.h"
#include "MathUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        BrushFaceGeometry::BrushFaceGeometry() :
        face(NULL) {
            vertices.reserve(8);
            edges.reserve(8);
        }

        BrushFaceGeometry::~BrushFaceGeometry() {
            face = NULL;
        }

        BrushFaceGeometry::Mark BrushFaceGeometry::mark() const {
            size_t drop = 0;
            size_t keep = 0;
            size_t split = 0;
            size_t undecided = 0;
            
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const BrushEdge& edge = **it;
                const BrushEdge::Mark edgeMark = edge.mark;
                if (edgeMark == BrushEdge::Drop)
                    drop++;
                else if (edgeMark == BrushEdge::Keep)
                    keep++;
                else if (edgeMark == BrushEdge::Split)
                    split++;
                else if (edgeMark == BrushEdge::Undecided)
                    undecided++;
            }
            
            assert(drop + keep + split + undecided == edges.size());
            assert(undecided < edges.size());
            if (keep + undecided == edges.size())
                return Mark_Keep;
            if (drop + undecided == edges.size())
                return Mark_Drop;
            return Mark_Split;
        }

        BrushEdge* BrushFaceGeometry::splitUsingEdgeMarks() {
            assert(mark() == Mark_Split);
            assert(!edges.empty());

            BrushEdgeList::iterator splitIt1 = edges.end();
            BrushEdgeList::iterator splitIt2 = edges.end();
            BrushVertex* newEdgeStart = NULL;
            BrushVertex* newEdgeEnd = NULL;
            
            BrushEdge* lastEdge = edges.back();
            BrushEdge::Mark lastMark = lastEdge->mark;
            BrushEdgeList::iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* currentEdge = *it;
                const BrushEdge::Mark currentMark = currentEdge->mark;
                if (currentMark == BrushEdge::Keep && lastMark == BrushEdge::Drop) {
                    splitIt2 = it;
                    newEdgeStart = currentEdge->startVertex(this);
                    assert(newEdgeStart != NULL);
                } else if (currentMark == BrushEdge::Split && lastMark == BrushEdge::Drop) {
                    splitIt2 = it;
                    newEdgeStart = currentEdge->startVertex(this);
                    assert(newEdgeStart != NULL);
                } else if (currentMark == BrushEdge::Drop && lastMark == BrushEdge::Keep) {
                    splitIt1 = it;
                    newEdgeEnd = currentEdge->startVertex(this);
                    assert(newEdgeEnd != NULL);
                } else if (currentMark == BrushEdge::Drop &&  lastMark == BrushEdge::Split) {
                    splitIt1 = it;
                    newEdgeEnd = lastEdge->endVertex(this);
                    assert(newEdgeEnd != NULL);
                } else if (currentMark == BrushEdge::Split && lastMark == BrushEdge::Split) {
                    if (currentEdge->startVertex(this)->mark == BrushVertex::Mark_New) {
                        splitIt1 = splitIt2 = it;
                        newEdgeStart = currentEdge->startVertex(this);
                        newEdgeEnd = lastEdge->endVertex(this);
                    } else {
                        assert(currentEdge->startVertex(this)->mark == BrushVertex::Mark_Keep);
                        splitIt1 = it < edges.end() - 1 ? it + 1 : edges.begin();
                        splitIt2 = it > edges.begin() ? it - 1 : edges.end() - 1;
                        newEdgeStart = lastEdge->startVertex(this);
                        newEdgeEnd = currentEdge->endVertex(this);
                    }
                    
                    assert(newEdgeStart != NULL);
                    assert(newEdgeEnd != NULL);
                    assert(newEdgeStart != newEdgeEnd);
                }
                
                if (splitIt1 != end && splitIt2 != end)
                    break;
                
                lastEdge = currentEdge;
                lastMark = currentMark;
            }

            if (splitIt1 == edges.end() || splitIt2 == edges.end()) {
                GeometryException e;
                e << "Invalid brush detected during side split";
                throw e;
            }
            
            assert(newEdgeStart != NULL);
            assert(newEdgeEnd != NULL);
            BrushEdge* newEdge = new BrushEdge(newEdgeStart, newEdgeEnd);
            replaceEdgesWithBackwardEdge(splitIt1, splitIt2, newEdge);
            return newEdge;
        }

        BrushEdge* BrushFaceGeometry::findUndecidedEdge() const {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                if (edge->mark == BrushEdge::Undecided)
                    return edge;
            }
            return NULL;
        }

        void BrushFaceGeometry::addForwardEdge(BrushEdge* edge) {
            assert(edge != NULL);
            
            if (!edges.empty()) {
                const BrushEdge* previous = edges.back();
                if (previous->endVertex(this) != edge->start)
                    throw GeometryException("Cannot add non-consecutive edge");
            }
            
            edge->right = this;
            edges.push_back(edge);
            vertices.push_back(edge->start);
        }
        
        void BrushFaceGeometry::addForwardEdges(const BrushEdgeList& edges) {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                addForwardEdge(edge);
            }
        }

        void BrushFaceGeometry::addBackwardEdge(BrushEdge* edge) {
            assert(edge != NULL);
            
            if (!edges.empty()) {
                const BrushEdge* previous = edges.back();
                if (previous->endVertex(this) != edge->end)
                    throw GeometryException("Cannot add non-consecutive edge");
            }
            
            edge->left = this;
            edges.push_back(edge);
            vertices.push_back(edge->end);
        }

        void BrushFaceGeometry::addBackwardEdges(const BrushEdgeList& edges) {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                addBackwardEdge(edge);
            }
        }

        void BrushFaceGeometry::addEdge(BrushEdge* edge, const bool forward) {
            if (forward)
                addForwardEdge(edge);
            else
                addBackwardEdge(edge);
        }

        bool BrushFaceGeometry::containsDroppedEdge() const {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                if (edge->mark == BrushEdge::Drop)
                    return true;
            }
            return false;
        }

        bool BrushFaceGeometry::isClosed() const {
            if (edges.size() < 3)
                return false;
            
            BrushEdgeList::const_iterator it, end;
            const BrushEdge* previous = edges.back();
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                if (previous->endVertex(this) != edge->startVertex(this))
                    return false;
                previous = edge;
            }
            
            return true;
        }

        bool BrushFaceGeometry::hasVertexPositions(const Vec3::List& positions) const {
            if (positions.size() != vertices.size())
                return false;
            
            const size_t size = vertices.size();
            for (size_t i = 0; i < size; i++) {
                bool equal = true;
                for (size_t j = 0; j < size && equal; j++) {
                    const size_t index = (j + i) % size;
                    if (vertices[j]->position != positions[index])
                        equal = false;
                }
                if (equal)
                    return true;
            }
            
            return false;
        }

        size_t BrushFaceGeometry::isColinearTriangle() const {
            assert(edges.size() >= 3);
            if (edges.size() > 3)
                return edges.size();
            
            Vec3 edgeVector1 = edges[0]->vector();
            Vec3 edgeVector2 = edges[1]->vector();
            
            if (edgeVector1.parallelTo(edgeVector2)) {
                const Vec3 edgeVector3 = edges[2]->vector();
                assert(edgeVector1.parallelTo(edgeVector3));
                assert(edgeVector2.parallelTo(edgeVector3));
                
                const FloatType length1 = edgeVector1.squaredLength();
                const FloatType length2 = edgeVector2.squaredLength();
                const FloatType length3 = edgeVector3.squaredLength();
                
                // we'll return the index of the longest of the three edges
                if (length1 > length2) {
                    if (length1 > length3)
                        return 0;
                    else
                        return 2;
                } else {
                    if (length2 > length3)
                        return 1;
                    else
                        return 2;
                }
            } else {
                const Vec3 edgeVector3 = edges[2]->vector();
                assert(!edgeVector1.parallelTo(edgeVector3));
                assert(!edgeVector2.parallelTo(edgeVector3));
                _unused(edgeVector3);
                
                return edges.size();
            }
        }

        void BrushFaceGeometry::chop(const size_t vertexIndex, BrushFaceGeometry*& newSide, BrushEdge*& newEdge) {
            const size_t vertexCount = vertices.size();
            const size_t edgeCount = edges.size();
            assert(vertexCount == edgeCount);
            assert(vertexCount > 3);
            assert(vertexIndex < vertexCount);
            
            BrushVertex* prevVertex = vertices[Math::pred(vertexIndex, vertexCount)];
            BrushVertex* nextVertex = vertices[Math::succ(vertexIndex, vertexCount)];
            
            BrushEdge* prevEdge = edges[Math::pred(vertexIndex, edgeCount)];
            BrushEdge* nextEdge = edges[vertexIndex];
            newEdge = new BrushEdge(nextVertex, prevVertex, this, NULL);

            newSide = new BrushFaceGeometry();
            newSide->addEdge(prevEdge, prevEdge->right == this);
            newSide->addEdge(nextEdge, nextEdge->right == this);
            newSide->addForwardEdge(newEdge);

            if (face != NULL) {
                BrushFace* newFace = face->clone();
                newSide->face = newFace;
                newFace->setSide(newSide);
            }
            
            BrushEdgeList::iterator start = edges.begin();
            BrushEdgeList::iterator end = edges.begin();
            std::advance(start, Math::pred(vertexIndex, edgeCount));
            std::advance(end, Math::succ(vertexIndex, edgeCount));
            replaceEdgesWithBackwardEdge(start, end, newEdge);
        }

        void BrushFaceGeometry::shift(const size_t offset) {
            const size_t count = edges.size();
            if (offset % count == 0)
                return;
            
            BrushEdgeList newEdges(edges.size());;
            BrushVertexList newVertices(vertices.size());;
            
            for (size_t i = 0; i < count; ++i) {
                const size_t index = Math::succ(i, count, offset);
                newEdges[i] = edges[index];
                newVertices[i] = vertices[index];
            }
            
            using std::swap;
            swap(edges, newEdges);
            swap(vertices, newVertices);
        }

        void BrushFaceGeometry::replaceEdgesWithEdge(const size_t index1, const size_t index2, BrushEdge* edge) {
            assert(index1 < edges.size());
            assert(index2 < edges.size());
            assert(edge != NULL);
            
            BrushEdgeList::iterator it1 = edges.begin();
            BrushEdgeList::iterator it2 = edges.begin();
            std::advance(it1, index1);
            std::advance(it2, index2);
            replaceEdgesWithEdge(it1, it2, edge);
        }

        Polygon3 BrushFaceGeometry::faceInfo() const {
            Vec3::List positions(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i)
                positions[i] = vertices[i]->position;
            return Polygon3(positions);
        }

        void BrushFaceGeometry::replaceEdgesWithBackwardEdge(const BrushEdgeList::iterator it1, const BrushEdgeList::iterator it2, BrushEdge* edge) {
            edge->left = this;
            replaceEdgesWithEdge(it1, it2, edge);
        }

        void BrushFaceGeometry::replaceEdgesWithEdge(const BrushEdgeList::iterator it1, const BrushEdgeList::iterator it2, BrushEdge* edge) {
            using std::swap;
            if (it1 == it2) {
                edges.insert(it1, edge);
            } else if (it1 < it2) {
                BrushEdgeList newEdges;
                newEdges.insert(newEdges.end(), edges.begin(), it1);
                newEdges.push_back(edge);
                newEdges.insert(newEdges.end(), it2, edges.end());
                swap(edges, newEdges);
            } else {
                BrushEdgeList newEdges;
                newEdges.insert(newEdges.end(), it2, it1);
                newEdges.push_back(edge);
                swap(edges, newEdges);
            }
            assert(edges.size() >= 3);
            updateVerticesFromEdges();
        }

        void BrushFaceGeometry::updateVerticesFromEdges() {
            vertices.clear();
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge& edge = **it;
                BrushVertex* vertex = edge.startVertex(this);
                assert(vertex != NULL);
                vertices.push_back(vertex);
            }
        }

        BrushFaceGeometryList::iterator findBrushFaceGeometry(BrushFaceGeometryList& faceGeometries, const Vec3::List& positions) {
            BrushFaceGeometryList::iterator it = faceGeometries.begin();
            const BrushFaceGeometryList::iterator end = faceGeometries.end();
            while (it != end) {
                const BrushFaceGeometry& faceGeometry = **it;
                if (faceGeometry.hasVertexPositions(positions))
                    return it;
                ++it;
            }
            return end;
        }
        
        BrushFaceGeometryList::const_iterator findBrushFaceGeometry(const BrushFaceGeometryList& faceGeometries, const Vec3::List& positions) {
            BrushFaceGeometryList::const_iterator it = faceGeometries.begin();
            const BrushFaceGeometryList::const_iterator end = faceGeometries.end();
            while (it != end) {
                const BrushFaceGeometry& faceGeometry = **it;
                if (faceGeometry.hasVertexPositions(positions))
                    return it;
                ++it;
            }
            return end;
        }
    }
}
