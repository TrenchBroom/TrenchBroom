/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
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
            
            inline Vbo& vbo() const {
                return m_vbo;
            }
            
            inline size_t offset() const {
                return m_offset;
            }
            
            inline size_t capacity() const {
                return m_capacity;
            }
            
            template <typename T>
            inline size_t writeElement(const size_t address, const T& element) {
                return m_vbo.writeElement(m_offset + address, element);
            }
            
            template <typename T>
            inline size_t writeElements(const size_t address, const std::vector<T>& elements) {
                return m_vbo.writeElements(m_offset + address, elements);
            }
            
            template <typename T>
            inline size_t writeBuffer(const size_t address, const std::vector<T>& buffer) {
                return m_vbo.writeBuffer(m_offset + address, buffer);
            }
            
            void free();
        private:
            inline VboBlock* previous() const {
                return m_previous;
            }
            
            inline void setPrevious(VboBlock* previous) {
                m_previous = previous;
            }
            
            inline VboBlock* next() const {
                return m_next;
            }
            
            inline void setNext(VboBlock* next) {
                m_next = next;
            }
            
            inline bool isFree() const {
                return m_free;
            }
            
            inline void setFree(const bool free) {
                m_free = free;
            }
            
            inline void setCapacity(const size_t capacity) {
                m_capacity = capacity;
            }
            
            VboBlock* mergeWithSuccessor();
            VboBlock* split(const size_t capacity);
            VboBlock* createSuccessor(const size_t capacity);
        };
    }
}

#endif /* defined(__TrenchBroom__VboBlock__) */
