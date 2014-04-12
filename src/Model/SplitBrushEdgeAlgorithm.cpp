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

#include "SplitBrushEdgeAlgorithm.h"

#include "CollectionUtils.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        SplitBrushEdgeAlgorithm::SplitBrushEdgeAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Edge3& edge, const Vec3& delta) :
        MoveBrushVertexAlgorithm(geometry),
        m_worldBounds(worldBounds),
        m_edge(edge),
        m_delta(delta) {}
        
        bool SplitBrushEdgeAlgorithm::doCanExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return false;
            
            BrushEdgeList::iterator edgeIt = findBrushEdge(geometry.edges, m_edge.start, m_edge.end);
            if (edgeIt == geometry.edges.end())
                return false;
            
            BrushEdge* edge = *edgeIt;
            assert(edge != NULL);
            
            // detect whether the drag would make the incident faces invalid
            const Vec3& leftNorm = edge->left->face->boundary().normal;
            const Vec3& rightNorm = edge->right->face->boundary().normal;
            
            // we allow a bit more leeway when testing here, as otherwise edges sometimes cannot be split
            if (Math::neg(m_delta.dot(leftNorm), 0.01) ||
                Math::neg(m_delta.dot(rightNorm), 0.01))
                return false;
            
            BrushGeometry testGeometry(geometry);
            testGeometry.restoreFaceGeometries();
            
            edgeIt = findBrushEdge(testGeometry.edges, m_edge.start, m_edge.end);
            assert(edgeIt != testGeometry.edges.end());
            edge = *edgeIt;
            assert(edge != NULL);
            
            BrushVertex* newVertex = splitEdge(testGeometry, edge);
            const Vec3 start = newVertex->position;
            const Vec3 end = start + m_delta;
            const MoveVertexResult result = moveVertex(testGeometry, newVertex, false, start, end);

            bool canSplit = result.type == MoveVertexResult::Type_VertexMoved;
            canSplit &= testGeometry.sides.size() >= 3;
            canSplit &= m_worldBounds.contains(testGeometry.bounds);
            
            geometry.restoreFaceGeometries();
            return canSplit;
        }
        
        SplitResult SplitBrushEdgeAlgorithm::doExecute(BrushGeometry& geometry) {
            assert(!m_delta.null());
            
            BrushEdgeList::iterator edgeIt = findBrushEdge(geometry.edges, m_edge.start, m_edge.end);
            assert(edgeIt != geometry.edges.end());
            BrushEdge* edge = *edgeIt;
            assert(edge != NULL);
            
            BrushVertex* newVertex = splitEdge(geometry, edge);
            const Vec3 start = newVertex->position;
            const Vec3 end = start + m_delta;
            const MoveVertexResult result = moveVertex(geometry, newVertex, false, start, end);
            assert(result.type == MoveVertexResult::Type_VertexMoved);
            
            updateNewAndDroppedFaces();
            return SplitResult(result.vertex->position, m_addedFaces, m_removedFaces);
        }

        BrushVertex* SplitBrushEdgeAlgorithm::splitEdge(BrushGeometry& geometry, BrushEdge* edge) {
            // split the edge
            edge->left->shift(VectorUtils::indexOf(edge->left->edges, edge) + 1);
            edge->right->shift(VectorUtils::indexOf(edge->right->edges, edge) + 1);
            
            // create a new vertex
            BrushVertex* newVertex = new BrushVertex(edge->center());
            geometry.vertices.push_back(newVertex);
            edge->left->vertices.push_back(newVertex);
            edge->right->vertices.push_back(newVertex);
            
            // create the new edges
            BrushEdge* newEdge1 = new BrushEdge(edge->start, newVertex, edge->left, edge->right);
            BrushEdge* newEdge2 = new BrushEdge(newVertex, edge->end, edge->left, edge->right);
            geometry.edges.push_back(newEdge1);
            geometry.edges.push_back(newEdge2);
            
            // remove the split edge from the incident sides
            edge->left->edges.pop_back();
            edge->right->edges.pop_back();
            
            // add the new edges to the incident sides
            edge->left->edges.push_back(newEdge2);
            edge->left->edges.push_back(newEdge1);
            edge->right->edges.push_back(newEdge1);
            edge->right->edges.push_back(newEdge2);
            
            // delete the split edge
            VectorUtils::remove(geometry.edges, edge);
            delete edge;
            
            return newVertex;
        }
    }
}
