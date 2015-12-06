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

#include "TexturedIndexRangeRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexRangeRenderer::TexturedIndexRangeRenderer() {}

        TexturedIndexRangeRenderer::TexturedIndexRangeRenderer(const VertexArray& vertexArray, const TexturedIndexRangeMap& indexRange) :
        m_vertexArray(vertexArray),
        m_indexRange(indexRange) {}

        TexturedIndexRangeRenderer::TexturedIndexRangeRenderer(const VertexArray& vertexArray, const Assets::Texture* texture, const IndexRangeMap& indexRange) :
        m_vertexArray(vertexArray),
        m_indexRange(texture, indexRange) {}

        bool TexturedIndexRangeRenderer::empty() const {
            return m_vertexArray.empty();
        }

        void TexturedIndexRangeRenderer::prepare(Vbo& vbo) {
            m_vertexArray.prepare(vbo);
        }
        
        void TexturedIndexRangeRenderer::render() {
            if (m_vertexArray.setup()) {
                m_indexRange.render(m_vertexArray);
                m_vertexArray.cleanup();
            }
        }

        void TexturedIndexRangeRenderer::render(TextureRenderFunc& func) {
            if (m_vertexArray.setup()) {
                m_indexRange.render(m_vertexArray, func);
                m_vertexArray.cleanup();
            }
        }
    }
}
