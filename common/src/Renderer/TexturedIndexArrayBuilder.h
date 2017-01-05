/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TexturedIndexArrayBuilder_h
#define TexturedIndexArrayBuilder_h

#include "Renderer/GL.h"
#include "Renderer/TexturedIndexArrayMap.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class TexturedIndexArrayBuilder {
        public:
            typedef Assets::Texture Texture;
            typedef GLuint Index;
            typedef std::vector<Index> IndexArray;
        private:
            IndexArray m_indices;
            TexturedIndexArrayMap m_ranges;
        public:
            TexturedIndexArrayBuilder(const TexturedIndexArrayMap::Size& size);
            
            const IndexArray& indices() const;
            IndexArray& indices();
            
            const TexturedIndexArrayMap& ranges() const;
            
            void addPoint(const Texture* texture, Index i);
            void addPoints(const Texture* texture, const IndexArray& indices);
            
            void addLine(const Texture* texture, Index i1, Index i2);
            void addLines(const Texture* texture, const IndexArray& indices);
            
            void addTriangle(const Texture* texture, Index i1, Index i2, Index i3);
            void addTriangles(const Texture* texture, const IndexArray& indices);
            
            void addQuad(const Texture* texture, Index, Index i1, Index i2, Index i3, Index i4);
            void addQuads(const Texture* texture, const IndexArray& indices);
            void addQuads(const Texture* texture, Index baseIndex, size_t vertexCount);
            
            void addPolygon(const Texture* texture, const IndexArray& indices);
            void addPolygon(const Texture* texture, Index baseIndex, size_t vertexCount);
        private:
            void add(const Texture* texture, PrimType primType, const IndexArray& indices);
        };
    }
}

#endif /* TexturedIndexArrayBuilder_h */
