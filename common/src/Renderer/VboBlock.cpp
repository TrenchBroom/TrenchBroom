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

#include "VboBlock.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        MapVboBlock::MapVboBlock(VboBlock* block) :
        m_block(block) {
            assert(m_block != NULL);
            m_block->map();
        }
        
        MapVboBlock::~MapVboBlock() {
            m_block->unmap();
        }

        VboBlock::VboBlock(Vbo& vbo, const size_t offset, const size_t capacity, VboBlock* previous, VboBlock* next) :
        m_vbo(vbo),
        m_free(true),
        m_offset(offset),
        m_capacity(capacity),
        m_previous(previous),
        m_next(next),
        m_mapped(false) {}
        
        Vbo& VboBlock::vbo() const {
            return m_vbo;
        }
        
        size_t VboBlock::offset() const {
            return m_offset;
        }
        
        size_t VboBlock::capacity() const {
            return m_capacity;
        }

        void VboBlock::free() {
            m_vbo.freeBlock(this);
        }
        
        bool VboBlock::mapped() const {
            return m_mapped;
        }
        
        void VboBlock::map() {
            assert(!mapped());
            m_mapped = true;
            m_vbo.mapPartially();
        }
        
        void VboBlock::unmap() {
            assert(mapped());
            m_mapped = false;
            m_vbo.unmapPartially();
        }

        VboBlock* VboBlock::previous() const {
            return m_previous;
        }
        
        void VboBlock::setPrevious(VboBlock* previous) {
            m_previous = previous;
        }
        
        VboBlock* VboBlock::next() const {
            return m_next;
        }
        
        void VboBlock::setNext(VboBlock* next) {
            m_next = next;
        }
        
        bool VboBlock::isFree() const {
            return m_free;
        }
        
        void VboBlock::setFree(const bool free) {
            m_free = free;
        }
        
        void VboBlock::setCapacity(const size_t capacity) {
            m_capacity = capacity;
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
