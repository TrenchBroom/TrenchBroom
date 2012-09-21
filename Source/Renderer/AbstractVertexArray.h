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

#ifndef __TrenchBroom__AbstractVertexArray__
#define __TrenchBroom__AbstractVertexArray__

#include <GL/glew.h>
#include "Renderer/Vbo.h"
#include "Renderer/Shader/Shader.h"
#include "Utility/String.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class VertexAttribute {
        public:
            typedef enum {
                User,
                Position,
                Normal,
                Color,
                TexCoord0,
                TexCoord1,
                TexCoord2,
                TexCoord3,
            } AttributeType;

            typedef std::vector<VertexAttribute> List;
        private:
            GLint m_size;
            GLenum m_valueType;
            AttributeType m_attributeType;
            String m_name;
        public:
            VertexAttribute(GLint size, GLenum valueType, const String& name) :
            m_size(size),
            m_valueType(valueType),
            m_attributeType(User),
            m_name(name) {
                assert(!Utility::trim(name).empty());
            }

            VertexAttribute(GLint size, GLenum valueType, AttributeType attributeType) :
            m_size(size),
            m_valueType(valueType),
            m_attributeType(attributeType) {
                assert(attributeType != User);
            }

            inline GLint size() const {
                return m_size;
            }

            inline GLsizei sizeInBytes() const {
                switch (m_valueType) {
                    case GL_BYTE:
                    case GL_UNSIGNED_BYTE:
                        return m_size * sizeof(GLchar);
                    case GL_SHORT:
                    case GL_UNSIGNED_SHORT:
                        return m_size * sizeof(GLshort);
                    case GL_INT:
                    case GL_UNSIGNED_INT:
                        return m_size * sizeof(GLint);
                    case GL_FLOAT:
                        return m_size * sizeof(GLfloat);
                    case GL_DOUBLE:
                        return m_size * sizeof(GLdouble);
                }

                return 0;
            }

            inline GLenum valueType() const {
                return m_valueType;
            }

            inline const String& name() const {
                return m_name;
            }

            inline void setGLState(GLuint index, GLsizei stride, GLsizei offset) {
                switch (m_attributeType) {
                    case User:
                        glEnableVertexAttribArray(index);
                        glVertexAttribPointer(index, m_size, m_valueType, true, stride, reinterpret_cast<GLvoid*>(offset));
                        break;
                    case Position:
                        glEnableClientState(GL_VERTEX_ARRAY);
                        glVertexPointer(m_size, m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        break;
                    case Normal:
                        glEnableClientState(GL_NORMAL_ARRAY);
                        glNormalPointer(m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        break;
                    case Color:
                        glEnableClientState(GL_COLOR_ARRAY);
                        glColorPointer(m_size, m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        break;
                    case TexCoord0:
                        glClientActiveTexture(GL_TEXTURE0);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        break;
                    case TexCoord1:
                        glClientActiveTexture(GL_TEXTURE1);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                    case TexCoord2:
                        glClientActiveTexture(GL_TEXTURE2);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                    case TexCoord3:
                        glClientActiveTexture(GL_TEXTURE3);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, stride, reinterpret_cast<GLvoid*>(offset));
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                }
            }

            inline void bindAttribute(GLuint index, GLuint programId) {
                if (m_attributeType == User)
                    glBindAttribLocation(programId, index, m_name.c_str());
            }

            inline void clearGLState(GLuint index) {
                switch (m_attributeType) {
                    case User:
                        glDisableVertexAttribArray(index);
                        break;
                    case Position:
                        glDisableClientState(GL_VERTEX_ARRAY);
                        break;
                    case Normal:
                        glDisableClientState(GL_NORMAL_ARRAY);
                        break;
                    case Color:
                        glDisableClientState(GL_COLOR_ARRAY);
                        break;
                    case TexCoord0:
                        glClientActiveTexture(GL_TEXTURE0);
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        break;
                    case TexCoord1:
                        glClientActiveTexture(GL_TEXTURE1);
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                    case TexCoord2:
                        glClientActiveTexture(GL_TEXTURE2);
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                    case TexCoord3:
                        glClientActiveTexture(GL_TEXTURE3);
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                }
            }
        };

        class AbstractVertexArray {
        protected:
            VboBlock* m_block;
            GLenum m_primType;
            VertexAttribute::List m_attributes;

            GLsizei m_padBy;
            GLsizei m_vertexSize;
            unsigned int m_vertexCapacity;
            unsigned int m_vertexCount;

            unsigned int m_specIndex;
            unsigned int m_writeOffset;

            void init(Vbo& vbo, GLsizei padTo) {
                for (unsigned int i = 0; i < m_attributes.size(); i++)
                    m_vertexSize += m_attributes[i].sizeInBytes();
                if (padTo != 0)
                    m_padBy = (m_vertexSize / padTo + 1) * padTo - m_vertexSize;
                m_block = vbo.allocBlock(m_vertexCapacity * (m_vertexSize + m_padBy));
            }

            inline void attributeAdded() {
                m_specIndex = static_cast<unsigned int>(succ(m_specIndex, m_attributes.size()));
                if (m_specIndex == 0) {
                    if (m_padBy > 0)
                        m_writeOffset += m_padBy;
                    m_vertexCount++;
                }
            }

            virtual void doRender() = 0;
        public:
            AbstractVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, GLsizei padTo = 16) :
            m_primType(primType),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                m_attributes.push_back(attribute1);
                init(vbo, padTo);
            }

            AbstractVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, GLsizei padTo = 16) :
            m_primType(primType),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                m_attributes.push_back(attribute1);
                m_attributes.push_back(attribute2);
                init(vbo, padTo);
            }

            AbstractVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, const VertexAttribute& attribute3, GLsizei padTo = 16) :
            m_primType(primType),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                m_attributes.push_back(attribute1);
                m_attributes.push_back(attribute2);
                m_attributes.push_back(attribute3);
                init(vbo, padTo);
            }

            AbstractVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, const VertexAttribute& attribute3, const VertexAttribute& attribute4, GLsizei padTo = 16) :
            m_primType(primType),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                m_attributes.push_back(attribute1);
                m_attributes.push_back(attribute2);
                m_attributes.push_back(attribute3);
                m_attributes.push_back(attribute4);
                init(vbo, padTo);
            }

            AbstractVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute& attribute1, const VertexAttribute& attribute2, const VertexAttribute& attribute3, const VertexAttribute& attribute4, const VertexAttribute& attribute5, GLsizei padTo = 16) :
            m_primType(primType),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                m_attributes.push_back(attribute1);
                m_attributes.push_back(attribute2);
                m_attributes.push_back(attribute3);
                m_attributes.push_back(attribute4);
                m_attributes.push_back(attribute5);
                init(vbo, padTo);
            }

            AbstractVertexArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const VertexAttribute::List& attributes, GLsizei padTo = 16) :
            m_primType(primType),
            m_attributes(attributes),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                init(vbo, padTo);
            }

            virtual ~AbstractVertexArray() {
                if (m_block != NULL) {
                    m_block->freeBlock();
                    m_block = NULL;
                }
            }

            inline void addAttribute(float value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 1);

                m_writeOffset = m_block->writeFloat(value, m_writeOffset);
                attributeAdded();
            }

            inline void addAttribute(const Vec2f& value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 2);

                m_writeOffset = m_block->writeVec(value, m_writeOffset);
                attributeAdded();
            }

            inline void addAttribute(const Vec3f& value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 3);

                m_writeOffset = m_block->writeVec(value, m_writeOffset);
                attributeAdded();
            }

            inline void addAttribute(const Vec4f& value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 4);

                m_writeOffset = m_block->writeVec(value, m_writeOffset);
                attributeAdded();
            }

            inline void bindAttributes(const ShaderProgram& program) {
                for (unsigned int i = 0; i < m_attributes.size(); i++) {
                    VertexAttribute& attribute = m_attributes[i];
                    attribute.bindAttribute(i, program.programId());
                }
            }

            inline void preRender() {
                assert(m_specIndex == 0);
                
                unsigned int offset = m_block->address();
                for (unsigned int i = 0; i < m_attributes.size(); i++) {
                    VertexAttribute& attribute = m_attributes[i];
                    attribute.setGLState(i, m_vertexSize + m_padBy, offset);
                    offset += attribute.sizeInBytes();
                }
            }
            
            inline void postRender() {
                for (unsigned int i = 0; i < m_attributes.size(); i++) {
                    VertexAttribute& attribute = m_attributes[i];
                    attribute.clearGLState(i);
                }
            }
            
            inline void render() {
                preRender();
                doRender();
                postRender();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__AbstractVertexArray__) */
