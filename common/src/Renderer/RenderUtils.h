/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
#include "Color.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

#include <utility>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class Vbo;
        
        void glSetEdgeOffset(float f);
        void glResetEdgeOffset();

        void coordinateSystemVerticesX(const BBox3f& bounds, vec3f& start, vec3f& end);
        void coordinateSystemVerticesY(const BBox3f& bounds, vec3f& start, vec3f& end);
        void coordinateSystemVerticesZ(const BBox3f& bounds, vec3f& start, vec3f& end);
        
        class TextureRenderFunc {
        public:
            virtual ~TextureRenderFunc();
            virtual void before(const Assets::Texture* texture);
            virtual void after(const Assets::Texture* texture);
        };
        
        class DefaultTextureRenderFunc : public TextureRenderFunc {
        public:
            void before(const Assets::Texture* texture) override;
            void after(const Assets::Texture* texture) override;
        };

        vec2f::List circle2D(float radius, size_t segments);
        vec2f::List circle2D(float radius, float startAngle, float angleLength, size_t segments);
        vec3f::List circle2D(float radius, Math::Axis::Type axis, float startAngle, float angleLength, size_t segments);
        std::pair<float, float> startAngleAndLength(const Math::Axis::Type axis, const vec3f& startAxis, const vec3f& endAxis);

        size_t roundedRect2DVertexCount(size_t cornerSegments);
        vec2f::List roundedRect2D(const vec2f& size, const float cornerRadius, const size_t cornerSegments);
        vec2f::List roundedRect2D(float width, float height, float cornerRadius, size_t cornerSegments);
        
        struct VertsAndNormals {
            vec3f::List vertices;
            vec3f::List normals;
            
            VertsAndNormals(size_t vertexCount);
        };
        
        vec3f::List sphere3D(float radius, size_t iterations);
        VertsAndNormals circle3D(float radius, size_t segments);
        VertsAndNormals cylinder3D(float radius, float length, size_t segments);
        VertsAndNormals cone3D(float radius, float length, size_t segments);
    }
}

#endif
