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

#ifndef TexturedIndexArrayMap_h
#define TexturedIndexArrayMap_h

#include "Renderer/IndexArrayMap.h"

#include <unordered_map>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class IndexArray;
        class TextureRenderFunc;
        
        class TexturedIndexArrayMap {
        public:
            using Texture = Assets::Texture;
        private:
            class HashPtr {
            public:
                size_t operator()(const Texture* texture) const {
                    return reinterpret_cast<size_t>(texture);
                }
            };
        public:
            class Size {
            private:
                friend class TexturedIndexArrayMap;
                
                std::unordered_map<const Texture*, IndexArrayMap::Size, HashPtr> m_sizes;

                size_t m_indexCount;
            public:
                Size();
                size_t indexCount() const;
                void inc(const Texture* texture, PrimType primType, size_t count);
            private:
                IndexArrayMap::Size& findCurrent(const Texture* texture);

                void initialize(TexturedIndexArrayMap& map) const;
            };
        private:
            std::unordered_map<const Texture*, IndexArrayMap, HashPtr> m_ranges;

        public:
            TexturedIndexArrayMap();
            TexturedIndexArrayMap(const Size& size);

            size_t add(const Texture* texture, PrimType primType, size_t count);

            void render(IndexArray& vertexArray);
            void render(IndexArray& vertexArray, TextureRenderFunc& func);
        private:
            IndexArrayMap& findCurrent(const Texture* texture);
        };
    }
}

#endif /* TexturedIndexArrayMap_h */
