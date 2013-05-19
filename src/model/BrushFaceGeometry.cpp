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
        void BrushFaceGeometry::addForwardEdge(BrushEdge* edge) {
            if (edge == NULL) {
                GeometryException e;
                e << "Edge must not be NULL";
                throw e;
            }
            
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
        
        void BrushFaceGeometry::addForwardEdges(const BrushEdgeList& edges) {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                addForwardEdge(edge);
            }
        }

        void BrushFaceGeometry::addBackwardEdge(BrushEdge* edge) {
            if (edge == NULL) {
                GeometryException e;
                e << "Edge must not be NULL";
                throw e;
            }
            
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

        void BrushFaceGeometry::addBackwardEdges(const BrushEdgeList& edges) {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge* edge = *it;
                addBackwardEdge(edge);
            }
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
    }
}
