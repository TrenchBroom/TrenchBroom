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

#ifndef __TrenchBroom__BrushVertex__
#define __TrenchBroom__BrushVertex__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Allocator.h"
#include "Model/BrushVertex.h"
#include "Model/BrushGeometryTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushVertex : public Allocator<BrushVertex> {
        public:
            struct GetPosition {
                const Vec3& operator()(const BrushVertex* vertex) const { return vertex->position; }
            };
        public:
            typedef enum {
                Mark_Drop,
                Mark_Keep,
                Mark_Undecided,
                Mark_New
            } Mark;

            Vec3 position;
            Mark mark;
        public:
            BrushVertex(const Vec3& position);
            
            void updateMark(const Plane3& plane);
            BrushFaceGeometryList incidentSides(const BrushEdgeList& edges) const;
        };
        
        Vec3 centerOfVertices(const BrushVertexList& vertices);
        Vec3::List vertexPositions(const BrushVertexList& vertices);
        BrushVertexList::iterator findBrushVertex(BrushVertexList& vertices, const Vec3& position, FloatType epsilon = 0.0);
        BrushVertexList::const_iterator findBrushVertex(const BrushVertexList& vertices, const Vec3& position, FloatType epsilon = 0.0);
        Math::PointStatus::Type pointStatus(const Plane3& plane, const BrushVertexList& vertices);
    }
}

#endif /* defined(__TrenchBroom__BrushVertex__) */
