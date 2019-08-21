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

#include "Vbo.h"

#include "Exceptions.h"
#include "Renderer/VboBlock.h"

#include <algorithm>
#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        bool CompareVboBlocksByCapacity::operator() (const VboBlock* lhs, const VboBlock* rhs) const {
            ensure(lhs != nullptr, "lhs is null");
            ensure(rhs != nullptr, "rhs is null");
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
        m_firstBlock(nullptr),
        m_lastBlock(nullptr),
        m_state(State_Inactive),
        m_type(type),
        m_usage(usage),
        m_vboId(0) {
            // cppcheck-suppress noCopyConstructor
            // cppcheck-suppress noOperatorEq
            m_lastBlock = m_firstBlock = new VboBlock(*this, 0, m_totalCapacity, nullptr, nullptr);
            m_freeBlocks.push_back(m_firstBlock);
            assert(checkBlockChain());
        }

        Vbo::~Vbo() {
            if (active()) {
                deactivate();
            }
            free();

            VboBlock* block = m_firstBlock;
            while (block != nullptr) {
                VboBlock* next = block->next();
                delete block;
                block = next;
            }

            m_lastBlock = m_firstBlock = nullptr;
        }

        VboBlock* Vbo::allocateBlock(const size_t capacity) {
            assert(checkBlockChain());

            if (!active()) {
                VboException e;
                e << "Vbo is inactive";
                throw e;
            }

            auto it = findFreeBlock(capacity);
            if (it == std::end(m_freeBlocks)) {
                increaseCapacityToAccomodate(capacity);
                it = findFreeBlock(capacity);
            }

            assert(it != std::end(m_freeBlocks));
            VboBlock* block = *it;

            ensure(block != nullptr, "block is null");
            removeFreeBlock(it);

            if (block->capacity() > capacity) {
                VboBlock* remainder = block->split(capacity);
                if (m_lastBlock == block) {
                    m_lastBlock = remainder;
                }
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
                glAssert(glBufferData(m_type, static_cast<GLsizeiptr>(m_totalCapacity), nullptr, m_usage));
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

        void Vbo::enqueueBlockForFreeing(VboBlock* block) {
            m_blocksPendingFree.push_back(block);
        }
        
        void Vbo::freePendingBlocks() {
            for (VboBlock* block : m_blocksPendingFree) {
                freeBlock(block);
            }
            m_blocksPendingFree.clear();
        }
        
        void Vbo::freeBlock(VboBlock* block) {
            ensure(block != nullptr, "block is null");
            assert(!block->isFree());
            assert(checkBlockChain());

            VboBlock* previous = block->previous();
            VboBlock* next = block->next();

            if (previous != nullptr && previous->isFree() &&
                next != nullptr && next->isFree()) {
                removeFreeBlock(previous);
                removeFreeBlock(next);
                previous->mergeWithSuccessor();
                previous->mergeWithSuccessor();
                if (m_lastBlock == next) {
                    m_lastBlock = previous;
                }
                delete block;
                delete next;
                insertFreeBlock(previous);
            } else if (previous != nullptr && previous->isFree()) {
                removeFreeBlock(previous);
                previous->mergeWithSuccessor();
                if (m_lastBlock == block) {
                    m_lastBlock = previous;
                }
                delete block;
                insertFreeBlock(previous);
            } else if (next != nullptr && next->isFree()) {
                removeFreeBlock(next);
                block->mergeWithSuccessor();
                if (m_lastBlock == next) {
                    m_lastBlock = block;
                }
                delete next;
                insertFreeBlock(block);
            } else {
                insertFreeBlock(block);
            }
            assert(checkBlockChain());
        }

        void Vbo::increaseCapacityToAccomodate(const size_t capacity) {
            auto newMinCapacity = m_totalCapacity + capacity;
            if (m_lastBlock->isFree()) {
                newMinCapacity -= m_lastBlock->capacity();
            }

            auto newCapacity = m_totalCapacity;
            while (newCapacity < newMinCapacity) {
                newCapacity = static_cast<size_t>(static_cast<float>(newCapacity) * GrowthFactor);
            }

            increaseCapacity(newCapacity - m_totalCapacity);
        }

        void Vbo::increaseCapacity(const size_t delta) {
            assert(active());
            assert(!partiallyMapped());
            assert(!fullyMapped());
            assert(delta > 0);
            assert(checkBlockChain());

            const auto begin = (m_firstBlock->isFree() ?  m_firstBlock->capacity() : 0);
            const auto end = m_totalCapacity - (m_lastBlock->isFree() ? m_lastBlock->capacity() : 0);

            if (m_lastBlock->isFree()) {
                removeFreeBlock(m_lastBlock);
                m_lastBlock->setCapacity(m_lastBlock->capacity() + delta);
                insertFreeBlock(m_lastBlock);
            } else {
                auto block = m_lastBlock->createSuccessor(delta);
                m_lastBlock = block;
                insertFreeBlock(m_lastBlock);
            }

            m_totalCapacity += delta;
            m_freeCapacity += delta;
            assert(checkBlockChain());

            if (begin < end) {
                unsigned char* buffer = map();

                auto temp = std::make_unique<unsigned char[]>(end - begin);
                memcpy(temp.get(), buffer + begin, end - begin);

                unmap();
                deactivate();
                free();
                activate();
                buffer = map();

                memcpy(buffer + begin, temp.get(), end - begin);

                unmap();
            } else {
                deactivate();
                free();
                activate();
            }
        }

        Vbo::VboBlockList::iterator Vbo::findFreeBlock(const size_t minCapacity) {
            const VboBlock query(*this, 0, minCapacity, nullptr, nullptr);
            return std::lower_bound(std::begin(m_freeBlocks), std::end(m_freeBlocks), &query, CompareVboBlocksByCapacity());
        }

        void Vbo::insertFreeBlock(VboBlock* block) {
            auto it = std::lower_bound(std::begin(m_freeBlocks), std::end(m_freeBlocks), block, CompareVboBlocksByCapacity());
            if (it == std::end(m_freeBlocks)) {
                m_freeBlocks.push_back(block);
            } else {
                m_freeBlocks.insert(it, block);
            }
            block->setFree(true);
            m_freeCapacity += block->capacity();
        }

        void Vbo::removeFreeBlock(VboBlock* block) {
            auto it = std::lower_bound(std::begin(m_freeBlocks), std::end(m_freeBlocks), block, CompareVboBlocksByCapacity());
            assert(it != std::end(m_freeBlocks));
            if (*it != block) {
                const auto end = std::upper_bound(std::begin(m_freeBlocks), std::end(m_freeBlocks), block, CompareVboBlocksByCapacity());
                while (it != end && *it != block) {
                    ++it;
                }
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
            ensure(buffer != nullptr, "buffer is null");
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
            ensure(block != nullptr, "block is null");

            auto count = 0u;
            VboBlock* next = block->next();
            while (next != nullptr) {
                assert(next->previous() == block);
                assert(!block->isFree() || !next->isFree());
                block = next;
                next = next->next();
                ++count;
            }

            assert(block == m_lastBlock);

            VboBlock* previous = block->previous();
            while (previous != nullptr) {
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
