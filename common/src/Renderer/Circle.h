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

#ifndef TrenchBroom_Circle
#define TrenchBroom_Circle

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class Circle {
        private:
            VertexArray m_array;
            bool m_filled;
        public:
            Circle(float radius, size_t segments, bool filled);
            Circle(float radius, size_t segments, bool filled, float startAngle, float angleLength);
            Circle(float radius, size_t segments, bool filled, Math::Axis::Type axis, const Vec3f& startAxis, const Vec3f& endAxis);
            Circle(float radius, size_t segments, bool filled, Math::Axis::Type axis, float startAngle, float angleLength);
            
            bool prepared() const;
            void prepare(Vbo& vbo);
            void render();
        private:
            void init3D(float radius, size_t segments, Math::Axis::Type axis, float startAngle, float angleLength);
            void init2D(float radius, size_t segments, float startAngle, float angleLength);
        };
    }
}

#endif /* defined(TrenchBroom_Circle) */
