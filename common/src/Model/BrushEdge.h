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

#ifndef __TrenchBroom__BrushEdge__
#define __TrenchBroom__BrushEdge__

#include "VecMath.h"
#include "TrenchBroom.h"
#include "Allocator.h"
#include "Model/BrushGeometryTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushVertex;
        class BrushFaceGeometry;

        class BrushEdge : public Allocator<BrushEdge> {
        public:
            enum Mark {
                Mark_Drop,
                Mark_Keep,
                Mark_Split,
                Mark_Undecided,
                Mark_New
            };

            struct EdgeDistance {
            };

            BrushVertex* start;
            BrushVertex* end;
            BrushFaceGeometry* left;
            BrushFaceGeometry* right;
            Mark mark;
        public:
            BrushEdge(BrushVertex* start, BrushVertex* end);
            BrushEdge(BrushVertex* start, BrushVertex* end, BrushFaceGeometry* left, BrushFaceGeometry* right);
            ~BrushEdge();
            
            const BrushFace* leftFace() const;
            BrushFace* leftFace();
            const BrushFace* rightFace() const;
            BrushFace* rightFace();
            
            void updateMark();
            BrushVertex* split(const Plane3& plane);
            
            void flip();
            void setLeftNull();
            void setRightNull();
            void replaceSide(BrushFaceGeometry* oldSide, BrushFaceGeometry* newSide);
            
            const BrushVertex* startVertex(const BrushFaceGeometry* side) const;
            BrushVertex* startVertex(const BrushFaceGeometry* side);
            const BrushVertex* endVertex(const BrushFaceGeometry* side) const;
            BrushVertex* endVertex(const BrushFaceGeometry* side);
            
            bool hasPositions(const Vec3& position1, const Vec3& position2) const;
            bool isIncidentWith(const BrushEdge* edge) const;
            bool connects(const BrushVertex* vertex1, BrushVertex* vertex2) const;
            bool contains(const Vec3& point, const FloatType maxDistance = Math::C::almostZero()) const;
            Vec3::EdgeDistance distanceTo(const Vec3& point) const;

            Vec3 vector() const;
            Vec3 center() const;
            
            Edge3 edgeInfo() const;
        };

        BrushEdgeList::iterator findBrushEdge(BrushEdgeList& edges, const Vec3& position1, const Vec3& position2);
        BrushEdgeList::const_iterator findBrushEdge(const BrushEdgeList& edges, const Vec3& position1, const Vec3& position2);
    }
}

#endif /* defined(__TrenchBroom__BrushEdge__) */
