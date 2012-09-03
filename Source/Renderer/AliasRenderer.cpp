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

#include "Model/Alias.h"
#include "Model/Entity.h"
#include "Model/Palette.h"
#include "Model/Texture.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "Utility/GLee.h"

namespace TrenchBroom {
    namespace Renderer {
        AliasRenderer::AliasRenderer(const Model::Alias& alias, unsigned int skinIndex, Vbo& vbo, const Model::Palette& palette) :
        m_alias(alias),
        m_skinIndex(skinIndex),
        m_vbo(vbo),
        m_palette(palette),
        m_vboBlock(NULL),
        m_texture(NULL) {}

        AliasRenderer::~AliasRenderer() {
            if (m_vboBlock != NULL) {
                m_vboBlock->freeBlock();
                m_vboBlock = NULL;
            }
            
            if (m_texture != NULL) {
                delete m_texture;
                m_texture = NULL;
            }
        }

        void AliasRenderer::render() {
            if (m_vboBlock == NULL) {
                Model::AliasSkin& skin = *m_alias.skins()[m_skinIndex];
                m_texture = new Model::Texture(m_alias.name(), skin, 0, m_palette);

                Model::AliasSingleFrame& frame = m_alias.firstFrame();
                const Model::AliasFrameTriangleList& triangles = frame.triangles();
                m_triangleCount = static_cast<unsigned int>(triangles.size());
                unsigned int vertexSize = 3 * 8;

                m_vboBlock = m_vbo.allocBlock(m_triangleCount * vertexSize * sizeof(float));
                m_vbo.map();
                int offset = 0;
                for (unsigned int i = 0; i < m_triangleCount; i++) {
                    Model::AliasFrameTriangle& triangle = *triangles[i];
                    for (unsigned int j = 0; j < 3; j++) {
                        Model::AliasFrameVertex& vertex = triangle[j];
                        // GL_T2F_N3F_V3F format
                        offset = m_vboBlock->writeVec(vertex.texCoords(), offset);
                        offset = m_vboBlock->writeVec(vertex.normal(), offset);
                        offset = m_vboBlock->writeVec(vertex.position(), offset);
                    }
                }
                m_vbo.unmap();
            }

            m_texture->activate();
            
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glInterleavedArrays(GL_T2F_N3F_V3F, 0, reinterpret_cast<const GLvoid *>(m_vboBlock->address()));
            glDrawArrays(GL_TRIANGLES, 0, m_triangleCount * 3);
            glPopClientAttrib();

            m_texture->deactivate();
        }

        const Vec3f& AliasRenderer::center() const {
            return m_alias.firstFrame().center();
        }

        const BBox& AliasRenderer::bounds() const {
            return m_alias.firstFrame().bounds();
        }
    }
}
