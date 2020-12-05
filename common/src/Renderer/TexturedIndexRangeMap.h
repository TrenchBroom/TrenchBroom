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

#include "Renderer/IndexRangeMap.h"

#include <map>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Renderer {
        class TextureRenderFunc;
        class VertexArray;

        /**
         * Manages ranges of textured primitives that consist of vertices stored in a vertex array. For each primitive
         * type, multiple ranges of vertices can be stored, each range having an offset and a length. When rendered
         * using a vertex array, each of the ranges is rendered using the vertices in the array at the range recorded
         * here.
         *
         * The primitives are grouped per texture to avoid costly texture switches during rendering.
         */
        class TexturedIndexRangeMap {
        public:
            using Texture = Assets::Texture;
        private:
            using TextureToIndexRangeMap = std::map<const Texture*, IndexRangeMap>;
            using TextureToIndexRangeMapPtr = std::shared_ptr<TextureToIndexRangeMap>;
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
                friend class TexturedIndexRangeMap;

                using TextureToSize = std::map<const Texture*, IndexRangeMap::Size>;
                TextureToSize m_sizes;
                TextureToSize::iterator m_current;
            public:
                /**
                 * Creates a new instance initialized to 0.
                 */
                Size();

                /**
                 * Count the given primitive.
                 *
                 * @param texture the texture
                 * @param primType the primitive type
                 * @param vertexCount the number of vertices to count
                 */
                void inc(const Texture* texture, PrimType primType, size_t vertexCount = 1);

                /**
                 * Increase the storage by the given size.
                 *
                 * @param other the size to increase by
                 */
                void inc(const Size& other);
            private:
                IndexRangeMap::Size& findCurrent(const Texture* texture);
                bool isCurrent(const Texture* texture) const;

                void initialize(TextureToIndexRangeMap& data) const;
            };
        private:
            TextureToIndexRangeMapPtr m_data;
            TextureToIndexRangeMap::iterator m_current;
        public:
            /**
             * Creates a new empty index range map that allows for dynamic growth. Note that dynamic growth may
             * incur a performance cost as data buffers are reallocated when they grow.
             */
            TexturedIndexRangeMap();

            /**
             * Creates a new index range map and initialize the internal data structures to the sizes recorded in the
             * given size helper.
             *
             * @param size the sizes to initialize this range map to
             */
            explicit TexturedIndexRangeMap(const Size& size);

            /**
             * Creates a new index range map containing the given primitives with the given texture.
             *
             * @param texture the texture
             * @param primitives an index range map containing the primitives
             */
            TexturedIndexRangeMap(const Texture* texture, IndexRangeMap primitives);

            /**
             * Creates a new index range map containing a single range of the given primitive type and texture,
             * starting at the given index and with the given number of vertices.
             *
             * @param texture the texture
             * @param primType the primitive type
             * @param index the start index of the range
             * @param vertexCount the number of vertices in the range
             */
            TexturedIndexRangeMap(const Texture* texture, PrimType primType, size_t index, size_t vertexCount);

            /**
             * Records a range of primitives at the given index with the given length and using the given texture.
             *
             * @param texture the texture to use
             * @param primType the type of primitives in the range
             * @param index the start index of the range
             * @param vertexCount the number of vertices in the range
             */
            void add(const Texture* texture, PrimType primType, size_t index, size_t vertexCount);

            /**
             * Records ranges of primitives using the given texture.
             *
             * @param texture the texture to use
             * @param primitives an index range map containing the primitives
             */
            void add(const Texture* texture, IndexRangeMap primitives);

            /**
             * Adds all ranges stored in the given textured index range map to this one.
             *
             * @param other the textured index range map to add
             */
            void add(const TexturedIndexRangeMap& other);

            /**
             * Renders the primitives stored in this index range map using the vertices in the given vertex array.
             * The primitives are batched by their associated textures.
             *
             * @param vertexArray the vertex array to render with
             */
            void render(VertexArray& vertexArray);

            /**
             * Renders the primitives stored in this index range map using the vertices in the given vertex array.
             * The primitives are batched by their associated textures. The given render function type provides two callbacks. One is
             * called before all primitives with a given texture is rendered, and one is called afterwards.
             *
             * @param vertexArray the vertex array to render with
             * @param func the texture callbacks
             */
            void render(VertexArray& vertexArray, TextureRenderFunc& func);

            /**
             * Invokes the given function for each primitive stored in this map.
             *
             * @param func the function to invoke
             */
            void forEachPrimitive(std::function<void(const Texture* texture, PrimType, size_t index, size_t count)> func) const;
        private:
            IndexRangeMap& findCurrent(const Texture* texture);
            bool isCurrent(const Texture* texture) const;
        };
    }
}

