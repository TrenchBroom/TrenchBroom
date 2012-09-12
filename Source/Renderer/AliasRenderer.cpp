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
#include "Renderer/Shader/Shader.h"
#include "Renderer/Vbo.h"
#include "Utility/GLee.h"

namespace TrenchBroom {
    namespace Renderer {
        AliasRenderer::AliasRenderer(const Model::Alias& alias, unsigned int skinIndex, Vbo& vbo, const Model::Palette& palette) :
        m_alias(alias),
        m_skinIndex(skinIndex),
        m_vbo(vbo),
        m_palette(palette),
        m_texture(NULL) {}

        void AliasRenderer::render(ShaderProgram& shaderProgram) {
            if (m_vertexArray.get() == NULL) {
                Model::AliasSkin& skin = *m_alias.skins()[m_skinIndex];
                m_texture = Model::TexturePtr(new Model::Texture(m_alias.name(), skin, 0, m_palette));

                Model::AliasSingleFrame& frame = m_alias.firstFrame();
                const Model::AliasFrameTriangleList& triangles = frame.triangles();
                unsigned int vertexCount = static_cast<unsigned int>(3 * triangles.size());
                
                m_vertexArray = VertexArrayPtr(new VertexArray(m_vbo,GL_TRIANGLES, vertexCount,
                                                               VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                                               VertexAttribute(2, GL_FLOAT, VertexAttribute::TexCoord0)));

                m_vbo.map();
                for (unsigned int i = 0; i < triangles.size(); i++) {
                    Model::AliasFrameTriangle& triangle = *triangles[i];
                    for (unsigned int j = 0; j < 3; j++) {
                        Model::AliasFrameVertex& vertex = triangle[j];
                        m_vertexArray->addAttribute(vertex.position());
                        m_vertexArray->addAttribute(vertex.texCoords());
                    }
                }
                m_vbo.unmap();
            }

            glActiveTexture(GL_TEXTURE0);
            m_texture->activate();
            shaderProgram.setUniformVariable("Texture", 0);
            m_vertexArray->render();
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
