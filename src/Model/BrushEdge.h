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
                Drop,
                Keep,
                Split,
                Undecided,
                New
            };
        private:
            BrushVertex* m_start;
            BrushVertex* m_end;
            BrushFaceGeometry* m_left;
            BrushFaceGeometry* m_right;
            Mark m_mark;
            
            friend class BrushFaceGeometry;
        public:
            BrushEdge(BrushVertex* start, BrushVertex* end);
            ~BrushEdge();
            
            const BrushVertex* start() const;
            BrushVertex* start();
            const BrushVertex* end() const;
            BrushVertex* end();
            const BrushFaceGeometry* left() const;
            BrushFaceGeometry* left();
            const BrushFaceGeometry* right() const;
            BrushFaceGeometry* right();
            const BrushFace* leftFace() const;
            BrushFace* leftFace();
            const BrushFace* rightFace() const;
            BrushFace* rightFace();
            Mark mark() const;
            
            void updateMark();
            BrushVertex* split(const Plane3& plane);
            
            void flip();
            void setLeftNull();
            void setRightNull();
            
            const BrushVertex* start(const BrushFaceGeometry* side) const;
            BrushVertex* start(const BrushFaceGeometry* side);
            const BrushVertex* end(const BrushFaceGeometry* side) const;
            BrushVertex* end(const BrushFaceGeometry* side);
            
            bool hasPositions(const Vec3& position1, const Vec3& position2) const;
            bool contains(const Vec3& point, const FloatType maxDistance = Math::Constants<FloatType>::AlmostZero) const;
            Vec3 vector() const;
        };

        BrushEdgeList::iterator findBrushEdge(BrushEdgeList& edges, const Vec3& position1, const Vec3& position2);
        BrushEdgeList::const_iterator findBrushEdge(const BrushEdgeList& edges, const Vec3& position1, const Vec3& position2);
    }
}

#endif /* defined(__TrenchBroom__BrushEdge__) */
