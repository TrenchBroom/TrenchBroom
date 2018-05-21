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

#ifndef IndexArray_h
#define IndexArray_h

#include "Renderer/Vbo.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * A reference-counted handle to a VboBlock (which is a subset of a VBO).
         *
         * It maintains an in-memory copy of the data that was uploaded to the VBO.
         * Support resizing.
         *
         * Copying the IndexArray just increments the reference count,
         * the same underlying buffer is shared between the copies.
         */
        class IndexArray {
        private:
            using Index = GLuint;

            class Holder;

            std::shared_ptr<Holder> m_holder;

            IndexArray(std::vector<Index>& indices);
        public:
            /**
             * creates an empty IndexArray
             */
            IndexArray();

            bool empty() const;

            static IndexArray swap(std::vector<Index>& indices);

            void resize(size_t newSize);

            void writeElements(const size_t offsetWithinBlock, const std::vector<Index> &elements);

            void zeroRange(const size_t offsetWithinBlock, const size_t count);

            void render(const PrimType primType, const size_t offset, size_t count) const;

            /**
             * Returns true if all of the edits have been uploaded to the VBO.
             */
            bool prepared() const;

            /**
             * Uploads pending changes to the VBO, allocating a VboBlock if needed.
             */
            void prepare(Vbo& vbo);
        };
    }
}

#endif /* IndexArray_h */
