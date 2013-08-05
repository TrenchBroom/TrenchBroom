/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushFaceGeometry.h"

#include "Exceptions.h"
#include "MathUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        const BrushFaceGeometry::List BrushFaceGeometry::EmptyList = BrushFaceGeometry::List();

        BrushFaceGeometry::BrushFaceGeometry() :
        m_face(NULL) {
            m_vertices.reserve(8);
            m_edges.reserve(8);
        }

        BrushFaceGeometry::~BrushFaceGeometry() {
            m_face = NULL;
        }

        BrushFace* BrushFaceGeometry::face() const {
            return m_face;
        }
        
        void BrushFaceGeometry::setFace(BrushFace* face) {
            m_face = face;
        }
        
        const BrushVertex::List& BrushFaceGeometry::vertices() const {
            return m_vertices;
        }
        
        const BrushEdge::List& BrushFaceGeometry::edges() const {
            return m_edges;
        }

        BrushFaceGeometry::Mark BrushFaceGeometry::mark() const {
            size_t drop = 0;
            size_t keep = 0;
            size_t split = 0;
            size_t undecided = 0;
            
            BrushEdge::List::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                const BrushEdge& edge = **it;
                const BrushEdge::Mark edgeMark = edge.mark();
                if (edgeMark == BrushEdge::Drop)
                    drop++;
                else if (edgeMark == BrushEdge::Keep)
                    keep++;
                else if (edgeMark == BrushEdge::Split)
                    split++;
                else if (edgeMark == BrushEdge::Undecided)
                    undecided++;
            }
            
            assert(drop + keep + split + undecided == m_edges.size());
            assert(undecided < m_edges.size());
            if (keep + undecided == m_edges.size())
                return Keep;
            if (drop + undecided == m_edges.size())
                return Drop;
            return Split;
        }

        BrushEdge* BrushFaceGeometry::splitUsingEdgeMarks() {
            assert(mark() == Split);
            assert(!m_edges.empty());

            BrushEdge::List::iterator splitIt1 = m_edges.end();
            BrushEdge::List::iterator splitIt2 = m_edges.end();
            BrushVertex* newEdgeStart = NULL;
            BrushVertex* newEdgeEnd = NULL;
            
            BrushEdge* lastEdge = m_edges.back();
            BrushEdge::Mark lastMark = lastEdge->mark();
            BrushEdge::List::iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                BrushEdge* currentEdge = *it;
                const BrushEdge::Mark currentMark = currentEdge->mark();
                if (currentMark == BrushEdge::Keep && lastMark == BrushEdge::Drop) {
                    splitIt2 = it;
                    newEdgeStart = currentEdge->start(this);
                    assert(newEdgeStart != NULL);
                } else if (currentMark == BrushEdge::Split && lastMark == BrushEdge::Drop) {
                    splitIt2 = it;
                    newEdgeStart = currentEdge->start(this);
                    assert(newEdgeStart != NULL);
                } else if (currentMark == BrushEdge::Drop && lastMark == BrushEdge::Keep) {
                    splitIt1 = it;
                    newEdgeEnd = currentEdge->start(this);
                    assert(newEdgeEnd != NULL);
                } else if (currentMark == BrushEdge::Drop &&  lastMark == BrushEdge::Split) {
                    splitIt1 = it;
                    newEdgeEnd = lastEdge->end(this);
                    assert(newEdgeEnd != NULL);
                } else if (currentMark == BrushEdge::Split && lastMark == BrushEdge::Split) {
                    if (currentEdge->start(this)->mark() == BrushVertex::New) {
                        splitIt1 = splitIt2 = it;
                        newEdgeStart = currentEdge->start(this);
                        newEdgeEnd = lastEdge->end(this);
                    } else {
                        assert(currentEdge->start(this)->mark() == BrushVertex::Keep);
                        splitIt1 = it < m_edges.end() - 1 ? it + 1 : m_edges.begin();
                        splitIt2 = it > m_edges.begin() ? it - 1 : m_edges.end() - 1;
                        newEdgeStart = lastEdge->start(this);
                        newEdgeEnd = currentEdge->end(this);
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

            if (splitIt1 == m_edges.end() || splitIt2 == m_edges.end()) {
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
            BrushEdge::List::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                if (edge->mark() == BrushEdge::Undecided)
                    return edge;
            }
            return NULL;
        }

        void BrushFaceGeometry::addForwardEdge(BrushEdge* edge) {
            assert(edge != NULL);
            
            if (edge->m_right != NULL) {
                GeometryException e;
                e << "Edge already has an incident side on its right";
                throw e;
            }
            
            if (!m_edges.empty()) {
                const BrushEdge* previous = m_edges.back();
                if (previous->end(this) != edge->start()) {
                    GeometryException e;
                    e << "Cannot add non-consecutive edge";
                    throw e;
                }
            }
            
            edge->m_right = this;
            m_edges.push_back(edge);
            m_vertices.push_back(edge->start());
        }
        
        void BrushFaceGeometry::addForwardEdges(const BrushEdge::List& edges) {
            BrushEdge::List::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                addForwardEdge(edge);
            }
        }

        void BrushFaceGeometry::addBackwardEdge(BrushEdge* edge) {
            assert(edge != NULL);
            
            if (edge->m_left != NULL) {
                GeometryException e;
                e << "Edge already has an incident side on its left";
                throw e;
            }

            if (!m_edges.empty()) {
                const BrushEdge* previous = m_edges.back();
                if (previous->end(this) != edge->end()) {
                    GeometryException e;
                    e << "Cannot add non-consecutive edge";
                    throw e;
                }
            }
            
            edge->m_left = this;
            m_edges.push_back(edge);
            m_vertices.push_back(edge->end());
        }

        void BrushFaceGeometry::addBackwardEdges(const BrushEdge::List& edges) {
            BrushEdge::List::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                addBackwardEdge(edge);
            }
        }

        bool BrushFaceGeometry::containsDroppedEdge() const {
            BrushEdge::List::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                if (edge->mark() == BrushEdge::Drop)
                    return true;
            }
            return false;
        }

        bool BrushFaceGeometry::isClosed() const {
            if (m_edges.size() < 3)
                return false;
            
            BrushEdge::List::const_iterator it, end;
            const BrushEdge* previous = m_edges.back();
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                const BrushEdge* edge = *it;
                if (previous->end(this) != edge->start(this))
                    return false;
                previous = edge;
            }
            
            return true;
        }

        bool BrushFaceGeometry::hasVertexPositions(const Vec3::List& positions) const {
            if (positions.size() != m_vertices.size())
                return false;
            
            const size_t size = m_vertices.size();
            for (size_t i = 0; i < size; i++) {
                bool equal = true;
                for (size_t j = 0; j < size && equal; j++) {
                    const size_t index = (j + i) % size;
                    if (m_vertices[j]->position() != positions[index])
                        equal = false;
                }
                if (equal)
                    return true;
            }
            
            return false;
        }

        void BrushFaceGeometry::replaceEdgesWithBackwardEdge(const BrushEdge::List::iterator it1, const BrushEdge::List::iterator it2, BrushEdge* edge) {
            if (it1 == it2) {
                m_edges.insert(it1, edge);
            } else if (it1 < it2) {
                BrushEdge::List newEdges;
                newEdges.insert(newEdges.end(), m_edges.begin(), it1);
                newEdges.push_back(edge);
                newEdges.insert(newEdges.end(), it2, m_edges.end());
                m_edges = newEdges;
            } else {
                BrushEdge::List newEdges;
                newEdges.insert(newEdges.end(), it2, it1);
                newEdges.push_back(edge);
                m_edges = newEdges;
            }
            assert(m_edges.size() >= 3);
            edge->m_left = this;
            updateVerticesFromEdges();
        }

        void BrushFaceGeometry::updateVerticesFromEdges() {
            m_vertices.clear();
            BrushEdge::List::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
                BrushEdge& edge = **it;
                BrushVertex* vertex = edge.start(this);
                assert(vertex != NULL);
                m_vertices.push_back(vertex);
            }
        }
    }
}
