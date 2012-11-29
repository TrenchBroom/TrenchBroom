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

#ifndef __TrenchBroom__VertexArray__
#define __TrenchBroom__VertexArray__

#include "Renderer/AttributeArray.h"

#include <GL/glew.h>
#include "Renderer/Vbo.h"
#include "Renderer/Shader/Shader.h"
#include "Utility/String.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class VertexArray : public RenderArray {
        protected:
        public:
            VertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, padTo) {}
            
            VertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, padTo) {}
            
            VertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, padTo) {}
            
            VertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, attribute4, padTo) {}
            
            VertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, const Attribute& attribute5, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, attribute4, attribute5, padTo) {}
            
            VertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute::List& attributes, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attributes, padTo) {}
            
            inline void renderPrimitives(unsigned int index, unsigned int vertexCount) {
                glDrawArrays(m_primType, index, vertexCount);
            }

            inline void render() {
                setup();
                glDrawArrays(m_primType, 0, m_vertexCount);
                cleanup();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__VertexArray__) */
