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

#include "MathUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        BrushFaceGeometry::BrushFaceGeometry(const BrushEdgeList& edges) :
        m_edges(edges) {
            BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                BrushEdge& edge = **it;
                m_vertices.push_back(edge.start(this));
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
