/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "TexturedIndexArrayRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayRenderer::TexturedIndexArrayRenderer() {}

        TexturedIndexArrayRenderer::TexturedIndexArrayRenderer(const VertexArray& vertexArray, const TexturedIndexArray& indexArray) :
        m_vertexArray(vertexArray),
        m_indexArray(indexArray) {}

        TexturedIndexArrayRenderer::TexturedIndexArrayRenderer(const VertexArray& vertexArray, const Assets::Texture* texture, const IndexArray& indexArray) :
        m_vertexArray(vertexArray),
        m_indexArray(texture, indexArray) {}

        bool TexturedIndexArrayRenderer::empty() const {
            return m_vertexArray.empty();
        }

        void TexturedIndexArrayRenderer::prepare(Vbo& vbo) {
            m_vertexArray.prepare(vbo);
        }
        
        void TexturedIndexArrayRenderer::render() {
            if (m_vertexArray.setup()) {
                m_indexArray.render(m_vertexArray);
                m_vertexArray.cleanup();
            }
        }

        void TexturedIndexArrayRenderer::render(TexturedIndexArray::RenderFunc& func) {
            if (m_vertexArray.setup()) {
                m_indexArray.render(m_vertexArray, func);
                m_vertexArray.cleanup();
            }
        }
    }
}
