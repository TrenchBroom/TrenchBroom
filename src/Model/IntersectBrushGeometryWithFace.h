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

#ifndef __TrenchBroom__IntersectBrushGeometryWithFace__
#define __TrenchBroom__IntersectBrushGeometryWithFace__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushAlgorithm.h"
#include "Model/BrushGeometry.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {

        class BrushEdge;
        class BrushVertex;

        class IntersectBrushGeometryWithFace : public BrushAlgorithm<AddFaceResult::Code> {
        private:
            BrushFace* m_face;
            BrushVertexList m_remainingVertices;
            BrushVertexList m_droppedVertices;
            BrushEdgeList m_remainingEdges;
            BrushEdgeList m_droppedEdges;
            BrushEdgeList m_newSideEdges;
            BrushFaceGeometryList m_remainingSides;
            BrushFaceGeometryList m_droppedSides;
        public:
            IntersectBrushGeometryWithFace(BrushGeometry& geometry, BrushFace* face);
            
            const BrushVertexList& vertices() const;
            const BrushEdgeList& edges() const;
            const BrushFaceGeometryList& sides() const;
        private:
            AddFaceResult::Code doExecute(BrushGeometry& geometry);
            bool isFaceIdenticalWithAnySide(BrushGeometry& geometry);
            AddFaceResult::Code processVertices(BrushGeometry& geometry);
            void processEdges(BrushGeometry& geometry);
            void processSides(BrushGeometry& geometry);
            void createNewSide();
            void cleanup();
            bool checkDroppedEdges();
        };
    }
}

#endif /* defined(__TrenchBroom__IntersectBrushGeometryWithFace__) */
