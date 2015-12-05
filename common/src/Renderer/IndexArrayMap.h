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

#ifndef IndexArrayMap_h
#define IndexArrayMap_h

#include "SharedPointer.h"
#include "Renderer/GL.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class IndexArrayMap {
        public:
            typedef GLuint Index;
            typedef std::vector<Index> IndexList;
            
            struct IndexArrayRange {
                size_t offset;
                size_t count;
                IndexArrayRange(size_t i_offset, size_t i_count);
            };
            
            typedef std::map<PrimType, IndexArrayRange> PrimTypeToRangeMap;
        private:
            typedef std::map<PrimType, IndexList> PrimTypeToIndexData;
            typedef std::tr1::shared_ptr<PrimTypeToIndexData> PrimTypeToIndexDataPtr;
        public:
            class Size {
            private:
                friend class IndexArrayMap;

                typedef std::map<PrimType, size_t> PrimTypeToSize;
                PrimTypeToSize m_sizes;
            public:
                void inc(const PrimType primType, size_t count);
            private:
                void initialize(PrimTypeToIndexData& data) const;
            };
        private:
            PrimTypeToIndexDataPtr m_data;
            bool m_dynamicGrowth;
        public:
            IndexArrayMap();
            IndexArrayMap(const Size& size);
            
            void add(PrimType primType, size_t index);
            void addPolygon(PrimType primType, size_t index, size_t count);
            
            size_t countIndices() const;
            void getIndices(IndexList& allIndices, PrimTypeToRangeMap& ranges) const;
        private:
            IndexList& findIndices(PrimType primType, size_t toAdd);
        };
    }
}

#endif /* IndexArrayMap_h */
