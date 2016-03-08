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

#ifndef TexturedIndexRangeMap_h
#define TexturedIndexRangeMap_h

#include "SharedPointer.h"
#include "Renderer/IndexRangeMap.h"

#include <map>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class TextureRenderFunc;
        class VertexArray;
        
        class TexturedIndexRangeMap {
        public:
            typedef Assets::Texture Texture;
        private:
            typedef std::map<const Texture*, IndexRangeMap> TextureToIndexRangeMap;
            typedef std::tr1::shared_ptr<TextureToIndexRangeMap> TextureToIndexRangeMapPtr;
        public:
            class Size {
            private:
                friend class TexturedIndexRangeMap;
                
                typedef std::map<const Texture*, IndexRangeMap::Size> TextureToSize;
                TextureToSize m_sizes;
                TextureToSize::iterator m_current;
            public:
                Size();
                void inc(const Texture* texture, PrimType primType, size_t count = 1);
            private:
                IndexRangeMap::Size& findCurrent(const Texture* texture);
                bool isCurrent(const Texture* texture) const;

                void initialize(TextureToIndexRangeMap& data) const;
            };
        private:
            TextureToIndexRangeMapPtr m_data;
            TextureToIndexRangeMap::iterator m_current;
        public:
            TexturedIndexRangeMap();
            TexturedIndexRangeMap(const Size& size);
            TexturedIndexRangeMap(const Texture* texture, const IndexRangeMap& primitives);
            TexturedIndexRangeMap(const Texture* texture, PrimType primType, size_t index, size_t count);

            void add(const Texture* texture, PrimType primType, size_t index, size_t count);
            
            void render(VertexArray& vertexArray);
            void render(VertexArray& vertexArray, TextureRenderFunc& func);
        private:
            IndexRangeMap& findCurrent(const Texture* texture);
            bool isCurrent(const Texture* texture) const;
        };
    }
}

#endif /* TexturedIndexRangeMap_h */
