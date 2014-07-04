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
        const VertexArray::IndexArray VertexArray::EmptyIndexArray(0);
        const VertexArray::CountArray VertexArray::EmptyCountArray(0);

        VertexArray::VertexArray() :
        m_primType(GL_INVALID_ENUM),
        m_prepared(false) {}
        
        VertexArray& VertexArray::operator= (VertexArray other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(VertexArray& left, VertexArray& right) {
            using std::swap;
            swap(left.m_primType, right.m_primType);
            swap(left.m_holder, right.m_holder);
            swap(left.m_indices, right.m_indices);
            swap(left.m_counts, right.m_counts);
            swap(left.m_prepared, right.m_prepared);
        }

        size_t VertexArray::size() const {
            return m_holder.get() == NULL ? 0 : m_holder->size();
        }

        size_t VertexArray::vertexCount() const {
            return m_holder.get() == NULL ? 0 : m_holder->vertexCount();
        }

        bool VertexArray::prepared() const {
            return m_prepared;
        }

        void VertexArray::prepare(Vbo& vbo) {
            if (!m_prepared && m_holder.get() != NULL && m_holder->vertexCount() > 0)
                m_holder->prepare(vbo);
            m_prepared = true;
        }

        void VertexArray::render() {
            assert(m_prepared);
            
            if (m_holder.get() == NULL || m_holder->vertexCount() == 0)
                return;

            m_holder->setup();
            const size_t primCount = m_indices.size();
            if (primCount <= 1) {
                glDrawArrays(m_primType, 0, static_cast<GLsizei>(m_holder->vertexCount()));
            } else {
                const GLint* indexArray = &m_indices[0];
                const GLsizei* countArray = &m_counts[0];
                glMultiDrawArrays(m_primType, indexArray, countArray, static_cast<GLint>(primCount));
            }
            m_holder->cleanup();
        }

        VertexArray::VertexArray(const GLenum primType, BaseHolder::Ptr holder, const IndexArray& indices, const CountArray& counts) :
        m_primType(primType),
        m_holder(holder),
        m_indices(indices),
        m_counts(counts),
        m_prepared(false) {
            assert(m_indices.size() == m_counts.size());
        }
        
        VertexArray::VertexArray(const GLenum primType, BaseHolder::Ptr holder, IndexArray& indices, CountArray& counts) :
        m_primType(primType),
        m_holder(holder),
        m_prepared(false) {
            using std::swap;
            swap(m_indices, indices);
            swap(m_counts, counts);
            assert(m_indices.size() == m_counts.size());
        }
    }
}
