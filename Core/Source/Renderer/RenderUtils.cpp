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

        std::vector<Vec3f> bboxEdgeVertices(const BBox& bounds) {
            std::vector<Vec3f> vertices;
            vertices.resize(24);
            
            vertices[ 0] = vertices[ 7] = vertices[16] = bounds.vertex(false, false, false);
            vertices[ 1] = vertices[ 2] = vertices[20] = bounds.vertex(true , false, false);
            vertices[ 3] = vertices[ 4] = vertices[22] = bounds.vertex(true , true , false);
            vertices[ 5] = vertices[ 6] = vertices[18] = bounds.vertex(false, true , false);
            vertices[ 8] = vertices[15] = vertices[17] = bounds.vertex(false, false, true );
            vertices[ 9] = vertices[10] = vertices[21] = bounds.vertex(true , false, true );
            vertices[11] = vertices[12] = vertices[23] = bounds.vertex(true , true , true );
            vertices[13] = vertices[14] = vertices[19] = bounds.vertex(false, true , true );
            
            return vertices;
        }

        std::vector<Vec3f> bboxTriangleVertices(const BBox& bounds) {
            std::vector<Vec3f> vertices;
            vertices.resize(6 * 6);

            // bottom
            vertices[ 0] = vertices[ 3] = vertices[12] = vertices[15] = vertices[24] = vertices[27] = bounds.vertex(false, false, false);
            vertices[ 1] = vertices[17] = vertices[32] = vertices[34] = bounds.vertex(true , false, false);
            vertices[ 2] = vertices[ 4] = vertices[23] = vertices[31] = bounds.vertex(true , true , false);
            vertices[ 5] = vertices[20] = vertices[22] = vertices[25] = bounds.vertex(false, true , false);
            vertices[ 6] = vertices[ 9] = vertices[13] = vertices[29] = bounds.vertex(false, false, true );
            vertices[ 7] = vertices[19] = vertices[26] = vertices[28] = bounds.vertex(false, true , true );
            vertices[ 8] = vertices[10] = vertices[18] = vertices[21] = vertices[30] = vertices[33] = bounds.vertex(true , true , true );
            vertices[11] = vertices[14] = vertices[16] = vertices[35] = bounds.vertex(true , false, true );

            return vertices;
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
        
        void glSetBrightness(float brightness, bool modulateAlpha) {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            float color[4] = {brightness / 2.0f, brightness / 2.0f, brightness / 2.0f, 1.0f};
            
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, modulateAlpha ? GL_MODULATE : GL_REPLACE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
            glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
        }

    }
}
