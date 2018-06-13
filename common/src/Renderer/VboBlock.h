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

#ifndef TrenchBroom_VboBlock
#define TrenchBroom_VboBlock

#include "Renderer/Vbo.h"

#include <cstring>
#include <vector>
#include <type_traits>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class VboBlock;
        class MapVboBlock {
        private:
            VboBlock* m_block;
        public:
            MapVboBlock(VboBlock* block);
            ~MapVboBlock();
        };
        
        class VboBlock {
        private:
            friend class Vbo;
            friend class MapVboBlock;
            
            Vbo& m_vbo;
            bool m_free;
            size_t m_offset;
            size_t m_capacity;
            VboBlock* m_previous;
            VboBlock* m_next;
            
            bool m_mapped;
        public:
            VboBlock(Vbo& vbo, const size_t offset, const size_t capacity, VboBlock* previous, VboBlock* next);
            
            Vbo& vbo() const;
            size_t offset() const;
            size_t capacity() const;
            
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
                assert(mapped());
                
                const size_t size = count * sizeof(T);
                assert(address + size <= m_capacity);

                static_assert(std::is_trivially_copyable<T>::value);
                static_assert(std::is_standard_layout<T>::value);

                const GLvoid* ptr = static_cast<const GLvoid*>(array);
                const GLintptr offset = static_cast<GLintptr>(m_offset + address);
                const GLsizeiptr sizei = static_cast<GLsizeiptr>(size);
                glAssert(glBufferSubData(m_vbo.type(), offset, sizei, ptr));
                
                return size;
            }

            void free();
        private:
            bool mapped() const;
            void map();
            void unmap();
        private:
            VboBlock* previous() const;
            void setPrevious(VboBlock* previous);
            VboBlock* next() const;
            void setNext(VboBlock* next);
            
            bool isFree() const;
            void setFree(const bool free);
            void setCapacity(const size_t capacity);
            
            VboBlock* mergeWithSuccessor();
            VboBlock* split(const size_t capacity);
            VboBlock* createSuccessor(const size_t capacity);
        };
    }
}

#endif /* defined(TrenchBroom_VboBlock) */
