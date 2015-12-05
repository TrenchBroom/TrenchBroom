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

#ifndef IndexRangeRenderer_h
#define IndexRangeRenderer_h

#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class IndexRangeRenderer {
        private:
            VertexArray m_vertexArray;
            IndexRangeMap m_indexArray;
        public:
            IndexRangeRenderer();
            template <typename VertexSpec>
            IndexRangeRenderer(IndexRangeMapBuilder<VertexSpec>& builder) :
            m_vertexArray(VertexArray::swap(builder.vertices())),
            m_indexArray(builder.indexArray()) {}
            
            IndexRangeRenderer(const VertexArray& vertexArray, const IndexRangeMap& indexArray);

            void prepare(Vbo& vbo);
            void render();
        };
    }
}

#endif /* IndexRangeRenderer_h */
