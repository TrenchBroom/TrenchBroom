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

#ifndef __TrenchBroom__BrushFaceGeometry__
#define __TrenchBroom__BrushFaceGeometry__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Allocator.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/BrushVertex.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        
        class BrushFaceGeometry : public Allocator<BrushFaceGeometry> {
        public:
            typedef enum {
                Mark_Keep,
                Mark_Drop,
                Mark_Split
            } Mark;

            BrushVertexList vertices;
            BrushEdgeList edges;
            BrushFace* face;
        public:
            BrushFaceGeometry();
            ~BrushFaceGeometry();

            Mark mark() const;
            BrushEdge* splitUsingEdgeMarks();
            BrushEdge* findUndecidedEdge() const;
            
            void addForwardEdge(BrushEdge* edge);
            void addForwardEdges(const BrushEdgeList& edges);
            void addBackwardEdge(BrushEdge* edge);
            void addBackwardEdges(const BrushEdgeList& edges);
            void addEdge(BrushEdge* edge, bool forward);
            
            bool containsDroppedEdge() const;
            bool isClosed() const;
            bool hasVertexPositions(const Vec3::List& positions) const;
            size_t isColinearTriangle() const;
            
            void chop(size_t vertexIndex, BrushFaceGeometry*& newSide, BrushEdge*& newEdge);
            void shift(size_t offset);
            void replaceEdgesWithEdge(size_t index1, size_t index2, BrushEdge* edge);
            
            Polygon3 faceInfo() const;
        private:
            void replaceEdgesWithBackwardEdge(const BrushEdgeList::iterator it1, const BrushEdgeList::iterator it2, BrushEdge* edge);
            void replaceEdgesWithEdge(const BrushEdgeList::iterator it1, const BrushEdgeList::iterator it2, BrushEdge* edge);
            void updateVerticesFromEdges();
        };
        
        BrushFaceGeometryList::iterator findBrushFaceGeometry(BrushFaceGeometryList& faceGeometries, const Vec3::List& positions);
        BrushFaceGeometryList::const_iterator findBrushFaceGeometry(const BrushFaceGeometryList& faceGeometries, const Vec3::List& positions);
    }
}

#endif /* defined(__TrenchBroom__BrushFaceGeometry__) */
