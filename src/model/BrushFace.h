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

#ifndef __TrenchBroom__Face__
#define __TrenchBroom__Face__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushTypes.h"
#include "Model/BrushFaceTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace {
        private:
            BrushFacePoints m_points;
            Plane3 m_boundary;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            BrushEdgeList m_edges;
        public:
            BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2);
            
            const BrushFacePoints& points() const;
            const Plane3& boundary() const;
            
            const float xOffset() const;
            const float yOffset() const;
            const float rotation() const;
            const float xScale() const;
            const float yScale() const;
            void setXOffset(const float xOffset);
            void setYOffset(const float yOffset);
            void setRotation(const float rotation);
            void setXScale(const float xScale);
            void setYScale(const float yScale);
            
            void setEdges(const BrushEdgeList& edges);
        private:
            void setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2);
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */
