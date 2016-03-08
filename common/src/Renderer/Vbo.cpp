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

#include "Vbo.h"

#include "Exceptions.h"
#include "Renderer/VboBlock.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        bool CompareVboBlocksByCapacity::operator() (const VboBlock* lhs, const VboBlock* rhs) const {
            assert(lhs != NULL);
            assert(rhs != NULL);
            return lhs->capacity() < rhs->capacity();
        }

        ActivateVbo::ActivateVbo(Vbo& vbo) :
        m_vbo(vbo),
        m_wasActive(m_vbo.active()) {
            if (!m_wasActive)
                m_vbo.activate();
        }
        
        ActivateVbo::~ActivateVbo() {
            if (!m_wasActive)
                m_vbo.deactivate();
        }

        const float Vbo::GrowthFactor = 1.5f;

        Vbo::Vbo(const size_t initialCapacity, const GLenum type, const GLenum usage) :
        m_totalCapacity(initialCapacity),
        m_freeCapacity(m_totalCapacity),
        m_firstBlock(NULL),
        m_lastBlock(NULL),
        m_state(State_Inactive),
        m_type(type),
        m_usage(usage),
        m_vboId(0) {
            m_lastBlock = m_firstBlock = new VboBlock(*this, 0, m_totalCapacity, NULL, NULL);
            m_freeBlocks.push_back(m_firstBlock);
            assert(checkBlockChain());
        }
        
        Vbo::~Vbo() {
            if (active())
                deactivate();
            free();
            
            VboBlock* block = m_firstBlock;
            while (block != NULL) {
                VboBlock* next = block->next();
                delete block;
                block = next;
            }
            
            m_lastBlock = m_firstBlock = NULL;
        }
        
        VboBlock* Vbo::allocateBlock(const size_t capacity) {
            assert(checkBlockChain());

            if (!active()) {
                VboException e;
                e << "Vbo is inactive";
                throw e;
            }

            VboBlockList::iterator it = findFreeBlock(capacity);
            if (it == m_freeBlocks.end()) {
                increaseCapacityToAccomodate(capacity);
                it = findFreeBlock(capacity);
            }
            
            assert(it != m_freeBlocks.end());
            VboBlock* block = *it;
            assert(block != NULL);
            removeFreeBlock(it);
            
            if (block->capacity() > capacity) {
                VboBlock* remainder = block->split(capacity);
                if (m_lastBlock == block)
                    m_lastBlock = remainder;
                insertFreeBlock(remainder);
            }
            
            assert(checkBlockChain());
            return block;
        }

        bool Vbo::active() const {
            return m_state > State_Inactive;
        }
        
        void Vbo::activate() {
            assert(!active());
            
            if (m_vboId == 0) {
                glAssert(glGenBuffers(1, &m_vboId));
                glAssert(glBindBuffer(m_type, m_vboId));
                glAssert(glBufferData(m_type, static_cast<GLsizeiptr>(m_totalCapacity), NULL, m_usage));
            } else {
                glAssert(glBindBuffer(m_type, m_vboId));
            }
            m_state = State_Active;
        }
        
        void Vbo::deactivate() {
            assert(active());
            assert(!fullyMapped());
            assert(!partiallyMapped());
            glAssert(glBindBuffer(m_type, 0));
            m_state = State_Inactive;
        }
        
        GLenum Vbo::type() const {
            return m_type;
        }

        void Vbo::free() {
            if (m_vboId > 0) {
                glAssert(glDeleteBuffers(1, &m_vboId));
                m_vboId = 0;
            }
        }

        void Vbo::freeBlock(VboBlock* block) {
            assert(block != NULL);
            assert(!block->isFree());
            assert(checkBlockChain());
            
            VboBlock* previous = block->previous();
            VboBlock* next = block->next();
            
            if (previous != NULL && previous->isFree() &&
                next != NULL && next->isFree()) {
                removeFreeBlock(previous);
                removeFreeBlock(next);
                previous->mergeWithSuccessor();
                previous->mergeWithSuccessor();
                if (m_lastBlock == next)
                    m_lastBlock = previous;
                delete block;
                delete next;
                insertFreeBlock(previous);
            } else if (previous != NULL && previous->isFree()) {
                removeFreeBlock(previous);
                previous->mergeWithSuccessor();
                if (m_lastBlock == block)
                    m_lastBlock = previous;
                delete block;
                insertFreeBlock(previous);
            } else if (next != NULL && next->isFree()) {
                removeFreeBlock(next);
                block->mergeWithSuccessor();
                if (m_lastBlock == next)
                    m_lastBlock = block;
                delete next;
                insertFreeBlock(block);
            } else {
                insertFreeBlock(block);
            }
            assert(checkBlockChain());
        }

        void Vbo::increaseCapacityToAccomodate(const size_t capacity) {
            size_t newMinCapacity = m_totalCapacity + capacity;
            if (m_lastBlock->isFree())
                newMinCapacity -= m_lastBlock->capacity();
            
            size_t newCapacity = m_totalCapacity;
            while (newCapacity < newMinCapacity)
                newCapacity = static_cast<size_t>(static_cast<float>(newCapacity) * GrowthFactor);
            
            increaseCapacity(newCapacity - m_totalCapacity);
        }

        void Vbo::increaseCapacity(const size_t delta) {
            assert(active());
            assert(!partiallyMapped());
            assert(!fullyMapped());
            assert(delta > 0);
            assert(checkBlockChain());

            const size_t begin = (m_firstBlock->isFree() ?  m_firstBlock->capacity() : 0);
            const size_t end = m_totalCapacity - (m_lastBlock->isFree() ? m_lastBlock->capacity() : 0);
            
            if (m_lastBlock->isFree()) {
                removeFreeBlock(m_lastBlock);
                m_lastBlock->setCapacity(m_lastBlock->capacity() + delta);
                insertFreeBlock(m_lastBlock);
            } else {
                VboBlock* block = m_lastBlock->createSuccessor(delta);
                m_lastBlock = block;
                insertFreeBlock(m_lastBlock);
            }
            
            m_totalCapacity += delta;
            m_freeCapacity += delta;
            assert(checkBlockChain());
            
            if (begin < end) {
                unsigned char* buffer = map();
                
                unsigned char* temp = new unsigned char[end - begin];
                memcpy(temp, buffer + begin, end - begin);
                
                unmap();
                deactivate();
                free();
                activate();
                buffer = map();
                
                memcpy(buffer + begin, temp, end - begin);
                delete [] temp;
                
                unmap();
            } else {
                deactivate();
                free();
                activate();
            }
        }

        Vbo::VboBlockList::iterator Vbo::findFreeBlock(const size_t minCapacity) {
            VboBlock query(*this, 0, minCapacity, NULL, NULL);
            return std::lower_bound(m_freeBlocks.begin(), m_freeBlocks.end(), &query, CompareVboBlocksByCapacity());
        }

        void Vbo::insertFreeBlock(VboBlock* block) {
            VboBlockList::iterator it = std::lower_bound(m_freeBlocks.begin(), m_freeBlocks.end(), block, CompareVboBlocksByCapacity());
            if (it == m_freeBlocks.end())
                m_freeBlocks.push_back(block);
            else
                m_freeBlocks.insert(it, block);
            block->setFree(true);
            m_freeCapacity += block->capacity();
        }

        void Vbo::removeFreeBlock(VboBlock* block) {
            VboBlockList::iterator it = std::lower_bound(m_freeBlocks.begin(), m_freeBlocks.end(), block, CompareVboBlocksByCapacity());
            assert(it != m_freeBlocks.end());
            if (*it != block) {
                const VboBlockList::iterator end = std::upper_bound(m_freeBlocks.begin(), m_freeBlocks.end(), block, CompareVboBlocksByCapacity());
                while (it != end && *it != block)
                    ++it;
                assert(it != end);
            }
            assert(*it == block);
            removeFreeBlock(it);
        }

        void Vbo::removeFreeBlock(const VboBlockList::iterator it) {
            VboBlock* block = *it;
            m_freeBlocks.erase(it);
            block->setFree(false);
            m_freeCapacity -= block->capacity();
        }

        bool Vbo::partiallyMapped() const {
            return m_state == State_PartiallyMapped;
        }
        
        void Vbo::mapPartially() {
            assert(active());
            assert(!partiallyMapped());
            assert(!fullyMapped());
            m_state = State_PartiallyMapped;
        }
        
        void Vbo::unmapPartially() {
            assert(active());
            assert(partiallyMapped());
            m_state = State_Active;
        }

        bool Vbo::fullyMapped() const {
            return m_state == State_FullyMapped;
        }
        
        unsigned char* Vbo::map() {
            assert(active());
            assert(!fullyMapped());
            assert(!partiallyMapped());

#ifdef __APPLE__
            // fixes a crash on Mac OS X where a buffer could not be mapped after another windows was closed
            glAssert(glFinishObjectAPPLE(GL_BUFFER_OBJECT_APPLE, static_cast<GLint>(m_vboId)));
#endif
            unsigned char* buffer = reinterpret_cast<unsigned char *>(glMapBuffer(m_type, GL_WRITE_ONLY));
            assert(buffer != NULL);
            m_state = State_FullyMapped;
            
            return buffer;
        }
        
        void Vbo::unmap() {
            assert(fullyMapped());
            glAssert(glUnmapBuffer(m_type));
            m_state = State_Active;
        }

        bool Vbo::checkBlockChain() const {
            VboBlock* block = m_firstBlock;
            assert(block != NULL);
            
            size_t count = 0;
            VboBlock* next = block->next();
            while (next != NULL) {
                assert(next->previous() == block);
                assert(!block->isFree() || !next->isFree());
                block = next;
                next = next->next();
                ++count;
            }
            
            assert(block == m_lastBlock);
            
            VboBlock* previous = block->previous();
            while (previous != NULL) {
                assert(previous->next() == block);
                block = previous;
                previous = previous->previous();
                --count;
            }
            assert(count == 0);
            
            return true;
        }
    }
}
