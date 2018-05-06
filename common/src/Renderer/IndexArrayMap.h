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

#ifndef IndexArrayMap_h
#define IndexArrayMap_h

#include "Renderer/GL.h"

namespace TrenchBroom {
    namespace Renderer {
        class IndexArray;
        
        class IndexArrayMap {
        private:
            struct IndexArrayRange {
                size_t offset;
                size_t capacity;
                size_t count;

                IndexArrayRange();
                IndexArrayRange(size_t i_offset, size_t i_capacity);
                
                size_t add(size_t count);
            };
        public:
            class Size {
            private:
                friend class IndexArrayMap;

                size_t m_points;
                size_t m_lines;
                size_t m_triangles;
            public:
                Size();
                void inc(PrimType primType, size_t count);
                size_t indexCount() const;
            };
        private:
            IndexArrayRange m_pointsRange;
            IndexArrayRange m_linesRange;
            IndexArrayRange m_trianglesRange;

            void initialize(const Size& size, size_t baseOffset);
        public:
            IndexArrayMap();
            explicit IndexArrayMap(const Size& size);
            IndexArrayMap(const Size& size, size_t baseOffset);

            size_t add(PrimType primType, size_t count);

            void render(IndexArray& indexArray) const;
        };
    }
}

#endif /* IndexArrayMap_h */
