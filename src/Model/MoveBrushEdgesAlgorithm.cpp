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

#include "MoveBrushEdgesAlgorithm.h"

#include "CollectionUtils.h"
#include "Macros.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        MoveBrushEdgesAlgorithm::MoveBrushEdgesAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Edge3::List& edges, const Vec3& delta) :
        MoveBrushVertexAlgorithm(geometry),
        m_worldBounds(worldBounds),
        m_edges(edges),
        m_delta(delta) {}
        
        bool MoveBrushEdgesAlgorithm::doCanExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return true;
            
            BrushGeometry testGeometry(geometry);
            testGeometry.restoreFaceGeometries();
            
            Vec3::List sortedVertexPositions = Edge3::asVertexList(m_edges);
            std::sort(sortedVertexPositions.begin(), sortedVertexPositions.end(), Vec3::InverseDotOrder(m_delta));
            
            bool canMove = true;
            Vec3::List::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd && canMove; ++vertexIt) {
                const Vec3& vertexPosition = *vertexIt;
                BrushVertexList::iterator vertexIt = findBrushVertex(testGeometry.vertices, vertexPosition);
                if (vertexIt == testGeometry.vertices.end()) {
                    canMove = false;
                    break;
                }
                
                BrushVertex* vertex = *vertexIt;
                assert(vertex != NULL);
                
                const Vec3 start = vertex->position;
                const Vec3 end = start + m_delta;
                
                MoveVertexResult result = moveVertex(testGeometry, vertex, true, start, end);
                canMove = result.type == MoveVertexResult::Type_VertexMoved;
            }
            
            // try to find all edges by their positions after applying the delta
            Edge3::List::const_iterator it, end;
            for (it = m_edges.begin(), end = m_edges.end(); canMove && it != end; ++it) {
                const Edge3& edge = *it;
                canMove = findBrushEdge(testGeometry.edges, edge.start + m_delta, edge.end + m_delta) != testGeometry.edges.end();
            }
            
            canMove &= testGeometry.sides.size() >= 3;
            canMove &= m_worldBounds.contains(testGeometry.bounds);
            
            geometry.restoreFaceGeometries();
            return canMove;
        }
        
        MoveEdgesResult MoveBrushEdgesAlgorithm::doExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return MoveEdgesResult(m_edges);
            
            Vec3::List sortedVertexPositions = Edge3::asVertexList(m_edges);
            std::sort(sortedVertexPositions.begin(), sortedVertexPositions.end(), Vec3::InverseDotOrder(m_delta));
            
            Vec3::List::const_iterator vertexIt, vertexEnd;
            for (vertexIt = sortedVertexPositions.begin(), vertexEnd = sortedVertexPositions.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vec3& vertexPosition = *vertexIt;
                BrushVertexList::iterator vertexIt = findBrushVertex(geometry.vertices, vertexPosition);
                assert(vertexIt != geometry.vertices.end());
                BrushVertex* vertex = *vertexIt;
                assert(vertex != NULL);
                
                const Vec3 start = vertex->position;
                const Vec3 end = start + m_delta;
                
                MoveVertexResult result = moveVertex(geometry, vertex, true, start, end);
                assert(result.type == MoveVertexResult::Type_VertexMoved);
                _unused(result);
                updateFacePoints(geometry);
            }

            Edge3::List newEdges(m_edges.size());
            for (size_t i = 0; i < m_edges.size(); ++i) {
                newEdges[i] = Edge3(m_edges[i].start + m_delta,
                                    m_edges[i].end + m_delta);
                assert(findBrushEdge(geometry.edges, newEdges[i].start, newEdges[i].end) != geometry.edges.end());
            }
            
            updateNewAndDroppedFaces();
            return MoveEdgesResult(newEdges, m_addedFaces, m_removedFaces);
        }
    }
}
