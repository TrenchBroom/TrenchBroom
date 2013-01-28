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

#ifndef __TrenchBroom__AttributeArray__
#define __TrenchBroom__AttributeArray__

#include <GL/glew.h>
#include "Renderer/FaceVertex.h"
#include "Renderer/Vbo.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/String.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Attribute {
        public:
            typedef enum {
                User,
                Position,
                Normal,
                Color,
                TexCoord0,
                TexCoord1,
                TexCoord2,
                TexCoord3
            } AttributeType;

            typedef std::vector<Attribute> List;
        private:
            GLint m_size;
            GLenum m_valueType;
            AttributeType m_attributeType;
            String m_name;
        public:
            Attribute(GLint size, GLenum valueType, const String& name) :
            m_size(size),
            m_valueType(valueType),
            m_attributeType(User),
            m_name(name) {
                assert(m_size >= 0);
                assert(!Utility::trim(name).empty());
            }

            Attribute(GLint size, GLenum valueType, AttributeType attributeType) :
            m_size(size),
            m_valueType(valueType),
            m_attributeType(attributeType) {
                assert(m_size >= 0);
                assert(attributeType != User);
            }

            static const Attribute& position2f() {
                static const Attribute attr = Attribute(2, GL_FLOAT, Position);
                return attr;
            }
            
            static const Attribute& position3f() {
                static const Attribute attr = Attribute(3, GL_FLOAT, Position);
                return attr;
            }
            
            static const Attribute& normal3f() {
                static const Attribute attr = Attribute(3, GL_FLOAT, Normal);
                return attr;
            }
            
            static const Attribute& color4f() {
                static const Attribute attr = Attribute(4, GL_FLOAT, Color);
                return attr;
            }
            
            static const Attribute& texCoord02f() {
                static const Attribute attr = Attribute(2, GL_FLOAT, TexCoord0);
                return attr;
            }
            
            inline GLint size() const {
                return m_size;
            }

            inline size_t sizeInBytes() const {
                switch (m_valueType) {
                    case GL_BYTE:
                    case GL_UNSIGNED_BYTE:
                        return static_cast<size_t>(m_size) * sizeof(GLchar);
                    case GL_SHORT:
                    case GL_UNSIGNED_SHORT:
                        return static_cast<size_t>(m_size) * sizeof(GLshort);
                    case GL_INT:
                    case GL_UNSIGNED_INT:
                        return static_cast<size_t>(m_size) * sizeof(GLint);
                    case GL_FLOAT:
                        return static_cast<size_t>(m_size) * sizeof(GLfloat);
                    case GL_DOUBLE:
                        return static_cast<size_t>(m_size) * sizeof(GLdouble);
                }

                return 0;
            }

            inline GLenum valueType() const {
                return m_valueType;
            }

            inline const String& name() const {
                return m_name;
            }

            inline void setGLState(unsigned int index, unsigned int stride, unsigned int offset) {
                switch (m_attributeType) {
                    case User:
                        glEnableVertexAttribArray(index);
                        glVertexAttribPointer(index, m_size, m_valueType, true, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        break;
                    case Position:
                        glEnableClientState(GL_VERTEX_ARRAY);
                        glVertexPointer(m_size, m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        break;
                    case Normal:
                        glEnableClientState(GL_NORMAL_ARRAY);
                        glNormalPointer(m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        break;
                    case Color:
                        glEnableClientState(GL_COLOR_ARRAY);
                        glColorPointer(m_size, m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        break;
                    case TexCoord0:
                        glClientActiveTexture(GL_TEXTURE0);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        break;
                    case TexCoord1:
                        glClientActiveTexture(GL_TEXTURE1);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                    case TexCoord2:
                        glClientActiveTexture(GL_TEXTURE2);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                        glClientActiveTexture(GL_TEXTURE0);
                        break;
                    case TexCoord3:
                        glClientActiveTexture(GL_TEXTURE3);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(m_size, m_valueType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
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

        class AttributeArray {
        protected:
            VboBlock* m_block;
            Attribute::List m_attributes;

            unsigned int m_padBy;
            unsigned int m_vertexSize;
            unsigned int m_vertexCapacity;
            unsigned int m_vertexCount;

            unsigned int m_specIndex;
            unsigned int m_writeOffset;

            void init(Vbo& vbo, unsigned int padTo) {
                for (unsigned int i = 0; i < m_attributes.size(); i++)
                    m_vertexSize += m_attributes[i].sizeInBytes();
                if (padTo != 0)
                    m_padBy = (m_vertexSize / padTo + 1) * padTo - m_vertexSize;
                m_block = vbo.allocBlock(m_vertexCapacity * (m_vertexSize + m_padBy));
            }

            inline void attributesAdded(unsigned int count = 1) {
                assert(count <= 1 || m_padBy == 0);
                if (count > 1) {
                    m_vertexCount += count;
                } else {
                    m_specIndex = static_cast<unsigned int>(succ(m_specIndex, m_attributes.size()));
                    if (m_specIndex == 0) {
                        if (m_padBy > 0)
                            m_writeOffset += m_padBy;
                        m_vertexCount++;
                    }
                }
            }
        public:
            static inline GLsizei vertexSize(GLsizei size, GLsizei padTo = 16) {
                return (size / padTo + 1) * padTo - size;
            }
            
            AttributeArray(Vbo& vbo, unsigned int vertexCapacity, const Attribute& attribute1, unsigned int padTo = 16) :
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                m_attributes.push_back(attribute1);
                init(vbo, padTo);
            }

            AttributeArray(Vbo& vbo, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, unsigned int padTo = 16) :
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

            AttributeArray(Vbo& vbo, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, unsigned int padTo = 16) :
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

            AttributeArray(Vbo& vbo, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, unsigned int padTo = 16) :
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

            AttributeArray(Vbo& vbo, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, const Attribute& attribute5, unsigned int padTo = 16) :
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

            AttributeArray(Vbo& vbo, unsigned int vertexCapacity, const Attribute::List& attributes, unsigned int padTo = 16) :
            m_attributes(attributes),
            m_padBy(0),
            m_vertexSize(0),
            m_vertexCapacity(vertexCapacity),
            m_vertexCount(0),
            m_specIndex(0),
            m_writeOffset(0) {
                init(vbo, padTo);
            }

            virtual ~AttributeArray() {
                if (m_block != NULL) {
                    m_block->freeBlock();
                    m_block = NULL;
                }
            }

            inline unsigned int vertexCount() const {
                return m_vertexCount;
            }
            
            inline void addAttribute(float value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 1);

                m_writeOffset = m_block->writeFloat(value, m_writeOffset);
                attributesAdded();
            }

            inline void addAttribute(const Vec2f& value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 2);

                m_writeOffset = m_block->writeVec(value, m_writeOffset);
                attributesAdded();
            }

            inline void addAttributes(const Vec2f::List& values) {
                assert(m_vertexCount + values.size() <= m_vertexCapacity);
                assert(m_attributes.size() == 1);
                assert(m_attributes.front().valueType() == GL_FLOAT);
                assert(m_attributes.front().size() == 2);
                assert(m_padBy == 0);
                
                const unsigned char* buffer = reinterpret_cast<const unsigned char*>(&values.front());
                unsigned int length = static_cast<unsigned int>(values.size() * 2 * sizeof(float));
                m_writeOffset = m_block->writeBuffer(buffer, m_writeOffset, length);
                attributesAdded(static_cast<unsigned int>(values.size()));
            }

            inline void addAttribute(const Vec3f& value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 3);

                m_writeOffset = m_block->writeVec(value, m_writeOffset);
                attributesAdded();
            }
            
            inline void addAttributes(const Vec3f::List& values) {
                assert(m_vertexCount + values.size() <= m_vertexCapacity);
                assert(m_attributes.size() == 1);
                assert(m_attributes.front().valueType() == GL_FLOAT);
                assert(m_attributes.front().size() == 3);
                assert(m_padBy == 0);
                
                const unsigned char* buffer = reinterpret_cast<const unsigned char*>(&values.front());
                unsigned int length = static_cast<unsigned int>(values.size() * 3 * sizeof(float));
                m_writeOffset = m_block->writeBuffer(buffer, m_writeOffset, length);
                attributesAdded(static_cast<unsigned int>(values.size()));
            }

            inline void addAttribute(const Vec4f& value) {
                assert(m_vertexCount < m_vertexCapacity);
                assert(m_attributes[m_specIndex].valueType() == GL_FLOAT);
                assert(m_attributes[m_specIndex].size() == 4);

                m_writeOffset = m_block->writeVec(value, m_writeOffset);
                attributesAdded();
            }

            inline void addAttributes(const FaceVertex::List& cachedVertices) {
                assert(m_attributes[0].valueType() == GL_FLOAT);
                assert(m_attributes[0].size() == 3);
                assert(m_attributes[1].valueType() == GL_FLOAT);
                assert(m_attributes[1].size() == 3);
                assert(m_attributes[2].valueType() == GL_FLOAT);
                assert(m_attributes[2].size() == 2);
                assert(m_padBy == 0);
                assert(m_vertexCount + cachedVertices.size() <= m_vertexCapacity);
                
                m_writeOffset = m_block->writeBuffer(reinterpret_cast<const unsigned char*>(&cachedVertices.front()), m_writeOffset, static_cast<unsigned int>(cachedVertices.size() * sizeof(FaceVertex)));
                attributesAdded(static_cast<unsigned int>(cachedVertices.size()));
            }
            
            inline void bindAttributes(const ShaderProgram& program) {
                for (unsigned int i = 0; i < m_attributes.size(); i++) {
                    Attribute& attribute = m_attributes[i];
                    attribute.bindAttribute(i, program.programId());
                }
            }

            inline void reset() {
                m_vertexCount = 0;
                m_specIndex = 0;
                m_writeOffset = 0;
            }
            
            inline void setup() {
                assert(m_specIndex == 0);
                
                unsigned int offset = m_block->address();
                for (unsigned int i = 0; i < m_attributes.size(); i++) {
                    Attribute& attribute = m_attributes[i];
                    attribute.setGLState(i, m_vertexSize + m_padBy, offset);
                    offset += attribute.sizeInBytes();
                }
            }
            
            inline void cleanup() {
                for (unsigned int i = 0; i < m_attributes.size(); i++) {
                    Attribute& attribute = m_attributes[i];
                    attribute.clearGLState(i);
                }
            }
        };
        
        class RenderArray : public AttributeArray {
        protected:
            GLenum m_primType;
        public:
            RenderArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, unsigned int padTo = 16) :
            AttributeArray(vbo, vertexCapacity, attribute1, padTo),
            m_primType(primType) {}
            
            RenderArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, unsigned int padTo = 16) :
            AttributeArray(vbo, vertexCapacity, attribute1, attribute2, padTo),
            m_primType(primType) {}
            
            RenderArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, unsigned int padTo = 16) :
            AttributeArray(vbo, vertexCapacity, attribute1, attribute2, attribute3, padTo),
            m_primType(primType) {}
            
            RenderArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, unsigned int padTo = 16) :
            AttributeArray(vbo, vertexCapacity, attribute1, attribute2, attribute3, attribute4, padTo),
            m_primType(primType) {}
            
            RenderArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute& attribute1, const Attribute& attribute2, const Attribute& attribute3, const Attribute& attribute4, const Attribute& attribute5, unsigned int padTo = 16) :
            AttributeArray(vbo, vertexCapacity, attribute1, attribute2, attribute3, attribute4, attribute5, padTo),
            m_primType(primType) {}
            
            RenderArray(Vbo& vbo, GLenum primType, unsigned int vertexCapacity, const Attribute::List& attributes, unsigned int padTo = 16) :
            AttributeArray(vbo, vertexCapacity, attributes, padTo),
            m_primType(primType) {}
            
            virtual ~RenderArray() {}
        };
    }
}

#endif /* defined(__TrenchBroom__AttributeArray__) */
