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

#include "TexturedIndexArray.h"

#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArray::RenderFunc::~RenderFunc() {}
        void TexturedIndexArray::RenderFunc::before(const Texture* texture) {}
        void TexturedIndexArray::RenderFunc::after(const Texture* texture) {}

        void TexturedIndexArray::DefaultRenderFunc::before(const Texture* texture) {
            if (texture != NULL)
                texture->activate();
        }
        
        void TexturedIndexArray::DefaultRenderFunc::after(const Texture* texture) {
            if (texture != NULL)
                texture->deactivate();
        }

        TexturedIndexArray::Size::Size() :
        m_current(m_sizes.end()) {}
        
        void TexturedIndexArray::Size::inc(const Texture* texture, const PrimType primType, const size_t count) {
            IndexArray::Size& sizeForKey = findCurrent(texture);
            sizeForKey.inc(primType, count);
        }

        IndexArray::Size& TexturedIndexArray::Size::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = MapUtils::findOrInsert(m_sizes, texture, IndexArray::Size());
            return m_current->second;
        }
        
        bool TexturedIndexArray::Size::isCurrent(const Texture* texture) const {
            if (m_current == m_sizes.end())
                return false;
            
            typedef TextureToSize::key_compare Cmp;
            const Cmp& cmp = m_sizes.key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }

        void TexturedIndexArray::Size::initialize(TextureToIndexArray& data) const {
            TextureToSize::const_iterator texIt, texEnd;
            for (texIt = m_sizes.begin(), texEnd = m_sizes.end(); texIt != texEnd; ++texIt) {
                const Texture* texture = texIt->first;
                const IndexArray::Size& size = texIt->second;
                data.insert(std::make_pair(texture, IndexArray(size)));
            }
        }

        TexturedIndexArray::TexturedIndexArray() :
        m_data(new TextureToIndexArray()),
        m_current(m_data->end()) {}

        TexturedIndexArray::TexturedIndexArray(const Size& size) :
        m_data(new TextureToIndexArray()),
        m_current(m_data->end()) {
            size.initialize(*m_data);
        }
        
        TexturedIndexArray::TexturedIndexArray(const Texture* texture, const IndexArray& primitives) :
        m_data(new TextureToIndexArray()),
        m_current(m_data->end()) {
            m_data->insert(std::make_pair(texture, primitives));
        }

        TexturedIndexArray::TexturedIndexArray(const Texture* texture, const PrimType primType, const GLint index, const GLsizei count) :
        m_data(new TextureToIndexArray()),
        m_current(m_data->end()) {
            m_data->insert(std::make_pair(texture, IndexArray(primType, index, count)));
        }

        void TexturedIndexArray::add(const Texture* texture, const PrimType primType, const GLint index, const GLsizei count) {
            IndexArray& current = findCurrent(texture);
            current.add(primType, index, count);
        }

        void TexturedIndexArray::render(VertexArray& vertexArray) {
            DefaultRenderFunc func;
            render(vertexArray, func);
        }
        
        void TexturedIndexArray::render(VertexArray& vertexArray, RenderFunc& func) {
            typename TextureToIndexArray::const_iterator texIt, texEnd;
            for (texIt = m_data->begin(), texEnd = m_data->end(); texIt != texEnd; ++texIt) {
                const Texture* texture = texIt->first;
                const IndexArray& indexArray = texIt->second;

                func.before(texture);
                indexArray.render(vertexArray);
                func.after(texture);
            }
        }

        IndexArray& TexturedIndexArray::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = m_data->find(texture);
            assert(m_current != m_data->end());
            return m_current->second;
        }
        
        bool TexturedIndexArray::isCurrent(const Texture* texture) const {
            if (m_current == m_data->end())
                return false;
            
            typedef TextureToIndexArray::key_compare Cmp;
            const Cmp& cmp = m_data->key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }
    }
}
