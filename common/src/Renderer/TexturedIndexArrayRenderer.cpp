/*
 Copyright (C) 2010-2017 Kristian Duske

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

        TexturedIndexArrayRenderer::TexturedIndexArrayRenderer(VertexArray vertexArray, IndexArray indexArray, TexturedIndexArrayMap indexArrayMap) :
        m_vertexArray{std::move(vertexArray)},
        m_indexArray{std::move(indexArray)},
        m_indexRanges{std::move(indexArrayMap)} {}

        bool TexturedIndexArrayRenderer::empty() const {
            return m_indexArray.empty();
        }

        void TexturedIndexArrayRenderer::prepare(VboManager& vboManager) {
            m_vertexArray.prepare(vboManager);
            m_indexArray.prepare(vboManager);
        }

        void TexturedIndexArrayRenderer::render() {
            if (m_vertexArray.setup()) {
                if (m_indexArray.setup()) {
                    m_indexRanges.render(m_indexArray);
                    m_indexArray.cleanup();
                }
                m_vertexArray.cleanup();
            }
        }

        void TexturedIndexArrayRenderer::render(TextureRenderFunc& func) {
            if (m_vertexArray.setup()) {
                if (m_indexArray.setup()) {
                    m_indexRanges.render(m_indexArray, func);
                    m_indexArray.cleanup();
                }
                m_vertexArray.cleanup();
            }
        }
    }
}
