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

#include "TexturedIndexArrayMap.h"

#include "CollectionUtils.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayMap::Size::Size() :
        m_current(m_sizes.end()) {}
        
        void TexturedIndexArrayMap::Size::inc(const Texture* texture, const PrimType primType, const size_t count) {
            IndexArrayMap::Size& sizeForKey = findCurrent(texture);
            sizeForKey.inc(primType, count);
        }

        IndexArrayMap::Size& TexturedIndexArrayMap::Size::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = MapUtils::findOrInsert(m_sizes, texture, IndexArrayMap::Size());
            return m_current->second;
        }
        
        bool TexturedIndexArrayMap::Size::isCurrent(const Texture* texture) const {
            if (m_current == m_sizes.end())
                return false;
            
            typedef TextureToSize::key_compare Cmp;
            const Cmp& cmp = m_sizes.key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }
        
        void TexturedIndexArrayMap::Size::initialize(TextureToIndexArrayMap& data) const {
            TextureToSize::const_iterator texIt, texEnd;
            for (texIt = m_sizes.begin(), texEnd = m_sizes.end(); texIt != texEnd; ++texIt) {
                const Texture* texture = texIt->first;
                const IndexArrayMap::Size& size = texIt->second;
                data.insert(std::make_pair(texture, IndexArrayMap(size)));
            }
        }

        TexturedIndexArrayMap::TexturedIndexArrayMap() :
        m_data(new TexturedIndexArrayMap()),
        m_current(m_data->end()) {}
        
        TexturedIndexArrayMap::TexturedIndexArrayMap(const Size& size) :
        m_data(new TexturedIndexArrayMap()),
        m_current(m_data->end()) {
            size.initialize(*m_data);
        }

        void TexturedIndexArrayMap::add(const Texture* texture, const PrimType primType, const size_t index) {
            IndexArrayMap& current = findCurrent(texture);
            current.add(primType, index);
        }
        
        void TexturedIndexArrayMap::addPolygon(const Texture* texture, const PrimType primType, const size_t index, const size_t count) {
            IndexArrayMap& current = findCurrent(texture);
            current.addPolygon(primType, index, count);
        }

        size_t TexturedIndexArrayMap::countIndices() const {
            size_t result = 0;
            TextureToIndexArrayMap::const_iterator it, end;
            for (it = m_data->begin(), end = m_data->end(); it != end; ++it) {
                const IndexArrayMap& indexArrayMap = it->second;
                result += indexArrayMap.countIndices();
            }
            return result;
        }

        void TexturedIndexArrayMap::getIndices(IndexList& allIndices, TextureToRangeMap& ranges) const {
            
            TextureToIndexArrayMap::const_iterator it, end;
            for (it = m_data->begin(), end = m_data->end(); it != end; ++it) {
                const Texture* texture = it->first;
                const IndexArrayMap& indexArrayMap = it->second;
                
                indexArrayMap.getIndices(allIndices, ranges[texture]);
            }
        }

        IndexArrayMap& TexturedIndexArrayMap::findCurrent(const Texture* texture) {
            if (!isCurrent(texture))
                m_current = m_data->find(texture);
            assert(m_current != m_data->end());
            return m_current->second;
        }
        
        bool TexturedIndexArrayMap::isCurrent(const Texture* texture) {
            if (m_current == m_data->end())
                return false;
            
            typedef TextureToIndexArrayMap::key_compare Cmp;
            const Cmp& cmp = m_data->key_comp();
            
            const Texture* currentTexture = m_current->first;
            if (cmp(texture, currentTexture) || cmp(currentTexture, texture))
                return false;
            return true;
        }
    }
}
