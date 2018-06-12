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

#include "TexturedIndexArrayMap.h"

#include "CollectionUtils.h"
#include "Reference.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayMap::Size::Size() :
        m_current(std::end(m_sizes)),
        m_indexCount(0) {}
        
        size_t TexturedIndexArrayMap::Size::indexCount() const {
            return m_indexCount;
        }

        void TexturedIndexArrayMap::Size::inc(const Texture* texture, const PrimType primType, const size_t count) {
            IndexArrayMap::Size& sizeForKey = findCurrent(texture);
            sizeForKey.inc(primType, count);
            m_indexCount += count;
        }

        IndexArrayMap::Size& TexturedIndexArrayMap::Size::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = MapUtils::findOrInsert(m_sizes, texture, IndexArrayMap::Size());
            return m_current->second;
        }
        
        bool TexturedIndexArrayMap::Size::isCurrent(const Texture* texture) const {
            if (m_current == std::end(m_sizes))
                return false;
            
            typedef TextureToSize::key_compare Cmp;
            const Cmp& cmp = m_sizes.key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }
        
        void TexturedIndexArrayMap::Size::initialize(TextureToIndexArrayMap& ranges) const {
            size_t baseOffset = 0;
            
            for (const auto& entry : m_sizes) {
                const Texture* texture = entry.first;
                const IndexArrayMap::Size& size = entry.second;
                ranges.insert(std::make_pair(texture, IndexArrayMap(size, baseOffset)));
                baseOffset += size.indexCount();
            }
        }

        TexturedIndexArrayMap::TexturedIndexArrayMap() :
        m_ranges(new TextureToIndexArrayMap()),
        m_current(m_ranges->end()) {}
        
        TexturedIndexArrayMap::TexturedIndexArrayMap(const Size& size) :
        m_ranges(new TextureToIndexArrayMap()),
        m_current(m_ranges->end()) {
            size.initialize(*m_ranges);
        }

        size_t TexturedIndexArrayMap::add(const Texture* texture, const PrimType primType, const size_t count) {
            IndexArrayMap& current = findCurrent(texture);
            return current.add(primType, count);
        }

        void TexturedIndexArrayMap::render(IndexArray& indexArray) {
            DefaultTextureRenderFunc func;
            render(indexArray, func);
        }
        
        void TexturedIndexArrayMap::render(IndexArray& vertexArray, TextureRenderFunc& func) {
            for (const auto& entry : *m_ranges) {
                const Texture* texture = entry.first;
                const IndexArrayMap& indexRange = entry.second;
                
                func.before(texture);
                indexRange.render(vertexArray);
                func.after(texture);
            }
        }

        IndexArrayMap& TexturedIndexArrayMap::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = m_ranges->find(texture);
            assert(m_current != m_ranges->end());
            return m_current->second;
        }
        
        bool TexturedIndexArrayMap::isCurrent(const Texture* texture) {
            if (m_current == m_ranges->end())
                return false;
            
            typedef TextureToIndexArrayMap::key_compare Cmp;
            const Cmp& cmp = m_ranges->key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }
    }
}
