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

#include "VertexArray.h"

#include <cassert>
#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        VertexArray::VertexArray() :
        m_prepared(false),
        m_setup(false) {}
        
        VertexArray& VertexArray::operator=(VertexArray other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(VertexArray& left, VertexArray& right) {
            using std::swap;
            swap(left.m_holder, right.m_holder);
            swap(left.m_prepared, right.m_prepared);
            swap(left.m_setup, right.m_setup);
        }

        bool VertexArray::empty() const {
            return vertexCount() == 0;
        }

        size_t VertexArray::sizeInBytes() const {
            return m_holder == NULL ? 0 : m_holder->sizeInBytes();
        }

        size_t VertexArray::vertexCount() const {
            return m_holder == NULL ? 0 : m_holder->vertexCount();
        }

        bool VertexArray::prepared() const {
            return m_prepared;
        }

        void VertexArray::prepare(Vbo& vbo) {
            if (!prepared() && !empty())
                m_holder->prepare(vbo);
            m_prepared = true;
        }

        bool VertexArray::setup() {
            if (empty())
                return false;

            assert(prepared());
            assert(!m_setup);
            
            
            m_holder->setup();
            m_setup = true;
            return true;
        }
        
        void VertexArray::cleanup() {
            assert(m_setup);
            assert(!empty());
            m_holder->cleanup();
            m_setup = false;
        }

        void VertexArray::render(const PrimType primType) {
            render(primType, 0, static_cast<GLsizei>(vertexCount()));
        }

        void VertexArray::render(const PrimType primType, const GLint index, const GLsizei count) {
            assert(prepared());
            if (!m_setup) {
                if (setup()) {
                    glAssert(glDrawArrays(primType, index, count));
                    cleanup();
                }
            } else {
                glAssert(glDrawArrays(primType, index, count));
            }
        }

        void VertexArray::render(const PrimType primType, const GLIndices& indices, const GLCounts& counts, const GLint primCount) {
            assert(prepared());
            if (!m_setup) {
                if (setup()) {
                    const GLint* indexArray   = indices.data();
                    const GLsizei* countArray = counts.data();
                    glAssert(glMultiDrawArrays(primType, indexArray, countArray, primCount));
                    cleanup();
                }
            } else {
                const GLint* indexArray   = indices.data();
                const GLsizei* countArray = counts.data();
                glAssert(glMultiDrawArrays(primType, indexArray, countArray, primCount));
            }
            
        }

        void VertexArray::render(const PrimType primType, const GLIndices& indices, const GLsizei count) {
            assert(prepared());
            if (!m_setup) {
                if (setup()) {
                    const GLint* indexArray = indices.data();
                    glAssert(glDrawElements(primType, count, GL_UNSIGNED_INT, indexArray));
                    cleanup();
                }
            } else {
                const GLint* indexArray = indices.data();
                glAssert(glDrawElements(primType, count, GL_UNSIGNED_INT, indexArray));
            }
        }

        VertexArray::VertexArray(BaseHolder::Ptr holder) :
        m_holder(holder),
        m_prepared(false),
        m_setup(false) {}
    }
}
