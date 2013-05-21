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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__BrushEdge__
#define __TrenchBroom__BrushEdge__

#include "VecMath.h"
#include "TrenchBroom.h"
#include "Model/BrushGeometryTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushVertex;
        class BrushFaceGeometry;
        
        class BrushEdge {
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
            
            inline const BrushVertex* start() const {
                return m_start;
            }
            
            inline BrushVertex* start() {
                return m_start;
            }

            inline const BrushVertex* end() const {
                return m_end;
            }
            
            inline BrushVertex* end() {
                return m_end;
            }
            
            inline const BrushFaceGeometry* left() const {
                return m_left;
            }
            
            inline BrushFaceGeometry* left() {
                return m_left;
            }
            
            inline const BrushFaceGeometry* right() const {
                return m_right;
            }

            inline BrushFaceGeometry* right() {
                return m_right;
            }
            
            inline const Mark mark() const {
                return m_mark;
            }
            
            void updateMark();
            BrushVertex* split(const Plane3& plane);
            
            void flip();
            
            const BrushVertex* start(const BrushFaceGeometry* side) const;
            BrushVertex* start(const BrushFaceGeometry* side);
            const BrushVertex* end(const BrushFaceGeometry* side) const;
            BrushVertex* end(const BrushFaceGeometry* side);
            
            bool hasPositions(const Vec3& position1, const Vec3& position2) const;
        };

        inline BrushEdgeList::iterator findBrushEdge(BrushEdgeList& edges, const Vec3& position1, const Vec3& position2) {
            BrushEdgeList::iterator it = edges.begin();
            const BrushEdgeList::iterator end = edges.end();
            while (it != end) {
                const BrushEdge& edge = **it;
                if (edge.hasPositions(position1, position2))
                    return it;
                ++it;
            }
            return end;
        }
        
        inline BrushEdgeList::const_iterator findBrushEdge(const BrushEdgeList& edges, const Vec3& position1, const Vec3& position2) {
            BrushEdgeList::const_iterator it = edges.begin();
            const BrushEdgeList::const_iterator end = edges.end();
            while (it != end) {
                const BrushEdge& edge = **it;
                if (edge.hasPositions(position1, position2))
                    return it;
                ++it;
            }
            return end;
        }
    }
}

#endif /* defined(__TrenchBroom__BrushEdge__) */
