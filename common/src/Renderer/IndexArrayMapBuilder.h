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

#ifndef IndexArrayMapBuilder_h
#define IndexArrayMapBuilder_h

#include "Renderer/GL.h"
#include "Renderer/IndexArrayMap.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class IndexArrayMapBuilder {
        public:
            typedef GLuint Index;
            typedef std::vector<Index> IndexList;
        private:
            IndexList m_indices;
            IndexArrayMap m_ranges;
        public:
            IndexArrayMapBuilder(const IndexArrayMap::Size& size);
            
            const IndexList& indices() const;
            IndexList& indices();
            
            const IndexArrayMap& ranges() const;

            void addPoint(Index i);
            void addPoints(const IndexList& indices);
            
            void addLine(Index i1, Index i2);
            void addLines(const IndexList& indices);
            
            void addTriangle(Index i1, Index i2, Index i3);
            void addTriangles(const IndexList& indices);
            
            void addQuad(Index, Index i1, Index i2, Index i3, Index i4);
            void addQuads(const IndexList& indices);
            void addQuads(Index baseIndex, size_t vertexCount);
            
            void addPolygon(const IndexList& indices);
            void addPolygon(Index baseIndex, size_t vertexCount);
        private:
            void add(PrimType primType, const IndexList& indices);
        };
    }
}

#endif /* IndexArrayMapBuilder_h */
