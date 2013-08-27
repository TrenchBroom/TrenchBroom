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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "VboBlock.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        VboBlock::VboBlock(Vbo& vbo, const size_t offset, const size_t capacity, VboBlock* previous, VboBlock* next) :
        m_vbo(vbo),
        m_free(true),
        m_offset(offset),
        m_capacity(capacity),
        m_previous(previous),
        m_next(next) {}
        
        void VboBlock::free() {
            m_vbo.freeBlock(this);
        }
        
        VboBlock* VboBlock::mergeWithSuccessor() {
            assert(m_next != NULL);
            
            VboBlock* next = m_next;
            VboBlock* nextNext = next->next();
            m_next = nextNext;
            if (m_next != NULL)
                m_next->setPrevious(this);
            m_capacity += next->capacity();
            return next;
        }

        VboBlock* VboBlock::split(const size_t capacity) {
            assert(m_capacity > capacity);
            const size_t remainderCapacity = m_capacity - capacity;
            m_capacity = capacity;
            
            VboBlock* remainder = createSuccessor(remainderCapacity);
            remainder->setFree(m_free);
            return remainder;
        }
        
        VboBlock* VboBlock::createSuccessor(const size_t capacity) {
            VboBlock* successor = new VboBlock(m_vbo, m_offset + m_capacity, capacity, this, m_next);
            if (m_next != NULL)
                m_next->setPrevious(successor);
            m_next = successor;
            return m_next;
        }
    }
}
