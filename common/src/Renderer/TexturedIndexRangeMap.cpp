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

#include "TexturedIndexRangeMap.h"

#include "Renderer/RenderUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexRangeMap::Size::Size() :
        m_current(std::end(m_sizes)) {}

        void TexturedIndexRangeMap::Size::inc(const Texture* texture, const PrimType primType, const size_t vertexCount) {
            auto& sizeForKey = findCurrent(texture);
            sizeForKey.inc(primType, vertexCount);
        }

        void TexturedIndexRangeMap::Size::inc(const TexturedIndexRangeMap::Size& other) {
            for (const auto& [texture, indexRange] : other.m_sizes) {
                auto& sizeForKey = findCurrent(texture);
                sizeForKey.inc(indexRange);
            }
        }

        IndexRangeMap::Size& TexturedIndexRangeMap::Size::findCurrent(const Texture* texture) {
            if (!isCurrent(texture)) {
                const auto result = m_sizes.try_emplace(texture);
                m_current = result.first;
            }
            return m_current->second;
        }

        bool TexturedIndexRangeMap::Size::isCurrent(const Texture* texture) const {
            if (m_current == std::end(m_sizes)) {
                return false;
            }

            const auto& cmp = m_sizes.key_comp();
            const auto* currentTexture = m_current->first;
            return !cmp(texture, currentTexture) && !cmp(currentTexture, texture);
        }

        void TexturedIndexRangeMap::Size::initialize(TextureToIndexRangeMap& data) const {
            for (const auto& [texture, size] : m_sizes) {
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

        TexturedIndexRangeMap::TexturedIndexRangeMap(const Texture* texture, IndexRangeMap primitives) :
        m_data(new TextureToIndexRangeMap()),
        m_current(m_data->end()) {
            add(texture, std::move(primitives));
        }

        TexturedIndexRangeMap::TexturedIndexRangeMap(const Texture* texture, const PrimType primType, const size_t index, const size_t vertexCount) :
        m_data(new TextureToIndexRangeMap()),
        m_current(m_data->end()) {
            m_data->insert(std::make_pair(texture, IndexRangeMap(primType, index, vertexCount)));
        }

        void TexturedIndexRangeMap::add(const Texture* texture, const PrimType primType, const size_t index, const size_t vertexCount) {
            auto& current = findCurrent(texture);
            current.add(primType, index, vertexCount);
        }

        void TexturedIndexRangeMap::add(const Texture* texture, IndexRangeMap primitives) {
            m_data->insert(std::make_pair(texture, std::move(primitives)));
        }

        void TexturedIndexRangeMap::add(const TexturedIndexRangeMap& other) {
            for (const auto& [texture, indexRangeMap] : *other.m_data) {
                auto& current = findCurrent(texture);
                current.add(indexRangeMap);
            }
        }

        void TexturedIndexRangeMap::render(VertexArray& vertexArray) {
            DefaultTextureRenderFunc func;
            render(vertexArray, func);
        }

        void TexturedIndexRangeMap::render(VertexArray& vertexArray, TextureRenderFunc& func) {
            for (const auto& [texture, indexArray] : *m_data) {
                func.before(texture);
                indexArray.render(vertexArray);
                func.after(texture);
            }
        }

        void TexturedIndexRangeMap::forEachPrimitive(std::function<void(const Texture*, PrimType, size_t, size_t)> func) const {
            for (const auto& entry : *m_data) {
                const auto* texture = entry.first;
                const auto& indexArray = entry.second;

                indexArray.forEachPrimitive([&func, &texture](const PrimType primType, const size_t index, const size_t count) {
                    func(texture, primType, index, count);
                });
            }
        }

        IndexRangeMap& TexturedIndexRangeMap::findCurrent(const Texture* texture) {
            if (!isCurrent(texture)) {
                m_current = m_data->find(texture);
            }
            assert(m_current != m_data->end());
            return m_current->second;
        }

        bool TexturedIndexRangeMap::isCurrent(const Texture* texture) const {
            if (m_current == m_data->end()) {
                return false;
            }

            const auto& cmp = m_data->key_comp();
            const auto* currentTexture = m_current->first;
            return !cmp(texture, currentTexture) && !cmp(currentTexture, texture);
        }
    }
}
