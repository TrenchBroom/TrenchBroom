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

#ifndef __TrenchBroom__CompassRenderer__
#define __TrenchBroom__CompassRenderer__

#include "Utility/Color.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class IndexedVertexArray;
        class RenderContext;
        class ShaderProgram;
        class Vbo;
        class VertexArray;
        
        class CompassRenderer {
        private:
            static const unsigned int m_segments = 32;
            static const float m_shaftLength;
            static const float m_shaftRadius;
            static const float m_headLength;
            static const float m_headRadius;
            
            Vec3f::List m_shaftVertices;
            Vec3f::List m_shaftNormals;
            Vec3f::List m_shaftCapVertices;
            Vec3f::List m_shaftCapNormals;
            Vec3f::List m_headVertices;
            Vec3f::List m_headNormals;
            Vec3f::List m_headCapVertices;
            Vec3f::List m_headCapNormals;
            
            const Mat4f cameraRotationMatrix(const Camera& camera) const;
            void renderAxis(Vbo& vbo, const Mat4f& rotation);
            void renderOutline(Vbo& vbo, const Camera& camera, const Mat4f& rotation, const Color& color);
        public:
            CompassRenderer();
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__CompassRenderer__) */
