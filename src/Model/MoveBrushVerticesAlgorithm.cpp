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

#include "MoveBrushVerticesAlgorithm.h"

#include "CollectionUtils.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        MoveBrushVerticesAlgorithm::MoveBrushVerticesAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta) :
        MoveBrushVertexAlgorithm(geometry),
        m_worldBounds(worldBounds),
        m_vertexPositions(vertexPositions),
        m_delta(delta) {}

        bool MoveBrushVerticesAlgorithm::doCanExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return true;
            
            BrushGeometry testGeometry(geometry);
            testGeometry.restoreFaceGeometries();
            
            Vec3::List sortedVertexPositions = m_vertexPositions;
            std::sort(sortedVertexPositions.begin(), sortedVertexPositions.end(), Vec3::InverseDotOrder(m_delta));
            
            bool canMove = true;
            Vec3::List::const_iterator positionIt, positionEnd;
            for (positionIt = sortedVertexPositions.begin(), positionEnd = sortedVertexPositions.end(); positionIt != positionEnd && canMove; ++positionIt) {
                const Vec3& vertexPosition = *positionIt;
                BrushVertexList::iterator vertexIt = findBrushVertex(testGeometry.vertices, vertexPosition);
                assert(vertexIt != testGeometry.vertices.end());
                BrushVertex* vertex = *vertexIt;
                assert(vertex != NULL);
                
                const Vec3 start = vertex->position;
                const Vec3 end = start + m_delta;
                
                MoveVertexResult result = moveVertex(testGeometry, vertex, true, start, end);
                canMove = result.type != MoveVertexResult::Type_VertexUnchanged;
            }
            
            canMove &= testGeometry.sides.size() >= 3;
            canMove &= m_worldBounds.contains(testGeometry.bounds);
            
            geometry.restoreFaceGeometries();
            return canMove;
        }
        
        MoveVerticesResult MoveBrushVerticesAlgorithm::doExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return MoveVerticesResult(m_vertexPositions);
            
            BrushVertexList movedVertices;
            Vec3::List sortedVertexPositions = m_vertexPositions;
            std::sort(sortedVertexPositions.begin(), sortedVertexPositions.end(), Vec3::InverseDotOrder(m_delta));
            
            Vec3::List::const_iterator positionIt, positionEnd;
            for (positionIt = sortedVertexPositions.begin(), positionEnd = sortedVertexPositions.end(); positionIt != positionEnd; ++positionIt) {
                const Vec3& vertexPosition = *positionIt;
                BrushVertexList::iterator vertexIt = findBrushVertex(geometry.vertices, vertexPosition);
                assert(vertexIt != geometry.vertices.end());
                BrushVertex* vertex = *vertexIt;
                assert(vertex != NULL);
                
                const Vec3 start = vertex->position;
                const Vec3 end = start + m_delta;
                
                MoveVertexResult result = moveVertex(geometry, vertex, true, start, end);
                if (result.type == MoveVertexResult::Type_VertexMoved)
                    movedVertices.push_back(result.vertex);
                updateFacePoints(geometry);
            }
            
            Vec3::List newVertexPositions(movedVertices.size());
            for (size_t i = 0; i < movedVertices.size(); ++i)
                newVertexPositions[i] = movedVertices[i]->position;
            
            updateNewAndDroppedFaces();
            return MoveVerticesResult(newVertexPositions, m_addedFaces, m_removedFaces);
        }
    }
}
