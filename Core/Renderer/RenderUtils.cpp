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


#include "RenderUtils.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

namespace TrenchBroom {
    namespace Renderer {
        float const EdgeOffset = 0.0001f;

        void glVertexV3f(const Vec3f& vertex) {
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        
        void glColorV4f(const Vec4f& color) {
            glColor4f(color.x, color.y, color.z, color.w);
        }
        void glSetEdgeOffset(float f) {
            glDepthRange(0.0, 1.0 - EdgeOffset * f);
        }
        
        void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0);
        }
    }
}