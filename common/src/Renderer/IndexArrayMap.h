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

#include <unordered_map>

namespace TrenchBroom {
    namespace Renderer {
        class IndexArray;

        /**
         * Manages ranges of primitives to be rendered using indices stored in an IndexArray instance. For each call to
         * the add method, the range of primitives of a given type is extended by the given number of indices.
         *
         * For each primitive type, this map stores a range of indices to be rendered. A range is made up of the offset
         * into the index data and the number of indices contained in the range.
         *
         * When the render method is called, the stored ranges are rendered by issuing the appropriate calls with the
         * corresponding recorded range data.
         */
        class IndexArrayMap {
        private:
            /**
             * And index array range, consisting of the offset and the number of indices contained in the range. The
             * capacity is only recorded for debugging purposes.
             */
            struct IndexArrayRange {
                size_t offset;
                size_t capacity;
                size_t count;

                IndexArrayRange(size_t i_offset, size_t i_capacity);
                size_t add(size_t count);
            };

            using PrimTypeToRangeMap = std::unordered_map<PrimType, IndexArrayRange>;
        public:
            /**
             * This helper structure is used to initialize the internal data structures of an index array map to the
             * correct sizes, avoiding the need for constly reallocation of data buffers as data is added.
             *
             * To record the correct sizes, call the inc method with the same parameters for every expected call to the
             * add method of the index array map itself.
             */
            class Size {
            private:
                friend class IndexArrayMap;

                using PrimTypeToSize = std::unordered_map<PrimType, size_t>;
                PrimTypeToSize m_sizes;
                size_t m_indexCount;
            public:
                /**
                 * Creates a new empty size helper.
                 */
                Size();

                /**
                 * Increase the storage for the given primitive type by the given number of indices.
                 *
                 * @param primType the primitive type
                 * @param count the number of primitives to account for
                 */
                void inc(PrimType primType, size_t count);

                /**
                 * Increase the storage by the given size.
                 *
                 * @param other the size to increase by
                 */
                void inc(const Size& other);

                /**
                 * The total number of indices that have been accounted for.
                 *
                 * @return the number of indices
                 */
                size_t indexCount() const;
            private:
                void initialize(PrimTypeToRangeMap& ranges, size_t baseOffset) const;
            };
        private:
            PrimTypeToRangeMap m_ranges;
        public:
            /**
             * Creates a new empty index array map and initializes the internal data structures to the expected sizes
             * indicates by the given data.
             *
             * @param size the sizes of the index array map to initialize to
             */
            explicit IndexArrayMap(const Size& size);

            /**
             * Creates a new empty index array map and initializes the internal data structures to the expected sizes
             * indicated by the given data. Additionally, the given base offset is added to the recorded offset of each
             * primitive range.
             *
             * @param size the sizes of the index array map to initialize to
             * @param baseOffset the base offset for all primitive indices recorded in this index array range map
             */
            IndexArrayMap(const Size& size, size_t baseOffset);

            /**
             * Returns the size of this index array map. An index array map initialized with the returned size can hold
             * exactly the same data as this index array map.
             *
             * @return the size of this index array map
             */
            Size size() const;

            /**
             * Adds the given number of primitives of the given type to this range map. Effectively, the range of
             * primitives of the given type that has been recorded so far is extended by the given number of indices.
             *
             * @param primType the type of primitive
             * @param count the number of indices
             * @return the offset of the next block that would be recorded for the given primitive type
             */
            size_t add(PrimType primType, size_t count);

            /**
             * Renders the recorded primitives using the indices stored in the given index array.
             *
             * @param indexArray the index array to render
             */
            void render(IndexArray& indexArray) const;
        };
    }
}
