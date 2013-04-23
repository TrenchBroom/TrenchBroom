/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__RingFigure__
#define __TrenchBroom__RingFigure__

#include "Renderer/Figure.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class VertexArray;
        
        class RingFigure : public Figure {
        private:
            Axis::Type m_normal;
            float m_startAngle;
            float m_angleLength;
            float m_innerRadius;
            float m_outerRadius;
            unsigned int m_segments;
            VertexArray* m_vertexArray;
        public:
            RingFigure(Axis::Type normal, float startAngle, float angleLength, float radius, float thickness, unsigned int segments);
            RingFigure(Axis::Type normal, const Vec3f& startAxis, const Vec3f& endAxis, float radius, float thickness, unsigned int segments);
            ~RingFigure();
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__RingFigure__) */
