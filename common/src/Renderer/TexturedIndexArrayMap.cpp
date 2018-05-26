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

#include "Renderer/IndexArray.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayMap::Size::Size() :
                m_indexCount(0) {}
        
        size_t TexturedIndexArrayMap::Size::indexCount() const {
            return m_indexCount;
        }

        void TexturedIndexArrayMap::Size::incTriangles(const Texture* texture, const size_t count) {
            m_triangles[texture] += count;
            m_indexCount += count;
        }

        void TexturedIndexArrayMap::Size::initialize(TexturedIndexArrayMap& map) const {
            size_t baseOffset = 0;

            for (const auto& [texture, size] : m_triangles) {
                map.m_ranges[texture] = IndexArrayRange(baseOffset, size);
                baseOffset += size;
            }
        }

        TexturedIndexArrayMap::TexturedIndexArrayMap() {}
        
        TexturedIndexArrayMap::TexturedIndexArrayMap(const Size& size) {
            size.initialize(*this);
        }

        size_t TexturedIndexArrayMap::addTriangles(const Texture* texture, const size_t count) {
            IndexArrayRange& current = m_ranges[texture];
            return current.add(count);
        }

        void TexturedIndexArrayMap::render(std::shared_ptr<IndexHolder> indexArray) {
            DefaultTextureRenderFunc func;
            render(indexArray, func);
        }
        
        void TexturedIndexArrayMap::render(std::shared_ptr<IndexHolder> indexArray, TextureRenderFunc& func) {
            for (const auto& [texture, indexRange] : m_ranges) {
                func.before(texture);
                indexArray->render(GL_TRIANGLES, indexRange.offset, indexRange.count);
                func.after(texture);
            }
        }

        const std::unordered_map<const Assets::Texture*, IndexArrayRange, TexturedIndexArrayMap::HashPtr>& TexturedIndexArrayMap::ranges() const {
            return m_ranges;
        }
    }
}
