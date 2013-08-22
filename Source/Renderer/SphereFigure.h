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

#ifndef __TrenchBroom__SphereFigure__
#define __TrenchBroom__SphereFigure__

#include "Renderer/Figure.h"
#include "Utility/Color.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <map>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VertexArray;
        
        class SphereFigure : public Figure {
        protected:
            float m_radius;
            unsigned int m_iterations;
            VertexArray* m_vertexArray;
        public:
            SphereFigure(float radius, unsigned int iterations);
            ~SphereFigure();
                    
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__SphereFigure__) */
