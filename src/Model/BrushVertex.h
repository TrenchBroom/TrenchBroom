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
            typedef enum {
                Drop,
                Keep,
                Undecided,
                New
            } Mark;
        private:
            Vec3 m_position;
            Mark m_mark;
        public:
            BrushVertex(const Vec3& position);
            
            inline const Vec3& position() const {
                return m_position;
            }
            
            inline Mark mark() const {
                return m_mark;
            }
            
            void updateMark(const Plane3& plane);
            BrushFaceGeometryList incidentSides(const BrushEdgeList& edges) const;
        };
        
        inline BrushVertexList::iterator findBrushVertex(BrushVertexList& vertices, const Vec3& position) {
            BrushVertexList::iterator it = vertices.begin();
            const BrushVertexList::iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position() == position)
                    return it;
                ++it;
            }
            return end;
        }

        inline BrushVertexList::const_iterator findBrushVertex(const BrushVertexList& vertices, const Vec3& position) {
            BrushVertexList::const_iterator it = vertices.begin();
            const BrushVertexList::const_iterator end = vertices.end();
            while (it != end) {
                const BrushVertex& vertex = **it;
                if (vertex.position() == position)
                    return it;
                ++it;
            }
            return end;
        }
    }
}

#endif /* defined(__TrenchBroom__BrushVertex__) */
