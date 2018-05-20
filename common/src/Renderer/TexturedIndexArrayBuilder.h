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
            typedef std::vector<Index> IndexList;
        private:
            IndexList m_indices;
            TexturedIndexArrayMap m_ranges;
        public:
            TexturedIndexArrayBuilder(const TexturedIndexArrayMap::Size& size);
            
            const IndexList& indices() const;
            IndexList& indices();
            
            const TexturedIndexArrayMap& ranges() const;

            void addPolygon(const Texture* texture, Index baseIndex, size_t vertexCount);
        private:
            Index* addTriangles(const Texture* texture, size_t indexCount);
        };
    }
}

#endif /* TexturedIndexArrayBuilder_h */
