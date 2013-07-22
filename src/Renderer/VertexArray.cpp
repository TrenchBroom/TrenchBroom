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

#include "VertexArray.h"

#include <cassert>
#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        size_t VertexArray::vertexCount() const {
            return m_vertexCount;
        }

        void VertexArray::prepare() {
            m_holder->setup();
        }

        void VertexArray::render() {
            if (m_holder == NULL || m_holder->size() == 0)
                return;

            m_holder->setup();
            const size_t primCount = m_indices.size();
            if (primCount <= 1) {
                glDrawArrays(m_primType, 0, static_cast<GLsizei>(m_holder->size()));
            } else {
                const GLint* indexArray = &m_indices[0];
                const GLsizei* countArray = &m_counts[0];
                glMultiDrawArrays(m_primType, indexArray, countArray, static_cast<GLint>(primCount));
            }
            m_holder->cleanup();
        }

    }
}
