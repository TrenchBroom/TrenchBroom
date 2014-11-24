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

#ifndef TrenchBroom_RenderUtils_h
#define TrenchBroom_RenderUtils_h

#include "Color.h"
#include "VecMath.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

#include <utility>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        void glSetEdgeOffset(float f);
        void glResetEdgeOffset();

        VertexSpecs::P3C4::Vertex::List coordinateSystem(const BBox3f& bounds, const Color& x, const Color& y, const Color& z);
        
        Vec2f::List circle2D(float radius, size_t segments);
        Vec2f::List circle2D(float radius, float startAngle, float angleLength, size_t segments);
        Vec3f::List circle2D(float radius, Math::Axis::Type axis, float startAngle, float angleLength, size_t segments);
        std::pair<float, float> startAngleAndLength(const Math::Axis::Type axis, const Vec3f& startAxis, const Vec3f& endAxis);

        size_t roundedRect2DVertexCount(size_t cornerSegments);
        Vec2f::List roundedRect2D(float width, float height, float cornerRadius, size_t cornerSegments);
        
        struct VertsAndNormals {
            Vec3f::List vertices;
            Vec3f::List normals;
            
            VertsAndNormals(size_t vertexCount);
        };
        
        Vec3f::List sphere3D(float radius, size_t iterations);
        VertsAndNormals circle3D(float radius, size_t segments);
        VertsAndNormals cylinder3D(float radius, float length, size_t segments);
        VertsAndNormals cone3D(float radius, float length, size_t segments);
    }
}

#endif
