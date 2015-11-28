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

#include <algorithm>
#include <cassert>
#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        VertexArray::VertexArray() :
        m_prepared(false),
        m_setup(false) {}
        
        VertexArray& VertexArray::operator= (VertexArray other) {
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

        size_t VertexArray::size() const {
            return m_holder == NULL ? 0 : m_holder->size();
        }

        size_t VertexArray::vertexCount() const {
            return m_holder == NULL ? 0 : m_holder->vertexCount();
        }

        bool VertexArray::prepared() const {
            return m_prepared;
        }

        void VertexArray::prepare(Vbo& vbo) {
            if (!m_prepared && m_holder != NULL && m_holder->vertexCount() > 0)
                m_holder->prepare(vbo);
            m_prepared = true;
        }

        bool VertexArray::setup() {
            assert(m_prepared);
            assert(!m_setup);
            
            if (m_holder == NULL || m_holder->vertexCount() > 0)
                return false;
            
            m_holder->setup();
            m_setup = true;
            return true;
        }
        
        void VertexArray::cleanup() {
            assert(m_setup);
            assert(m_holder != NULL && m_holder->vertexCount() > 0);
            m_holder->cleanup();
            m_setup = false;
        }

        void VertexArray::render(const GLenum primType) const {
            render(primType, 0, static_cast<GLsizei>(vertexCount()));
        }

        void VertexArray::render(const GLenum primType, const GLint index, const GLsizei count) const {
            assert(m_prepared);
            assert(m_setup);

            glDrawArrays(primType, index, count);
        }

        void VertexArray::render(const GLenum primType, const IndexArray& indices, const CountArray& counts, const GLint primCount) const {
            assert(m_prepared);
            assert(m_setup);
            
            const GLint* indexArray   = indices.data();
            const GLsizei* countArray = counts.data();
            glMultiDrawArrays(primType, indexArray, countArray, primCount);
        }

        void VertexArray::render(GLenum primType, const IndexArray& indices, const GLsizei count) const {
            assert(m_prepared);
            assert(m_setup);
            
            const GLint* indexArray = indices.data();
            glDrawElements(primType, count, GL_UNSIGNED_INT, indexArray);
        }

        VertexArray::VertexArray(BaseHolder::Ptr holder) :
        m_holder(holder),
        m_prepared(false),
        m_setup(false) {}
    }
}
