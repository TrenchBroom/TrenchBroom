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

#ifndef __TrenchBroom__IndexedVertexArray__
#define __TrenchBroom__IndexedVertexArray__

#include "Renderer/AbstractVertexArray.h"

#include "Renderer/Vbo.h"
#include "Renderer/Shader/Shader.h"
#include "Utility/GLee.h"
#include "Utility/String.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class IndexedVertexArray : public AbstractVertexArray {
        private:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
            
            IndexArray m_primIndices;
            IndexArray m_primVertexCounts;
            unsigned int m_currentPrimIndex;
            unsigned int m_primCount;
            
            inline void doRender() {
                if (m_primCount == 0)
                    return;
                
                assert(m_primCount == m_primIndices.size());
                assert(m_primCount == m_primVertexCounts.size());
                
                GLint* indexArray = &m_primIndices[0];
                GLsizei* countArray = &m_primVertexCounts[0];
                
                glMultiDrawArrays(m_primType, indexArray, countArray, m_primCount);
            }
        public:
            IndexedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, GLsizei padTo = 16) :
            AbstractVertexArray(vbo, primType, vertexCapacity, attribute1, padTo),
            m_currentPrimIndex(0),
            m_primCount(0) {}
            
            IndexedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, GLsizei padTo = 16) :
            AbstractVertexArray(vbo, primType, vertexCapacity, attribute1, attribute2, padTo),
            m_currentPrimIndex(0),
            m_primCount(0) {}
            
            IndexedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, const VertexAttribute& attribute3, GLsizei padTo = 16) :
            AbstractVertexArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, padTo),
            m_currentPrimIndex(0),
            m_primCount(0) {}
            
            IndexedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, const VertexAttribute& attribute3, const VertexAttribute& attribute4, GLsizei padTo = 16) :
            AbstractVertexArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, attribute4, padTo),
            m_currentPrimIndex(0),
            m_primCount(0) {}
            
            IndexedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, const VertexAttribute& attribute3, const VertexAttribute& attribute4, const VertexAttribute& attribute5, GLsizei padTo = 16) :
            AbstractVertexArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, attribute4, attribute5, padTo),
            m_currentPrimIndex(0),
            m_primCount(0) {}
            
            IndexedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute::List& attributes, GLsizei padTo = 16) :
            AbstractVertexArray(vbo, primType, vertexCapacity, attributes, padTo),
            m_currentPrimIndex(0),
            m_primCount(0) {}

            inline void endPrimitive() {
                assert(m_specIndex == 0);
                
                if (m_currentPrimIndex < m_vertexCount) {
                    GLsizei primVertexCount = static_cast<GLsizei>(m_vertexCount - m_currentPrimIndex);
                    m_primIndices.push_back(m_currentPrimIndex);
                    m_primVertexCounts.push_back(primVertexCount);
                    m_primCount++;
                    m_currentPrimIndex = m_vertexCount;
                }
                
            }
        };
    }
}

#endif /* defined(__TrenchBroom__IndexedVertexArray__) */
