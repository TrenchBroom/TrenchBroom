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
#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayMap::Size::Size() :
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
            return m_sizes[texture];
        }

        void TexturedIndexArrayMap::Size::initialize(TexturedIndexArrayMap& map) const {
            size_t baseOffset = 0;

            for (const auto& [texture, size] : m_sizes) {
                map.m_ranges[texture] = IndexArrayMap(size, baseOffset);
                baseOffset += size.indexCount();
            }
        }

        TexturedIndexArrayMap::TexturedIndexArrayMap() {}
        
        TexturedIndexArrayMap::TexturedIndexArrayMap(const Size& size) {
            size.initialize(*this);
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
            for (const auto& [texture, indexRange] : m_ranges) {
                func.before(texture);
                indexRange.render(vertexArray);
                func.after(texture);
            }
        }

        IndexArrayMap& TexturedIndexArrayMap::findCurrent(const Texture* texture) {
            return m_ranges[texture];
        }
    }
}
