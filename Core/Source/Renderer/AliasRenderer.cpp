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

#include "AliasRenderer.h"
#include "Model/Assets/Alias.h"
#include "Model/Assets/Palette.h"
#include "Model/Assets/Texture.h"
#include "Model/Map/Entity.h"
#include "Model/Preferences.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "GL/GLee.h"

namespace TrenchBroom {
    namespace Renderer {
        AliasRenderer::AliasRenderer(Model::Assets::Alias& alias, int skinIndex, Vbo& vbo, Model::Assets::Palette& palette) : m_alias(alias), m_skinIndex(skinIndex), m_vbo(vbo), m_palette(palette), m_vboBlock(NULL), m_texture(NULL) {}

        AliasRenderer::~AliasRenderer() {
            if (m_vboBlock != NULL)
                m_vboBlock->freeBlock();
            if (m_texture != NULL)
                delete m_texture;
        }

        void AliasRenderer::render(Model::Entity& entity) {
            render(entity.origin(), static_cast<float>(entity.angle()));
        }

        void AliasRenderer::render(const Vec3f& position, float angle, float scale) {
            if (m_vboBlock == NULL) {
                Model::Assets::AliasSkin& skin = *m_alias.skins[m_skinIndex];
                m_texture = new Model::Assets::Texture(m_alias.name, skin, 0, m_palette);

                Model::Assets::AliasSingleFrame& frame = m_alias.firstFrame();
                m_triangleCount = static_cast<int>(frame.triangles.size());
                int vertexSize = 3 * 8;

                m_vboBlock = &m_vbo.allocBlock(m_triangleCount * vertexSize * sizeof(float));
                m_vbo.map();
                int offset = 0;
                for (unsigned int i = 0; i < m_triangleCount; i++) {
                    Model::Assets::AliasFrameTriangle& triangle = *frame.triangles[i];
                    for (unsigned int j = 0; j < 3; j++) {
                        Model::Assets::AliasFrameVertex& vertex = triangle.vertices[j];
                        // GL_T2F_N3F_V3F format
                        offset = m_vboBlock->writeVec(vertex.texCoords, offset);
                        offset = m_vboBlock->writeVec(vertex.normal, offset);
                        offset = m_vboBlock->writeVec(vertex.position, offset);
                    }
                }
                m_vbo.unmap();
            }

            glTranslatef(position.x, position.y, position.z);

            if (scale != 1.0f)
                glScalef(scale, scale, scale);

            if (angle != 0.0f) {
                if (angle == -1.0f) glRotatef(90, 1, 0, 0);
                else if (angle == -2.0f) glRotatef(-90, 1, 0, 0);
                else glRotatef(-angle, 0, 0, 1);
            }

            m_texture->activate();
            
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glInterleavedArrays(GL_T2F_N3F_V3F, 0, (const GLvoid *)(long)m_vboBlock->address);
            glDrawArrays(GL_TRIANGLES, 0, m_triangleCount * 3);
            glPopClientAttrib();

            m_texture->deactivate();
        }

        const Vec3f& AliasRenderer::center() {
            return m_alias.firstFrame().center;
        }

        const BBox& AliasRenderer::bounds() {
            return m_alias.firstFrame().bounds;
        }

        const BBox& AliasRenderer::maxBounds() {
            return m_alias.firstFrame().maxBounds;
        }
    }
}
