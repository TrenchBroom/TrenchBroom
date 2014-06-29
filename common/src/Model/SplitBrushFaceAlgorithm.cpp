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

#include "SplitBrushFaceAlgorithm.h"

#include "CollectionUtils.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        SplitBrushFaceAlgorithm::SplitBrushFaceAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Polygon3& face, const Vec3& delta) :
        MoveBrushVertexAlgorithm(geometry),
        m_worldBounds(worldBounds),
        m_face(face),
        m_delta(delta) {}
        
        bool SplitBrushFaceAlgorithm::doCanExecute(BrushGeometry& geometry) {
            if (m_delta.null())
                return false;
            
            BrushFaceGeometryList::iterator faceIt = findBrushFaceGeometry(geometry.sides, m_face.vertices());
            if (faceIt == geometry.sides.end())
                return false;
            
            BrushFaceGeometry* side = *faceIt;
            assert(side != NULL);
            BrushFace* face = side->face;
            assert(face != NULL);
            
            // detect whether the drag would lead to an indented face
            const Vec3& norm = face->boundary().normal;
            if (Math::zero(m_delta.dot(norm)))
                return false;
            
            BrushGeometry testGeometry(geometry);
            testGeometry.restoreFaceGeometries();

            BrushVertex* newVertex = splitFace(testGeometry, face);
            const Vec3 start = newVertex->position;
            const Vec3 end = start + m_delta;
            const MoveVertexResult result = moveVertex(testGeometry, newVertex, false, start, end);
            
            bool canSplit = result.type == MoveVertexResult::Type_VertexMoved;
            canSplit &= testGeometry.sides.size() >= 3;
            canSplit &= m_worldBounds.contains(testGeometry.bounds);
            
            geometry.restoreFaceGeometries();
            return canSplit;
        }
        
        SplitResult SplitBrushFaceAlgorithm::doExecute(BrushGeometry& geometry) {
            assert(!m_delta.null());
            
            BrushFaceGeometryList::iterator faceIt = findBrushFaceGeometry(geometry.sides, m_face.vertices());
            assert(faceIt != geometry.sides.end());
            BrushFaceGeometry* side = *faceIt;
            assert(side != NULL);
            BrushFace* face = side->face;
            assert(face != NULL);
            
            BrushVertex* newVertex = splitFace(geometry, face);
            const Vec3 start = newVertex->position;
            const Vec3 end = start + m_delta;
            const MoveVertexResult result = moveVertex(geometry, newVertex, false, start, end);
            assert(result.type == MoveVertexResult::Type_VertexMoved);
            
            updateNewAndDroppedFaces();
            return SplitResult(result.vertex->position, m_addedFaces, m_removedFaces);
        }
        
        BrushVertex* SplitBrushFaceAlgorithm::splitFace(BrushGeometry& geometry, BrushFace* face) {
            BrushFaceGeometry* side = face->side();
            assert(side != NULL);
            
            // create a new vertex
            BrushVertex* newVertex = new BrushVertex(centerOfVertices(side->vertices));
            geometry.vertices.push_back(newVertex);
            
            // create the new edges
            BrushEdge* firstEdge = new BrushEdge(newVertex, side->edges[0]->startVertex(side));
            geometry.edges.push_back(firstEdge);
            
            BrushEdge* lastEdge = firstEdge;
            for (size_t i = 0; i < side->edges.size(); ++i) {
                BrushEdge* sideEdge = side->edges[i];
                
                BrushEdge* newEdge;
                if (i == side->edges.size() - 1) {
                    newEdge = firstEdge;
                } else {
                    newEdge = new BrushEdge(newVertex, sideEdge->endVertex(side));
                    geometry.edges.push_back(newEdge);
                }
                
                BrushFaceGeometry* newSide = new BrushFaceGeometry();
                newSide->vertices.push_back(newVertex);
                newSide->edges.push_back(lastEdge);
                lastEdge->right = newSide;
                
                newSide->vertices.push_back(lastEdge->end);
                newSide->edges.push_back(sideEdge);
                if (sideEdge->left == side)
                    sideEdge->left = newSide;
                else
                    sideEdge->right = newSide;
                
                newSide->vertices.push_back(newEdge->end);
                newSide->edges.push_back(newEdge);
                newEdge->left = newSide;
                
                newSide->face = side->face->clone();
                newSide->face->setSide(newSide);
                geometry.sides.push_back(newSide);
                m_faceManager.addFace(side->face, newSide->face);
                
                lastEdge = newEdge;
            }
            
            // delete the side that was split
            m_faceManager.dropFace(side);
            assert(VectorUtils::contains(geometry.sides, side));
            VectorUtils::removeAndDelete(geometry.sides, side);
            
            return newVertex;
        }
    }
}
