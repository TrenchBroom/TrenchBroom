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

#ifndef __TrenchBroom__BrushGeometry__
#define __TrenchBroom__BrushGeometry__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/BrushFaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry {
        private:
            BrushVertexList m_vertices;
            BrushEdgeList m_edges;
            BrushFaceGeometryList m_sides;
        public:
            BrushGeometry(const BBox3& worldBounds, const BrushFaceList& faces);
            ~BrushGeometry();
            
            inline const BrushVertexList& vertices() const {
                return m_vertices;
            }
            
            inline const BrushEdgeList& edges() const {
                return m_edges;
            }
            
            inline const BrushFaceGeometryList& sides() const {
                return m_sides;
            }
        private:
            void initializeWithBounds(const BBox3& bounds);
        };
    }
}

#endif /* defined(__TrenchBroom__BrushGeometry__) */
