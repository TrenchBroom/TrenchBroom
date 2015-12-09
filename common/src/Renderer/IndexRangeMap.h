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

#ifndef IndexRangeMap_h
#define IndexRangeMap_h

#include "SharedPointer.h"
#include "Renderer/GL.h"
#include "Renderer/VertexArray.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class IndexRangeMap {
        private:
            struct IndicesAndCounts {
                GLIndices indices;
                GLCounts counts;
                
                IndicesAndCounts();
                IndicesAndCounts(size_t index, size_t count);
                
                size_t size() const;
                void reserve(size_t capacity);
                void add(PrimType primType, size_t index, size_t count, bool dynamicGrowth);
            };
            
            typedef std::map<PrimType, IndicesAndCounts> PrimTypeToIndexData;
            typedef std::tr1::shared_ptr<PrimTypeToIndexData> PrimTypeToIndexDataPtr;
        public:
            class Size {
            private:
                friend class IndexRangeMap;
                
                typedef std::map<PrimType, size_t> PrimTypeToSize;
                PrimTypeToSize m_sizes;
            public:
                void inc(const PrimType primType, size_t count = 1);
            private:
                void initialize(PrimTypeToIndexData& data) const;
            };
        private:
            PrimTypeToIndexDataPtr m_data;
            bool m_dynamicGrowth;
        public:
            IndexRangeMap();
            IndexRangeMap(const Size& size);
            IndexRangeMap(PrimType primType, size_t index, size_t count);
            
            void add(PrimType primType, size_t index, size_t count);
            
            void render(VertexArray& vertexArray) const;
        };
    }
}

#endif /* IndexRangeMap_h */
