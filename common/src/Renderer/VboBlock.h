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

#ifndef TrenchBroom_VboBlock
#define TrenchBroom_VboBlock

#include "Renderer/Vbo.h"

#include <cstring>
#include <vector>

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
            
            unsigned char* m_buffer;
        public:
            VboBlock(Vbo& vbo, const size_t offset, const size_t capacity, VboBlock* previous, VboBlock* next);
            ~VboBlock();
            
            Vbo& vbo() const;
            size_t offset() const;
            size_t capacity() const;
            
            template <typename T>
            size_t writeElement(const size_t address, const T& element) {
                assert(mapped());
                assert(address + sizeof(T) <= m_capacity);
                reinterpret_cast<T>(m_buffer + address) = element;
            }
            
            template <typename T>
            size_t writeElements(const size_t address, const std::vector<T>& elements) {
                assert(mapped());
                assert(address + elements.size() * sizeof(T) <= m_capacity);
                typename std::vector<T>::const_iterator it, end;
                for (it = elements.begin(), end = elements.end(); it != end; ++it)
                    reinterpret_cast<T>(m_buffer + address) = *it;
                return elements.size() * sizeof(T);
            }
            
            template <typename T>
            size_t writeBuffer(const size_t address, const std::vector<T>& buffer) {
                assert(mapped());
                assert(address + buffer.size() * sizeof(T) <= m_capacity);
                const T* ptr = &(buffer[0]);
                const size_t size = buffer.size() * sizeof(T);
                memcpy(m_buffer + address, ptr, size);
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
