/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__Ring__
#define __TrenchBroom__Ring__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "MathUtils.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class Ring {
        private:
            VertexArray m_array;
        public:
            Ring(const float radius, const float width, const float startAngle, const float angleLength, const size_t segments);
            
            void prepare(Vbo& vbo);
            void render();
        };
    }
}

#endif /* defined(__TrenchBroom__Ring__) */
