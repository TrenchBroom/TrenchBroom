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

#ifndef __TrenchBroom__Circle__
#define __TrenchBroom__Circle__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class Circle {
        private:
            VertexArray m_array;
        public:
            Circle(Vbo& vbo, const float radius, const size_t segments, const bool filled);
            Circle(Vbo& vbo, const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength);
            Circle(Vbo& vbo, const float radius, const size_t segments, const bool filled, const Vec3f& startAxis, const Vec3f& endAxis);
            
            void prepare();
            void render();
        private:
            void init(Vbo& vbo, const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength);
        };
    }
}

#endif /* defined(__TrenchBroom__Circle__) */
