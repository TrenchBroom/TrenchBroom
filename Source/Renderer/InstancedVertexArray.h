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

#ifndef TrenchBroom_InstancedVertexArray_h
#define TrenchBroom_InstancedVertexArray_h

#include "Renderer/AttributeArray.h"
#include "Renderer/RenderTypes.h"
#include "Utility/String.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Attribute;
        class ShaderProgram;
        class Vbo;
        
        class InstanceAttributes {
        private:
            String m_name;
            String m_textureSizeName;
            GLuint m_textureId;
            GLint m_textureSize;
        protected:
            virtual GLint createTexture(GLuint textureId) = 0;
        public:
            InstanceAttributes(const String& name) :
            m_name(name),
            m_textureId(0) {
                StringStream stream;
                stream << m_name << "Size";
                m_textureSizeName = stream.str();
            }
            
            virtual ~InstanceAttributes() {
                if (m_textureId > 0) {
                    glDeleteTextures(1, &m_textureId);
                    m_textureId = 0;
                }
            }

            inline const String& name() const {
                return m_name;
            }
            
            inline const String& textureSizeName() const {
                return m_textureSizeName;
            }
            
            inline GLint textureSize() const {
                return m_textureSize;
            }
            
            inline void setup() {
                if (m_textureId == 0) {
                    glGenTextures(1, &m_textureId);
                    assert(m_textureId > 0);
                    glBindTexture(GL_TEXTURE_2D, m_textureId);
                    m_textureSize = createTexture(m_textureId);
                } else {
                    glBindTexture(GL_TEXTURE_2D, m_textureId);
                }
            }
            
            inline void cleanup() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        };
        
        class InstanceAttributesVec4f : public InstanceAttributes {
        private:
            Vec4f::List m_vertices;
        protected:
            GLint createTexture(GLuint textureId) {
                GLint size = 1;
                while (size * size < m_vertices.size())
                    size *= 2;

                unsigned char* buffer = new unsigned char[m_vertices.size() * 4 * 4];
                memcpy(buffer, reinterpret_cast<const unsigned char*>(&m_vertices.front()), m_vertices.size() * 4 * 4);
                
                // requires GL_ARB_texture_float, see http://www.opengl.org/wiki/Floating_point_and_mipmapping_and_filtering
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<GLvoid*>(buffer));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                delete [] buffer;
                m_vertices.clear();
                
                return size;
            }
        public:
            InstanceAttributesVec4f(const String& name, const Vec4f::List& vertices) :
            InstanceAttributes(name),
            m_vertices(vertices) {}
        };
        
        class InstancedVertexArray : public RenderArray {
        protected:
            typedef std::vector<InstanceAttributes*> InstanceAttributesList;
            InstanceAttributesList m_instanceAttributes;
            unsigned int m_instanceCount;
        public:
            InstancedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, unsigned int instanceCount, const Attribute& attribute1, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, padTo),
            m_instanceCount(instanceCount) {}
            
            InstancedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, unsigned int instanceCount, const Attribute& attribute1, const Attribute& attribute2, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, padTo),
            m_instanceCount(instanceCount) {}
            
            InstancedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, unsigned int instanceCount, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, padTo),
            m_instanceCount(instanceCount) {}
            
            InstancedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, unsigned int instanceCount, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, attribute4, padTo),
            m_instanceCount(instanceCount) {}
            
            InstancedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, unsigned int instanceCount, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, const Attribute& attribute5, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attribute1, attribute2, attribute3, attribute4, attribute5, padTo),
            m_instanceCount(instanceCount) {}
            
            InstancedVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, unsigned int instanceCount, const Attribute::List& attributes, GLsizei padTo = 16) :
            RenderArray(vbo, primType, vertexCapacity, attributes, padTo),
            m_instanceCount(instanceCount) {}
            
            ~InstancedVertexArray() {
                while (!m_instanceAttributes.empty()) delete m_instanceAttributes.back(), m_instanceAttributes.pop_back();
            }
            
            inline void addAttributeArray(const String& name, const Vec4f::List& values) {
                assert(values.size() == m_instanceCount);
                m_instanceAttributes.push_back(new InstanceAttributesVec4f(name, values));
            }
            
            inline void render(ShaderProgram& program) {
                bindAttributes(program);
                setup();
                
                int textureNum = 0;
                InstanceAttributesList::const_iterator it, end;
                for (it = m_instanceAttributes.begin(), end = m_instanceAttributes.end(); it != end; ++it) {
                    InstanceAttributes& attributes = **it;
                    glActiveTexture(GL_TEXTURE0 + textureNum);
                    attributes.setup();
                    program.setUniformVariable(attributes.name(), textureNum);
                    program.setUniformVariable(attributes.textureSizeName(), attributes.textureSize());
                    textureNum++;
                }
                
                glDrawArraysInstancedARB(m_primType, 0, m_vertexCount, m_instanceCount);
                
                textureNum = GL_TEXTURE0;
                for (it = m_instanceAttributes.begin(), end = m_instanceAttributes.end(); it != end; ++it) {
                    InstanceAttributes& attributes = **it;
                    glActiveTexture(textureNum++);
                    attributes.cleanup();
                }
                
                cleanup();
            }
        };
    }
}

#endif
