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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushVertex.h"

#include "CollectionUtils.h"
#include "MathUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        BrushVertex::BrushVertex(const Vec3& position) :
        m_position(position),
        m_mark(New) {}

        const Vec3& BrushVertex::position() const {
            return m_position;
        }
        
        BrushVertex::Mark BrushVertex::mark() const {
            return m_mark;
        }

        void BrushVertex::updateMark(const Plane3& plane) {
            const Math::PointStatus::Type status = plane.pointStatus(m_position);
            switch (status) {
                case Math::PointStatus::PSAbove:
                    m_mark = Drop;
                    break;
                case Math::PointStatus::PSBelow:
                    m_mark = Keep;
                    break;
                default:
                    m_mark = Undecided;
                    break;
            }
        }

        BrushFaceGeometryList BrushVertex::incidentSides(const BrushEdgeList& edges) const {
            BrushFaceGeometryList result;
            
            BrushEdge* edge = NULL;
            BrushEdgeList::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd && edge == NULL; ++eIt) {
                BrushEdge* candidate = *eIt;
                if (candidate->start() == this || candidate->end() == this)
                    edge = candidate;
            }
            
            assert(edge != NULL);
            
            // iterate over the incident sides in clockwise order
            BrushFaceGeometry* side = edge->start() == this ? edge->right() : edge->left();
            do {
                result.push_back(side);
                const size_t index = VectorUtils::indexOf(side->edges(), edge);
                
                const BrushEdgeList& sideEdges = side->edges();
                edge = sideEdges[Math::pred(index, sideEdges.size())];
                side = edge->start() == this ? edge->right() : edge->left();
            } while (side != result.front());
            
            return result;
        }

        BrushVertexList::iterator findBrushVertex(BrushVertexList& vertices, const Vec3& position) {
            BrushVertexList::iterator it = vertices.begin();
            const BrushVertexList::iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position() == position)
                    return it;
                ++it;
            }
            return end;
        }
        
        BrushVertexList::const_iterator findBrushVertex(const BrushVertexList& vertices, const Vec3& position) {
            BrushVertexList::const_iterator it = vertices.begin();
            const BrushVertexList::const_iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position() == position)
                    return it;
                ++it;
            }
            return end;
        }
    }
}
