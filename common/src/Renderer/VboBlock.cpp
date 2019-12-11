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

#include "VboBlock.h"

#include "Ensure.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        // MapVboBlock

        MapVboBlock::MapVboBlock(VboBlock* block) :
        m_block(block) {
            m_block->bind();
        }

        MapVboBlock::~MapVboBlock() {
            m_block->unbind();
        }

        // VboBlock

        VboBlock::VboBlock(GLenum type, const size_t capacity) :
        m_type(type),
        m_capacity(capacity) {
            assert(m_type == GL_ELEMENT_ARRAY_BUFFER
                   || m_type == GL_ARRAY_BUFFER);

            glAssert(glGenBuffers(1, &m_bufferId));
            glAssert(glBindBuffer(m_type, m_bufferId));
            glAssert(glBufferData(m_type, m_capacity, nullptr, GL_STATIC_DRAW));
        }

        void VboBlock::free() {
            assert(m_bufferId != 0);
            glAssert(glDeleteBuffers(1, &m_bufferId));
            m_bufferId = 0;
        }

        VboBlock::~VboBlock() {
            assert(m_bufferId == 0);
        }

        size_t VboBlock::offset() const {
            return 0;
        }

        size_t VboBlock::capacity() const {
            return m_capacity;
        }

        void VboBlock::bind() {
            assert(m_bufferId != 0);
            glAssert(glBindBuffer(m_type, m_bufferId));
        }

        void VboBlock::unbind() {
            glAssert(glBindBuffer(m_type, 0));
        }
    }
}
