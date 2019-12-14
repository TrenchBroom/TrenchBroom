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

#ifndef TrenchBroom_Vbo
#define TrenchBroom_Vbo

#include "Renderer/VboManager.h"

#include <cassert>
#include <vector>
#include <type_traits>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * Wrapper around an OpenGL buffer
         */
        class Vbo {
        private:
            friend class VboManager;

            /**
             * e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
             */
            GLenum m_type;
            size_t m_capacity;
            GLuint m_bufferId;

            /**
             * Immediately creates and binds to a buffer of the given type and capacity.
             * The contents are initially unspecified.
             */
            Vbo(GLenum type, size_t capacity, GLenum usage);
            ~Vbo();

            /**
             * Deletes the underlying OpenGL buffer with glDeleteBuffers.
             * Must be called before the destructor.
             * Calling any other methods after free() is disallowed.
             */
            void free();

        public:
            /**
             * Deprecated, always returns 0.
             */
            size_t offset() const;
            size_t capacity() const;

            void bind();
            void unbind();

            template <typename T>
            size_t writeElements(const size_t address, const std::vector<T>& elements) {
                return writeArray(address, elements.data(), elements.size());
            }

            template <typename T>
            size_t writeBuffer(const size_t address, const std::vector<T>& buffer) {
                return writeArray(address, buffer.data(), buffer.size());
            }

            /**
             * Writes a C array to the VBO block.
             *
             * @tparam T        element type
             * @param address   byte offset from the start of the block to write at
             * @param array     elements to write
             * @param count     number of elements to write
             * @return          number of bytes written
             */
            template <typename T>
            size_t writeArray(const size_t address, const T* array, const size_t count) {
                const size_t size = count * sizeof(T);
                assert(address + size <= m_capacity);

                static_assert(std::is_trivially_copyable<T>::value);
                static_assert(std::is_standard_layout<T>::value);

                const GLvoid* ptr = static_cast<const GLvoid*>(array);
                const GLintptr offset = static_cast<GLintptr>(address);
                const GLsizeiptr sizei = static_cast<GLsizeiptr>(size);
                glAssert(glBindBuffer(m_type, m_bufferId));
                glAssert(glBufferSubData(m_type, offset, sizei, ptr));

                return size;
            }
        };
    }
}

#endif /* defined(TrenchBroom_Vbo) */
