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

#ifndef TrenchBroom_RenderUtils_h
#define TrenchBroom_RenderUtils_h

#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        void glSetEdgeOffset(const float f);
        void glResetEdgeOffset();

        Vec2f::List circle2D(const float radius, const float startAngle, const float angleLength, const size_t segments);
        Vec2f::List roundedRect2D(const float width, const float height, const float cornerRadius, const size_t cornerSegments);
        
        struct VertsAndNormals {
            Vec3f::List vertices;
            Vec3f::List normals;
            
            VertsAndNormals(const size_t vertexCount);
        };
        
        Vec3f::List sphere3D(const float radius, const size_t iterations);
        VertsAndNormals circle3D(const float radius, const size_t segments);
        VertsAndNormals cylinder3D(const float radius, const float length, const size_t segments);
        VertsAndNormals cone3D(const float radius, const float length, const size_t segments);
    }
}

#endif
