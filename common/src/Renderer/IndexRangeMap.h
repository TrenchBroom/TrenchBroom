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

#pragma once

#include "Renderer/GL.h"
#include "Renderer/PrimType.h"

#include <kdl/enum_array.h>

#include <functional>
#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class VertexArray;

        /**
         * Manages ranges of primitives that consist of vertices stored in a vertex array. For each primitive type,
         * multiple ranges of vertices can be stored, each range having an offset and a length. When rendered using a
         * vertex array, each of the ranges is rendered using the vertices in the array at the range recorded here.
         */
        class IndexRangeMap {
        private:
            struct IndicesAndCounts {
                /**
                 * The offsets of the ranges stored here.
                 */
                GLIndices indices;
                /**
                 * The lengths of the ranges stored here.
                 */
                GLCounts counts;

                IndicesAndCounts();
                IndicesAndCounts(size_t index, size_t count);

                bool empty() const;
                size_t size() const;
                void reserve(size_t capacity);
                void add(PrimType primType, size_t index, size_t count, bool dynamicGrowth);
                void add(const IndicesAndCounts& other, bool dynamicGrowth);
            };

            using PrimTypeToIndexData = kdl::enum_array<IndicesAndCounts, PrimType, PrimTypeCount>;
            using PrimTypeToIndexDataPtr = std::shared_ptr<PrimTypeToIndexData>;
        public:
            /**
             * This helper structure is used to initialize the internal data structures of an index range map to the
             * correct sizes, avoiding the need for costly reallocation of data buffers as data is added.
             *
             * To record the correct sizes, call the inc method with the same parameters for every expected call to the
             * add method of the index range map itself.
             */
            class Size {
            private:
                friend class IndexRangeMap;

                using PrimTypeToSize = kdl::enum_array<std::size_t, PrimType, PrimTypeCount>;
                PrimTypeToSize m_sizes;
            public:
                void inc(PrimType primType, size_t count = 1u);

                /**
                 * Increase the storage by the given size.
                 *
                 * @param other the size to increase by
                 */
                void inc(const Size& other);
            private:
                void initialize(PrimTypeToIndexData& data) const;
            };
        private:
            PrimTypeToIndexDataPtr m_data;
            bool m_dynamicGrowth;
        public:
            /**
             * Creates a new empty index range map that allows for dynamic growth. Note that dynamic growth may
             * incur a performance cost as data buffers are reallocated when they grow.
             */
            IndexRangeMap();

            /**
             * Creates a new index range map and initialize the internal data structures to the sizes recorded in the
             * given size helper.
             *
             * @param size the sizes to initialize this range map to
             */
            explicit IndexRangeMap(const Size& size);

            /**
             * Creates a new index range map containing a single range of the given primitive type, starting at the given
             * index and with the given number of vertices.
             *
             * @param primType the primitive type
             * @param index the start index of the range
             * @param count the number of vertices in the range
             */
            IndexRangeMap(PrimType primType, size_t index, size_t count);

            /**
             * Returns the size of this index range map. An index range map initialized with the returned size can hold
             * exactly the same data as this index range map.
             *
             * @return the size of this index range map
             */
            Size size() const;

            /**
             * Records a range of primitives at the given index with the given length.
             *
             * @param primType the type of primitives in the range
             * @param index the start index of the range
             * @param count the number of vertices in the range
             */
            void add(PrimType primType, size_t index, size_t count);

            /**
             * Adds all data from the given index range map to this one.
             *
             * @param other the index range map to add
             */
            void add(const IndexRangeMap& other);

            /**
             * Renders the primitives stored in this index range map using the vertices in the given vertex array.
             *
             * @param vertexArray the vertex array to render with
             */
            void render(VertexArray& vertexArray) const;

            /**
             * Invokes the given function for each primitive stored in this map.
             *
             * @param func the function to invoke
             */
            void forEachPrimitive(std::function<void(PrimType, size_t index, size_t count)> func) const;
        };
    }
}


