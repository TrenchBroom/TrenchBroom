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

#ifndef __TrenchBroom__VboBlock__
#define __TrenchBroom__VboBlock__

#include "Renderer/Vbo.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class VboBlock {
        private:
            friend class Vbo;
            
            Vbo& m_vbo;
            bool m_free;
            size_t m_offset;
            size_t m_capacity;
            VboBlock* m_previous;
            VboBlock* m_next;
        public:
            VboBlock(Vbo& vbo, const size_t offset, const size_t capacity, VboBlock* previous, VboBlock* next);
            
            Vbo& vbo() const;
            size_t offset() const;
            size_t capacity() const;
            
            template <typename T>
            size_t writeElement(const size_t address, const T& element) {
                assert(address + sizeof(T) <= m_capacity);
                return m_vbo.writeElement(m_offset + address, element);
            }
            
            template <typename T>
            size_t writeElements(const size_t address, const std::vector<T>& elements) {
                assert(address + elements.size() * sizeof(T) <= m_capacity);
                return m_vbo.writeElements(m_offset + address, elements);
            }
            
            template <typename T>
            size_t writeBuffer(const size_t address, const std::vector<T>& buffer) {
                assert(address + buffer.size() * sizeof(T) <= m_capacity);
                return m_vbo.writeBuffer(m_offset + address, buffer);
            }
            
            void free();
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

#endif /* defined(__TrenchBroom__VboBlock__) */
