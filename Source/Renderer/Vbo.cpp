/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "Vbo.h"
#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        void VboBlock::insertBetween(VboBlock* previousBlock, VboBlock* nextBlock) {
            if (previousBlock != NULL) previousBlock->m_next = this;
            m_previous = previousBlock;
            if (nextBlock != NULL) nextBlock->m_previous = this;
            m_next = nextBlock;
        }
        
        void VboBlock::freeBlock() {
            m_vbo.freeBlock(*this);
        }
        
        size_t Vbo::findFreeBlockInRange(size_t address, size_t capacity, size_t start, size_t length) {
            assert(length > 0);
            
            size_t pivot = start + length / 2;
            VboBlock* block = m_freeBlocks[pivot];

            int order = block->compare(address, capacity);
            if (order == 0)
                return pivot;
            if (length == 1) {
                if (order > 0)
                    return pivot;
                return pivot + 1;
            }
            
            if (order > 0)
                return findFreeBlockInRange(address, capacity, start, length / 2);
            return findFreeBlockInRange(address, capacity, pivot, (length + 1) / 2);
        }
        
        size_t Vbo::findFreeBlock(size_t address, size_t capacity) {
            if (m_freeBlocks.empty()) return 0;
            size_t index = findFreeBlockInRange(address, capacity, 0, static_cast<size_t>(m_freeBlocks.size()));
            assert(index == m_freeBlocks.size() ||
                   capacity < m_freeBlocks[index]->capacity() ||
                   (capacity == m_freeBlocks[index]->capacity() && address <= m_freeBlocks[index]->address()));
            return index;
        }

        void Vbo::insertFreeBlock(VboBlock& block) {
            assert(block.free());
            size_t index = findFreeBlock(block.address(), block.capacity());
            assert(index <= m_freeBlocks.size());
            if (index < m_freeBlocks.size()) {
                std::vector<VboBlock*>::iterator it = m_freeBlocks.begin();
                std::advance(it, index);
                m_freeBlocks.insert(it, &block);
            } else {
                m_freeBlocks.push_back(&block);
            }
#ifdef _DEBUG_VBO
            checkFreeBlocks();
#endif
        }
        
        void Vbo::removeFreeBlock(VboBlock& block) {
            assert(block.free());
            size_t index = findFreeBlock(block.address(), block.capacity());
            assert(index < m_freeBlocks.size());
            assert(m_freeBlocks[index] == &block);
            std::vector<VboBlock*>::iterator it = m_freeBlocks.begin();
            std::advance(it, index);
            m_freeBlocks.erase(it);
#ifdef _DEBUG_VBO
            checkFreeBlocks();
#endif
        }
        
        void Vbo::resizeVbo(size_t newCapacity) {
            VboState oldState = m_state;
            
            unsigned char* temp = NULL;
            MemBlock::List memBlocks;
            if (m_vboId != 0 && m_freeCapacity < m_totalCapacity) {
                VboBlock* currentBlock = m_first;
                size_t totalLength = 0;
                
                while (currentBlock != NULL) {
                    while (currentBlock != NULL && currentBlock->free())
                        currentBlock = currentBlock->m_next;
                    if (currentBlock != NULL) {
                        size_t start = currentBlock->address();
                        size_t length = 0;
                        while (currentBlock != NULL && !currentBlock->free()) {
                            length += currentBlock->capacity();
                            currentBlock = currentBlock->m_next;
                        }
                        memBlocks.push_back(MemBlock(start, length));
                        totalLength += length;
                    }
                }
                
                if (m_state < VboActive)
                    activate();
                if (m_state < VboMapped)
                    map();

                temp = new unsigned char[totalLength];
                size_t offset = 0;
                MemBlock::List::const_iterator it, end;
                for (it = memBlocks.begin(), end = memBlocks.end(); it != end; ++it) {
                    const MemBlock& memBlock = *it;
                    memcpy(temp + offset, m_buffer + memBlock.start, memBlock.length);
                    offset += memBlock.length;
                }
            }
            
            size_t addedCapacity = newCapacity - m_totalCapacity;
            m_freeCapacity = newCapacity - (m_totalCapacity - m_freeCapacity);
            m_totalCapacity = newCapacity;
            
            if (m_last->free()) {
                resizeBlock(*m_last, m_last->capacity() + addedCapacity);
            } else {
                VboBlock* block = new VboBlock(*this, m_last->address() + m_last->capacity(), addedCapacity);
                block->insertBetween(m_last, NULL);
                insertFreeBlock(*block);
                m_last = block;
            }
            
            if (m_vboId != 0) {
                if (m_state == VboMapped)
                    unmap();
                if (m_state == VboActive)
                    deactivate();
                glDeleteBuffers(1, &m_vboId);
                m_vboId = 0;
            }
            
            if (temp != NULL) {
                assert(!memBlocks.empty());
                
                if (m_state < VboActive)
                    activate();
                if (m_state < VboMapped)
                    map();
                
                size_t offset = 0;
                MemBlock::List::const_iterator it, end;
                for (it = memBlocks.begin(), end = memBlocks.end(); it != end; ++it) {
                    const MemBlock& memBlock = *it;
                    memcpy(m_buffer + memBlock.start, temp + offset, memBlock.length);
                }
                
                delete [] temp;
                temp = NULL;
                memBlocks.clear();
                
                if (oldState < VboMapped)
                    unmap();
                if (oldState < VboActive)
                    deactivate();
            } else {
                if (oldState > VboInactive && m_state < VboActive)
                    activate();
                if (oldState > VboActive && m_state < VboMapped)
                    map();
            }
            
#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif
        }
        
        void Vbo::resizeBlock(VboBlock& block, size_t newCapacity) {
            if (block.capacity() == newCapacity) return;
            if (block.free()) {
                removeFreeBlock(block);
                block.m_capacity = newCapacity;
                insertFreeBlock(block);
            }
        }
        
        VboBlock* Vbo::packBlock(VboBlock& block) {
            VboBlock* first = block.m_next;
            if (first == NULL)
                return NULL;
            
            VboBlock* previous = NULL;
            VboBlock* last = first;
            size_t size = 0;
            size_t address = first->address();
            
            do {
                last->m_address -= block.capacity();
                size += last->capacity();
                previous = last;
                last = last->m_next;
            } while (last != NULL && !last->free());
            
            memmove(m_buffer + block.address(), m_buffer + address, size);
            
            if (last != NULL) {
                last->m_address -= block.capacity();
                resizeBlock(*last, last->capacity() + block.capacity());
            } else {
                VboBlock* newBlock = new VboBlock(*this, previous->address() + previous->capacity(), block.capacity());
                insertFreeBlock(*newBlock);
                newBlock->insertBetween(previous, NULL);
                m_last = newBlock;
            }
            
            if (m_first == &block) m_first = block.m_next;
            
            removeFreeBlock(block);
            if (block.m_previous != NULL) block.m_previous->m_next = block.m_next;
            if (block.m_next != NULL) block.m_next->m_previous = block.m_previous;
            delete &block;

            return last;
        }
        
        Vbo::Vbo(GLenum type, size_t capacity) : m_type(type), m_totalCapacity(capacity), m_freeCapacity(capacity), m_buffer(NULL), m_vboId(0), m_state(VboInactive) {
            m_first = new VboBlock(*this, 0, m_totalCapacity);
            m_last = m_first;
            m_freeBlocks.push_back(m_first);
#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif
        }
        
        Vbo::~Vbo() {
#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif
            if (m_state == VboMapped)
                unmap();
            if (m_state == VboActive)
                deactivate();
            if (m_vboId != 0)
                glDeleteBuffers(1, &m_vboId);
            m_freeBlocks.clear();
            VboBlock* block = m_first;
            while (block != NULL) {
                VboBlock* next = block->m_next;
                delete block;
                block = next;
            }
        }
        
        void Vbo::activate() {
            assert(m_state != VboActive);
            
            if (m_vboId == 0) {
                glGenBuffers(1, &m_vboId);
                glBindBuffer(m_type, m_vboId);
                glBufferData(m_type, static_cast<GLsizeiptr>(m_totalCapacity), NULL, GL_DYNAMIC_DRAW);
            } else {
                glBindBuffer(m_type, m_vboId);
            }

            GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw VboException(*this, "Vbo could not be activated", error);

            m_state = VboActive;
        }
        
        void Vbo::deactivate() {
            assert(m_state == VboActive);
            
            glBindBuffer(m_type, 0);
            m_state = VboInactive;
        }
        
        void Vbo::map() {
            assert(m_state == VboActive);
            
            m_buffer = (unsigned char *)glMapBuffer(m_type, GL_WRITE_ONLY);
            GLenum error = glGetError();
			if (m_buffer == NULL || error != GL_NO_ERROR)
				throw VboException(*this, "Vbo could not be mapped", error);

            m_state = VboMapped;
        }
        
        void Vbo::unmap() {
            assert(m_state == VboMapped);
            
            glUnmapBuffer(m_type);

            GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw VboException(*this, "Vbo could not be unmapped", error);

			m_buffer = NULL;
            m_state = VboActive;
        }
        
        void Vbo::ensureFreeCapacity(size_t capacity) {
            pack();
            if (m_freeCapacity < capacity)
                resizeVbo(m_totalCapacity + (capacity - m_freeCapacity));
        }

        VboBlock* Vbo::allocBlock(size_t capacity) {
            assert(capacity > 0);
            
#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif

            size_t index = findFreeBlock(0, capacity);
            if (index >= m_freeBlocks.size()) {
                SetVboState mapVbo(*this, VboMapped);
                pack();

                if (capacity > m_freeCapacity) {
                    const size_t usedCapacity = m_totalCapacity - m_freeCapacity;
                    size_t newCapacity = m_totalCapacity;
                    size_t newFreeCapacity = m_freeCapacity;
                    while (capacity > newFreeCapacity) {
                        newCapacity *= 2;
                        newFreeCapacity = newCapacity - usedCapacity;
                    }
                    resizeVbo(newCapacity);
                }
                
                index = findFreeBlock(0, capacity);
                assert(index < m_freeBlocks.size());
            }
            
            VboBlock* block = m_freeBlocks[index];
            std::vector<VboBlock*>::iterator it = m_freeBlocks.begin();
            std::advance(it, index);
            m_freeBlocks.erase(it);
            
            // split block
            if (capacity < block->capacity()) {
                VboBlock* remainder = new VboBlock(*this, block->address() + capacity, block->capacity() - capacity);
                remainder->insertBetween(block, block->m_next);
                block->m_capacity = capacity;
                insertFreeBlock(*remainder);
                if (m_last == block) m_last = remainder;
            }
            
            m_freeCapacity -= block->capacity();
            block->m_free = false;

#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif

            return block;
        }
        
        VboBlock* Vbo::freeBlock(VboBlock& block) {
#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif

            VboBlock* previous = block.m_previous;
            VboBlock* next = block.m_next;
            
            m_freeCapacity += block.capacity();
            block.m_free = true;
            
            if (previous != NULL && previous->free() && next != NULL && next->free()) {
                resizeBlock(*previous, previous->capacity() + block.capacity() + next->capacity());
                if (m_last == next) m_last = previous;
                removeFreeBlock(*next);
                previous->insertBetween(previous->m_previous, next->m_next);
                delete &block;
                delete next;
                return previous;
            }
            
            if (previous != NULL && previous->free()) {
                resizeBlock(*previous, previous->capacity() + block.capacity());
                if (m_last == &block) m_last = previous;
                previous->insertBetween(previous->m_previous, next);
                delete &block;
                return previous;
            }
            
            if (next != NULL && next->free()) {
                if (m_last == next) m_last = &block;
                removeFreeBlock(*next);
                block.m_capacity += next->capacity();
                block.m_free = true;
                block.insertBetween(previous, next->m_next);
                insertFreeBlock(block);
                delete next;
                return &block;
            }
            
            insertFreeBlock(block);

#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif

            return &block;
        }

        void Vbo::freeAllBlocks() {
            m_freeBlocks.clear();
            VboBlock* block = m_first;
            while (block != NULL) {
                VboBlock* next = block->m_next;
                delete block;
                block = next;
            }
            m_first = m_last = new VboBlock(*this, 0, m_totalCapacity);
            m_freeBlocks.push_back(m_first);
            m_freeCapacity = m_totalCapacity;
        }

        void Vbo::pack() {
            assert(m_state == VboMapped);
            
#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif

            if (m_totalCapacity == m_freeCapacity || (m_last->free() && m_last->capacity() == m_freeCapacity)) return;
            
            // find first free block
            VboBlock* block = m_first;
            while (block != NULL && !block->free())
                block = block->m_next;
            while (block != NULL && block->m_next != NULL)
                block = packBlock(*block);

#ifdef _DEBUG_VBO
            checkBlockChain();
            checkFreeBlocks();
#endif
        }

#ifdef _DEBUG_VBO
        void Vbo::checkBlockChain() {
            VboBlock* block = m_first;
            VboBlock* previous = NULL;
            assert(block != NULL && block->m_previous == NULL);
            
            while (block != NULL) {
                assert(&block->m_vbo == this);
                previous = block;
                block = block->m_next;
                assert(block == NULL || block->m_previous == previous);
            }
            
            assert(previous == m_last);
        }
        
        void Vbo::checkFreeBlocks() {
            for (size_t i = 0; i < m_freeBlocks.size(); i++) {
                VboBlock* block = m_freeBlocks[i];
                assert(block->free());
                if (i < m_freeBlocks.size() - 1)
                    assert((block->capacity() < m_freeBlocks[i + 1]->capacity()) || 
                           (block->capacity() == m_freeBlocks[i + 1]->capacity() && block->address() < m_freeBlocks[i + 1]->address()));
            }
        }
#endif
        
        bool Vbo::ownsBlock(VboBlock& block) {
            return &block.m_vbo == this;
        }
    }
}
