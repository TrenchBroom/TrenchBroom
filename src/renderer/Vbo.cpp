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

        SetVboState::SetVboState(Vbo& vbo, const VboState::Type newState) :
        m_vbo(vbo),
        m_previousState(m_vbo.state()) {
            if (newState > m_previousState) {
                if (newState == VboState::Active)
                    m_vbo.activate();
                else if (newState == VboState::Mapped)
                    m_vbo.map();
            } else if (newState < m_previousState) {
                if (newState == VboState::Inactive)
                    m_vbo.deactivate();
                else if (newState == VboState::Active)
                    m_vbo.unmap();
            }
        }
        
        SetVboState::~SetVboState() {
            const VboState::Type currentState = m_vbo.state();
            if (m_previousState > currentState) {
                if (m_previousState == VboState::Active)
                    m_vbo.activate();
                else if (m_previousState == VboState::Mapped)
                    m_vbo.map();
            } else if (m_previousState < currentState) {
                if (m_previousState == VboState::Inactive)
                    m_vbo.deactivate();
                else if (m_previousState == VboState::Active)
                    m_vbo.unmap();
            }
        }

        const float Vbo::GrowthFactor = 1.5f;

        Vbo::Vbo(const size_t initialCapacity, const GLenum type, const GLenum usage) :
        m_totalCapacity(initialCapacity),
        m_freeCapacity(m_totalCapacity),
        m_firstBlock(NULL),
        m_lastBlock(NULL),
        m_state(VboState::Inactive),
        m_type(type),
        m_usage(usage),
        m_vboId(0),
        m_buffer(NULL) {
            m_lastBlock = m_firstBlock = new VboBlock(*this, 0, m_totalCapacity, NULL, NULL);
            m_freeBlocks.push_back(m_firstBlock);
        }
        
        Vbo::~Vbo() {
            if (isActive())
                deactivate();
            
            if (m_vboId > 0) {
                glDeleteBuffers(1, &m_vboId);
                m_vboId = 0;
            }
            
            VboBlock* block = m_firstBlock;
            while (block != NULL) {
                VboBlock* next = block->next();
                delete block;
                block = next;
            }
            
            m_lastBlock = m_firstBlock = NULL;
        }
        
        VboBlock& Vbo::allocateBlock(const size_t capacity) {
            if (m_state != VboState::Mapped) {
                VboException e;
                e << "Vbo is not mapped";
                throw e;
            }

            VboBlockList::iterator it = findFreeBlock(capacity);
            if (it == m_freeBlocks.end()) {
                increaseCapacityToAccomodate(capacity);
                it = findFreeBlock(capacity);
            }
            
            assert(it != m_freeBlocks.end());
            removeFreeBlock(it);
            
            VboBlock* block = *it;
            assert(block != NULL);
            
            if (block->capacity() > capacity) {
                VboBlock* remainder = block->split(capacity);
                if (m_lastBlock == block)
                    m_lastBlock = remainder;
                insertFreeBlock(remainder);
            }
            
            return *block;
        }

        void Vbo::activate() {
            assert(!isActive());
            
            if (m_vboId == 0) {
                glGenBuffers(1, &m_vboId);
                glBindBuffer(m_type, m_vboId);
                glBufferData(m_type, static_cast<GLsizeiptr>(m_totalCapacity), NULL, m_usage);
            } else {
                glBindBuffer(m_type, m_vboId);
            }
            
            m_state = VboState::Active;
        }
        
        void Vbo::deactivate() {
            assert(isActive());
            if (isMapped())
                unmap();
            glBindBuffer(m_type, 0);
            m_state = VboState::Inactive;
        }
        
        void Vbo::map() {
            assert(!isMapped());
            if (!isActive())
                activate();
            m_buffer = reinterpret_cast<unsigned char *>(glMapBuffer(m_type, GL_WRITE_ONLY));
            assert(m_buffer != NULL);
            m_state = VboState::Mapped;
        }
        
        void Vbo::unmap() {
            assert(isMapped());
            glUnmapBuffer(m_type);
			m_buffer = NULL;
            m_state = VboState::Active;
        }

        void Vbo::increaseCapacityToAccomodate(const size_t capacity) {
            assert(m_state == VboState::Mapped);
            
            size_t newMinCapacity = m_totalCapacity + capacity;
            if (m_lastBlock->isFree())
                newMinCapacity -= m_lastBlock->capacity();
            
            size_t newCapacity = m_totalCapacity;
            while (newCapacity < newMinCapacity)
                newCapacity *= GrowthFactor;
            
            increaseCapacity(newCapacity - m_totalCapacity);
        }

        void Vbo::increaseCapacity(const size_t delta) {
            assert(m_state == VboState::Mapped);
            assert(delta > 0);
            
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
            m_freeBlocks.erase(it);
            VboBlock* block = *it;
            block->setFree(false);
            m_freeCapacity -= block->capacity();
        }
    }
}
