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


#ifndef TrenchBroom_RenderUtils_h
#define TrenchBroom_RenderUtils_h

#include <GL/glew.h>
#include "Model/Texture.h"
#include "Utility/Color.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        static const float EdgeOffset = 0.0001f;
        
        inline void glVertexV3f(const Vec3f& vertex) {
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        
        inline void glColorV4f(const Color& color) {
            glColor4f(color.x, color.y, color.z, color.w);
        }
        
        inline void glColorV4f(const Color& color, float blendFactor) {
            glColor4f(color.x, color.y, color.z, color.w * blendFactor);
        }
        
        inline void glSetEdgeOffset(float f) {
            glDepthRange(0.0f, 1.0f - EdgeOffset * f);
        }
        
        inline void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0f);
        }
        
        inline void roundedRect(float width, float height, float cornerRadius, unsigned int cornerSegments, Vec2f::List& vertices) {
            assert(cornerSegments > 0);
            assert(cornerRadius <= width / 2.0f &&
                   cornerRadius <= height / 2.0f);
            
            const float angle = Math::Pi / 2.0f / cornerSegments;
            Vec2f center(0.0f, 0.0f);
            Vec2f translation;

            float curAngle = 0.0f;
            float x = std::cos(curAngle) * cornerRadius;
            float y = std::sin(curAngle) * cornerRadius;

            // lower right corner
            translation.x =  (width  / 2.0f - cornerRadius);
            translation.y = -(height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));

                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }

            // lower left corner
            translation.x = -(width  / 2.0f - cornerRadius);
            translation.y = -(height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }
            
            // upper left corner
            translation.x = -(width  / 2.0f - cornerRadius);
            translation.y =  (height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }

            // upper right corner
            translation.x =  (width  / 2.0f - cornerRadius);
            translation.y =  (height / 2.0f - cornerRadius);
            for (unsigned int i = 0; i < cornerSegments; i++) {
                vertices.push_back(center);
                vertices.push_back(translation + Vec2f(x, y));
                
                curAngle -= angle;
                x = std::cos(curAngle) * cornerRadius;
                y = std::sin(curAngle) * cornerRadius;
                vertices.push_back(translation + Vec2f(x, y));
            }
            
            // upper body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f(-(width / 2.0f - cornerRadius), height / 2.0f));
            vertices.push_back(Vec2f( (width / 2.0f - cornerRadius), height / 2.0f));
            
            // right body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f(width / 2.0f,  (height / 2.0f - cornerRadius)));
            vertices.push_back(Vec2f(width / 2.0f, -(height / 2.0f - cornerRadius)));

            // lower body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f( (width / 2.0f - cornerRadius), -height / 2.0f));
            vertices.push_back(Vec2f(-(width / 2.0f - cornerRadius), -height / 2.0f));

            // left body triangle
            vertices.push_back(center);
            vertices.push_back(Vec2f(-width / 2.0f, -(height / 2.0f - cornerRadius)));
            vertices.push_back(Vec2f(-width / 2.0f,  (height / 2.0f - cornerRadius)));
        }
    }
}

#endif
