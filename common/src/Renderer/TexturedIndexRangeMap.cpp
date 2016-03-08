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

#include "TexturedIndexRangeMap.h"

#include "CollectionUtils.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexRangeMap::Size::Size() :
        m_current(m_sizes.end()) {}
        
        void TexturedIndexRangeMap::Size::inc(const Texture* texture, const PrimType primType, const size_t count) {
            IndexRangeMap::Size& sizeForKey = findCurrent(texture);
            sizeForKey.inc(primType, count);
        }

        IndexRangeMap::Size& TexturedIndexRangeMap::Size::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = MapUtils::findOrInsert(m_sizes, texture, IndexRangeMap::Size());
            return m_current->second;
        }
        
        bool TexturedIndexRangeMap::Size::isCurrent(const Texture* texture) const {
            if (m_current == m_sizes.end())
                return false;
            
            typedef TextureToSize::key_compare Cmp;
            const Cmp& cmp = m_sizes.key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }

        void TexturedIndexRangeMap::Size::initialize(TextureToIndexRangeMap& data) const {
            TextureToSize::const_iterator texIt, texEnd;
            for (texIt = m_sizes.begin(), texEnd = m_sizes.end(); texIt != texEnd; ++texIt) {
                const Texture* texture = texIt->first;
                const IndexRangeMap::Size& size = texIt->second;
                data.insert(std::make_pair(texture, IndexRangeMap(size)));
            }
        }

        TexturedIndexRangeMap::TexturedIndexRangeMap() :
        m_data(new TextureToIndexRangeMap()),
        m_current(m_data->end()) {}

        TexturedIndexRangeMap::TexturedIndexRangeMap(const Size& size) :
        m_data(new TextureToIndexRangeMap()),
        m_current(m_data->end()) {
            size.initialize(*m_data);
        }
        
        TexturedIndexRangeMap::TexturedIndexRangeMap(const Texture* texture, const IndexRangeMap& primitives) :
        m_data(new TextureToIndexRangeMap()),
        m_current(m_data->end()) {
            m_data->insert(std::make_pair(texture, primitives));
        }

        TexturedIndexRangeMap::TexturedIndexRangeMap(const Texture* texture, const PrimType primType, const size_t index, const size_t count) :
        m_data(new TextureToIndexRangeMap()),
        m_current(m_data->end()) {
            m_data->insert(std::make_pair(texture, IndexRangeMap(primType, index, count)));
        }

        void TexturedIndexRangeMap::add(const Texture* texture, const PrimType primType, const size_t index, const size_t count) {
            IndexRangeMap& current = findCurrent(texture);
            current.add(primType, index, count);
        }

        void TexturedIndexRangeMap::render(VertexArray& vertexArray) {
            DefaultTextureRenderFunc func;
            render(vertexArray, func);
        }
        
        void TexturedIndexRangeMap::render(VertexArray& vertexArray, TextureRenderFunc& func) {
            TextureToIndexRangeMap::const_iterator texIt, texEnd;
            for (texIt = m_data->begin(), texEnd = m_data->end(); texIt != texEnd; ++texIt) {
                const Texture* texture = texIt->first;
                const IndexRangeMap& indexArray = texIt->second;

                func.before(texture);
                indexArray.render(vertexArray);
                func.after(texture);
            }
        }

        IndexRangeMap& TexturedIndexRangeMap::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = m_data->find(texture);
            assert(m_current != m_data->end());
            return m_current->second;
        }
        
        bool TexturedIndexRangeMap::isCurrent(const Texture* texture) const {
            if (m_current == m_data->end())
                return false;
            
            typedef TextureToIndexRangeMap::key_compare Cmp;
            const Cmp& cmp = m_data->key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }
    }
}
