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

#ifndef TexturedIndexArray_h
#define TexturedIndexArray_h

#include "SharedPointer.h"
#include "Renderer/IndexArray.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class VertexArray;
        
        class TexturedIndexArray {
        private:
            typedef Assets::Texture Texture;
            typedef std::map<const Texture*, IndexArray> TextureToIndexArray;
            typedef std::tr1::shared_ptr<TextureToIndexArray> TextureToIndexArrayPtr;
        public:
            class Size {
            private:
                friend class TexturedIndexArray;
                
                typedef std::map<const Texture*, IndexArray::Size> TextureToSize;
                TextureToSize m_sizes;
                TextureToSize::iterator m_current;
            public:
                Size();
                void inc(const Texture* texture, IndexArray::PrimType primType, size_t count = 1);
                IndexArray::Size& findCurrent(const Texture* texture);
            private:
                bool isCurrent(const Texture* texture) const;
            private:
                void initialize(TextureToIndexArray& data) const;
            };
        private:
            TextureToIndexArrayPtr m_data;
            TextureToIndexArray::iterator m_current;
        public:
            TexturedIndexArray(const Size& size);
            TexturedIndexArray(const Texture* texture, const IndexArray& primitives);
            TexturedIndexArray(const Texture* texture, IndexArray::PrimType primType, GLint index, GLsizei count);
            void add(const Texture* texture, IndexArray::PrimType primType, GLint index, GLsizei count);
            void render(const VertexArray& vertexArray);
        private:
            IndexArray& findCurrent(const Texture* texture);
            bool isCurrent(const Texture* texture) const;
        };
    }
}

#endif /* TexturedIndexArray_h */
