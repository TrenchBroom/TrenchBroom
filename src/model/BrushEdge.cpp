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

#include "BrushEdge.h"

#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        BrushEdge::BrushEdge(BrushVertex* start, BrushVertex* end) :
        m_start(start),
        m_end(end) {}
        
        BrushEdge::~BrushEdge() {
            m_start = NULL;
            m_end = NULL;
        }

        bool BrushEdge::hasPositions(const Vec3& position1, const Vec3& position2) const {
            if (m_start == NULL || m_end == NULL)
                return false;
            return (m_start->position() == position1 && m_end->position() == position2) || (m_start->position() == position2 && m_end->position() == position1);
                    
        }
    }
}
