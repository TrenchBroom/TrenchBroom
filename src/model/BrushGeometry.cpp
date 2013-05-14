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

#include "BrushGeometry.h"

#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        BrushGeometry::BrushGeometry(const BBox3& worldBounds, const BrushFaceList& faces) {
            initializeWithBounds(worldBounds);
        }

        BrushGeometry::~BrushGeometry() {
            VectorUtils::clearAndDelete(m_sides);
            VectorUtils::clearAndDelete(m_edges);
            VectorUtils::clearAndDelete(m_vertices);
        }

        void BrushGeometry::initializeWithBounds(const BBox3& bounds) {
            
        }
    }
}
