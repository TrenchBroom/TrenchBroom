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

#ifndef __TrenchBroom__BrushGeometry__
#define __TrenchBroom__BrushGeometry__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushEdge.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry {
        public:
            typedef enum {
                BrushIsSplit,
                BrushIsNull,
                FaceIsRedundant
            } AddFaceResultCode;

            struct Result {
                BrushFaceList addedFaces;
                BrushFaceList droppedFaces;
                void append(const Result& other);
            protected:
                Result(const BrushFaceList& i_addedFaces, const BrushFaceList& i_droppedFaces);
            };
            
            struct AddFaceResult : public Result {
                AddFaceResultCode resultCode;
                AddFaceResult(AddFaceResultCode i_resultCode, const BrushFaceList& i_addedFaces = EmptyBrushFaceList, const BrushFaceList& i_droppedFaces = EmptyBrushFaceList);
            };
            
            struct MoveVerticesResult : public Result {
                Vec3::List newVertexPositions;
                MoveVerticesResult(Vec3::List& i_newVertexPositions, const BrushFaceList& i_addedFaces = EmptyBrushFaceList, const BrushFaceList& i_droppedFaces = EmptyBrushFaceList);
            };
            
            struct MoveEdgesResult : public Result {
                Edge3::List newEdgePositions;
                MoveEdgesResult(Edge3::List& i_newEdgePositions, const BrushFaceList& i_addedFaces = EmptyBrushFaceList, const BrushFaceList& i_droppedFaces = EmptyBrushFaceList);
            };
            
            struct MoveFacesResult : public Result {
                Polygon3::List newFacePositions;
                MoveFacesResult(Polygon3::List& i_newFacePositions, const BrushFaceList& i_addedFaces = EmptyBrushFaceList, const BrushFaceList& i_droppedFaces = EmptyBrushFaceList);
            };
            
            struct SplitResult : public Result {
                Vec3 newVertexPosition;
                SplitResult(const Vec3& i_newVertexPosition, const BrushFaceList& i_addedFaces = EmptyBrushFaceList, const BrushFaceList& i_droppedFaces = EmptyBrushFaceList);
            };
        public:
            BrushVertexList vertices;
            BrushEdgeList edges;
            BrushFaceGeometryList sides;
            BBox3 bounds;
        public:
            BrushGeometry(const BrushGeometry& original);
            BrushGeometry(const BBox3& worldBounds);
            ~BrushGeometry();
            
            BrushFaceGeometryList incidentSides(const BrushVertex* vertex) const;
            AddFaceResult addFaces(const BrushFaceList& faces);
            
            bool canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);
            MoveVerticesResult moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);
            
            bool canMoveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta);
            MoveEdgesResult moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta);
            
            bool canMoveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta);
            MoveEdgesResult moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta);
            
            bool canSplitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta);
            SplitResult splitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta);
            
            bool canSplitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta);
            SplitResult splitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta);
            
            void restoreFaceGeometries();
            void updateBounds();

            bool sanityCheck() const;
        private:
            void copy(const BrushGeometry& original);
            AddFaceResult addFace(BrushFace* face);
            void initializeWithBounds(const BBox3& bounds);
        };
    }
}

#endif /* defined(__TrenchBroom__BrushGeometry__) */
