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

#include "SnapBrushVerticesAlgorithm.h"

#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        SnapBrushVerticesAlgorithm::SnapBrushVerticesAlgorithm(BrushGeometry& geometry, const Vec3::List& vertexPositions, const size_t snapTo) :
        MoveBrushVertexAlgorithm(geometry),
        m_snapTo(snapTo) {
            m_vertexPositions.insert(vertexPositions.begin(), vertexPositions.end());
            assert(!m_vertexPositions.empty());
            assert(m_snapTo > 0);
        }

        bool SnapBrushVerticesAlgorithm::doCanExecute(BrushGeometry& geometry) {
            return true;
        }
        
        SnapVerticesResult SnapBrushVerticesAlgorithm::doExecute(BrushGeometry& geometry) {
            const Vec3::Map positions = moveableVertices(geometry);
            if (positions.empty())
                return SnapVerticesResult(Vec3::List(0));
            
            Vec3::List newPositions;
            Vec3::Map::const_iterator positionIt, positionEnd;
            for (positionIt = positions.begin(), positionEnd = positions.end(); positionIt != positionEnd; ++positionIt) {
                const Vec3& start = positionIt->first;
                const Vec3& end = positionIt->second;
                BrushVertexList::iterator vertexIt = findBrushVertex(geometry.vertices, start);
                if (vertexIt != geometry.vertices.end()) {
                    BrushVertex* vertex = *vertexIt;
                    const MoveVertexResult result = moveVertex(geometry, vertex, true, start, end);
                    if (result.type != MoveVertexResult::Type_VertexDeleted)
                        newPositions.push_back(result.vertex->position);
                    updateFacePoints(geometry);
                }
            }

            updateNewAndDroppedFaces();
            return SnapVerticesResult(newPositions, m_addedFaces, m_removedFaces);
        }

        Vec3::Map SnapBrushVerticesAlgorithm::moveableVertices(const BrushGeometry& geometry) const {
            Vec3::Map result;
            for (size_t i = 0; i < geometry.vertices.size(); ++i) {
                const BrushVertex* vertex = geometry.vertices[i];
                const Vec3& start = vertex->position;
                if (m_vertexPositions.count(start) > 0) {
                    const Vec3 end = snapVertex(start, m_snapTo);
                    if (!start.equals(end, 0.0))
                        result[start] = end;
                }
            }
            return result;
        }

        Vec3 SnapBrushVerticesAlgorithm::snapVertex(const Vec3& position, const size_t snapTo) const {
            Vec3 result;
            for (size_t i = 0; i < 3; ++i)
                result[i] = snapTo * Math::round(position[i] / snapTo);
            return result;
        }
    }
}
