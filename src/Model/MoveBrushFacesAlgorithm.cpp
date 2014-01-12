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

#include "MoveBrushFacesAlgorithm.h"

#include "CollectionUtils.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        MoveBrushFacesAlgorithm::MoveBrushFacesAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Polygon3::List& faces, const Vec3& delta) :
        MoveBrushVertexAlgorithm(geometry),
        m_worldBounds(worldBounds),
        m_faces(faces),
        m_delta(delta) {}
        
        bool MoveBrushFacesAlgorithm::doCanExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return true;
            
            BrushGeometry testGeometry(geometry);
            testGeometry.restoreFaceGeometries();
            
            Vec3::List sortedVertexPositions = Polygon3::asVertexList(m_faces);
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
                canMove = result.type == MoveVertexResult::VertexMoved;
            }
            
            // try to find all faces by their positions after applying the delta
            Polygon3::List::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); canMove && it != end; ++it) {
                const Polygon3& face = *it;
                canMove = findBrushFaceGeometry(testGeometry.sides, face.vertices + m_delta) != testGeometry.sides.end();
            }
            
            canMove &= testGeometry.sides.size() >= 3;
            canMove &= m_worldBounds.contains(testGeometry.bounds);
            
            geometry.restoreFaceGeometries();
            return canMove;
        }
        
        MoveFacesResult MoveBrushFacesAlgorithm::doExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return MoveFacesResult(m_faces);
            
            Vec3::List sortedVertexPositions = Polygon3::asVertexList(m_faces);
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
                assert(result.type == MoveVertexResult::VertexMoved);
                updateFacePoints(geometry);
            }
            
            Polygon3::List newFaces(m_faces.size());
            for (size_t i = 0; i < m_faces.size(); ++i) {
                newFaces[i] = Polygon3(m_faces[i].vertices + m_delta);
                assert(findBrushFaceGeometry(geometry.sides, newFaces[i].vertices) != geometry.sides.end());
            }
            
            updateNewAndDroppedFaces();
            return MoveFacesResult(newFaces, m_addedFaces, m_removedFaces);
        }
    }
}
