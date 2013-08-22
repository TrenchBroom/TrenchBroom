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

#ifndef __TrenchBroom__PointHandleHighlightFigure__
#define __TrenchBroom__PointHandleHighlightFigure__

#include "Renderer/Figure.h"

#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VertexArray;
        
        class PointHandleHighlightFigure : public Figure {
        protected:
            Vec3f::List m_positions;
            Color m_color;
            float m_radius;
            float m_scalingFactor;
        public:
            PointHandleHighlightFigure(const Vec3f& position, const Color& color, float radius, float scalingFactor);
            PointHandleHighlightFigure(const Vec3f::List& positions, const Color& color, float radius, float scalingFactor);
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__PointHandleHighlightFigure__) */
