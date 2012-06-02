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
#include "GL/GLee.h"

namespace TrenchBroom {
    namespace Renderer {
        float const EdgeOffset = 0.0001f;

        void bboxEdgeVertices(const BBox& bounds, std::vector<Vec3f>& vertices) {
            if (vertices.size() != 24)
                vertices.resize(24);
            
            // bottom
            vertices[ 0] = vertices[ 7] = bounds.vertex(false, false, false);
            vertices[ 1] = vertices[ 2] = bounds.vertex(true , false, false);
            vertices[ 3] = vertices[ 4] = bounds.vertex(true , true , false);
            vertices[ 5] = vertices[ 6] = bounds.vertex(false, true , false);
            
            // top
            vertices[ 8] = vertices[15] = bounds.vertex(false, false, true );
            vertices[ 9] = vertices[10] = bounds.vertex(true , false, true );
            vertices[11] = vertices[12] = bounds.vertex(true , true , true );
            vertices[13] = vertices[14] = bounds.vertex(false, true , true );
            
            // verticals
            vertices[16] = bounds.vertex(false, false, false);
            vertices[17] = bounds.vertex(false, false, true );
            vertices[18] = bounds.vertex(false, true , false );
            vertices[19] = bounds.vertex(false, true , true );
            vertices[20] = bounds.vertex(true , false, false);
            vertices[21] = bounds.vertex(true , false, true );
            vertices[22] = bounds.vertex(true , true , false);
            vertices[23] = bounds.vertex(true , true , true );
        }

        void glVertexV3f(const Vec3f& vertex) {
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }

        void glColorV4f(const Vec4f& color) {
            glColor4f(color.x, color.y, color.z, color.w);
        }

        void glColorV4f(const Vec4f& color, float blendFactor) {
            glColor4f(color.x, color.y, color.z, color.w * blendFactor);
        }

        void glSetEdgeOffset(float f) {
            glDepthRange(0.0, 1.0 - EdgeOffset * f);
        }

        void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0);
        }
    }
}
