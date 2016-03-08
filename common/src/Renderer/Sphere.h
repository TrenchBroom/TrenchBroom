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

#ifndef TrenchBroom_Sphere
#define TrenchBroom_Sphere

#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class Sphere {
        private:
            VertexArray m_array;
        public:
            Sphere(float radius, size_t iterations);

            bool prepared() const;
            void prepare(Vbo& vbo);
            void render();
        };
    }
}

#endif /* defined(TrenchBroom_Sphere) */
