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

#include "BrushVertex.h"

#include "CollectionUtils.h"
#include "MathUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        BrushVertex::BrushVertex(const Vec3& i_position) :
        position(i_position),
        mark(Mark_New) {}

        void BrushVertex::updateMark(const Plane3& plane) {
            const Math::PointStatus::Type status = plane.pointStatus(position);
            switch (status) {
                case Math::PointStatus::PSAbove:
                    mark = Mark_Drop;
                    break;
                case Math::PointStatus::PSBelow:
                    mark = Mark_Keep;
                    break;
                default:
                    mark = Mark_Undecided;
                    break;
            }
        }

        BrushFaceGeometryList BrushVertex::incidentSides(const BrushEdgeList& edges) const {
            BrushFaceGeometryList result;
            
            BrushEdge* edge = NULL;
            BrushEdgeList::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd && edge == NULL; ++eIt) {
                BrushEdge* candidate = *eIt;
                if (candidate->start == this || candidate->end == this)
                    edge = candidate;
            }
            
            assert(edge != NULL);
            
            // iterate over the incident sides in clockwise order
            BrushFaceGeometry* side = edge->start == this ? edge->right : edge->left;
            do {
                result.push_back(side);
                const size_t index = VectorUtils::indexOf(side->edges, edge);
                
                const BrushEdgeList& sideEdges = side->edges;
                edge = sideEdges[Math::pred(index, sideEdges.size())];
                side = edge->start == this ? edge->right : edge->left;
            } while (side != result.front());
            
            return result;
        }

        Vec3 centerOfVertices(const BrushVertexList& vertices) {
            assert(!vertices.empty());
            Vec3 center = vertices[0]->position;
            for (size_t i = 1; i < vertices.size(); ++i)
                center += vertices[i]->position;
            return center / static_cast<FloatType>(vertices.size());
        }
        
        Vec3::List vertexPositions(const BrushVertexList& vertices) {
            const size_t count = vertices.size();
            Vec3::List positions(count);
            for (size_t i = 0; i < count; ++i)
                positions[i] = vertices[i]->position;
            return positions;
        }

        BrushVertexList::iterator findBrushVertex(BrushVertexList& vertices, const Vec3& position) {
            BrushVertexList::iterator it = vertices.begin();
            const BrushVertexList::iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position == position)
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
                if (vertex.position == position)
                    return it;
                ++it;
            }
            return end;
        }

        Math::PointStatus::Type pointStatus(const Plane3& plane, const BrushVertexList& vertices) {
            size_t above = 0;
            size_t below = 0;
            for (size_t i = 0; i < vertices.size(); ++i) {
                const Math::PointStatus::Type status = plane.pointStatus(vertices[i]->position);
                if (status == Math::PointStatus::PSAbove)
                    ++above;
                else if (status == Math::PointStatus::PSBelow)
                    ++below;
                if (above > 0 && below > 0)
                    return Math::PointStatus::PSInside;
            }
            return above > 0 ? Math::PointStatus::PSAbove : Math::PointStatus::PSBelow;
        }
    }
}
