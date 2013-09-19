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

#include "RenderUtils.h"

#include "GL/GL.h"

namespace TrenchBroom {
    namespace Renderer {
        static const float EdgeOffset = 0.0001f;

        void glSetEdgeOffset(const float f) {
            glDepthRange(0.0f, 1.0f - EdgeOffset * f);
        }
        
        void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0f);
        }
        
        Vec2f::List roundedRect(const float width, const float height, const float cornerRadius, const size_t cornerSegments) {
            assert(cornerSegments > 0);
            assert(cornerRadius <= width / 2.0f &&
                   cornerRadius <= height / 2.0f);
            
            Vec2f::List vertices;
            vertices.resize(4 * (3 * cornerSegments + 3));
            size_t vertexIndex = 0;
            
            const float angle = Math::Constants<float>::PiOverTwo / cornerSegments;
            Vec2f center(0.0f, 0.0f);
            Vec2f translation;
            
            float curAngle = 0.0f;
            float x = std::cos(curAngle) * cornerRadius;
            float y = std::sin(curAngle) * cornerRadius;
            
            // lower right corner
            translation = Vec2f( (width  / 2.0f - cornerRadius),
                                -(height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // lower left corner
            translation = Vec2f(-(width  / 2.0f - cornerRadius),
                                -(height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // upper left corner
            translation = Vec2f(-(width  / 2.0f - cornerRadius),
                                (height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // upper right corner
            translation = Vec2f( (width  / 2.0f - cornerRadius),
                                (height / 2.0f - cornerRadius));
            for (size_t i = 0; i < cornerSegments; ++i) {
                vertices[vertexIndex++] = center;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices[vertexIndex++] = translation + Vec2f(x, y);
            }
            
            // upper body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f(-(width / 2.0f - cornerRadius), height / 2.0f);
            vertices[vertexIndex++] = Vec2f( (width / 2.0f - cornerRadius), height / 2.0f);
            
            // right body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f(width / 2.0f,  (height / 2.0f - cornerRadius));
            vertices[vertexIndex++] = Vec2f(width / 2.0f, -(height / 2.0f - cornerRadius));
            
            // lower body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f( (width / 2.0f - cornerRadius), -height / 2.0f);
            vertices[vertexIndex++] = Vec2f(-(width / 2.0f - cornerRadius), -height / 2.0f);
            
            // left body triangle
            vertices[vertexIndex++] = center;
            vertices[vertexIndex++] = Vec2f(-width / 2.0f, -(height / 2.0f - cornerRadius));
            vertices[vertexIndex++] = Vec2f(-width / 2.0f,  (height / 2.0f - cornerRadius));
            
            return vertices;
        }
    }
}
