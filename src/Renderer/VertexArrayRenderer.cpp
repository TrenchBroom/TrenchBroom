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

#include "VertexArrayRenderer.h"

#include <cassert>
#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        AttributeSpec::AttributeSpec(const AttributeType attributeType, const GLenum dataType, const GLint size) :
        m_attributeType(attributeType),
        m_dataType(dataType),
        m_size(size) {}
        
        const AttributeSpec& AttributeSpec::P2() {
            static const AttributeSpec attr(Position, GL_FLOAT, 2);
            return attr;
        }
        
        const AttributeSpec& AttributeSpec::P3() {
            static const AttributeSpec attr(Position, GL_FLOAT, 3);
            return attr;
        }
        
        const AttributeSpec& AttributeSpec::N3() {
            static const AttributeSpec attr(Normal, GL_FLOAT, 3);
            return attr;
        }
        
        const AttributeSpec& AttributeSpec::C4() {
            static const AttributeSpec attr(Color, GL_FLOAT, 4);
            return attr;
        }
        
        const AttributeSpec& AttributeSpec::T02() {
            static const AttributeSpec attr(TexCoord0, GL_FLOAT, 2);
            return attr;
        }
        
        AttributeSpec::AttributeType AttributeSpec::attributeType() const {
            return m_attributeType;
        }
        
        GLenum AttributeSpec::dataType() const {
            return m_dataType;
        }
        
        GLint AttributeSpec::size() const {
            return m_size;
        }
        
        size_t AttributeSpec::sizeInBytes() const {
            switch (m_dataType) {
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

        void AttributeSpec::setup(const size_t index, const size_t stride, const size_t offset) const {
            switch (m_attributeType) {
                case User:
                    glEnableVertexAttribArray(static_cast<GLuint>(index));
                    glVertexAttribPointer(static_cast<GLuint>(index), m_size, m_dataType, true, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    break;
                case Position:
                    glEnableClientState(GL_VERTEX_ARRAY);
                    glVertexPointer(m_size, m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    break;
                case Normal:
                    assert(m_size == 3);
                    glEnableClientState(GL_NORMAL_ARRAY);
                    glNormalPointer(m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    break;
                case Color:
                    glEnableClientState(GL_COLOR_ARRAY);
                    glColorPointer(m_size, m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    break;
                case TexCoord0:
                    glClientActiveTexture(GL_TEXTURE0);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(m_size, m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    break;
                case TexCoord1:
                    glClientActiveTexture(GL_TEXTURE1);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(m_size, m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    glClientActiveTexture(GL_TEXTURE0);
                    break;
                case TexCoord2:
                    glClientActiveTexture(GL_TEXTURE2);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(m_size, m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    glClientActiveTexture(GL_TEXTURE0);
                    break;
                case TexCoord3:
                    glClientActiveTexture(GL_TEXTURE3);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(m_size, m_dataType, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
                    glClientActiveTexture(GL_TEXTURE0);
                    break;
            }
        }
        
        void AttributeSpec::cleanup(const size_t index) const {
            switch (m_attributeType) {
                case User:
                    glDisableVertexAttribArray(static_cast<GLuint>(index));
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
        
        VertexSpec::VertexSpec() :
        m_totalSize(0) {}

        VertexSpec::VertexSpec(const AttributeSpec& attributeSpec1) :
        m_totalSize(0) {
            m_attributeSpecs.push_back(attributeSpec1);
            m_totalSize += attributeSpec1.sizeInBytes();
        }
        
        VertexSpec::VertexSpec(const AttributeSpec& attributeSpec1, const AttributeSpec& attributeSpec2) :
        m_totalSize(0) {
            m_attributeSpecs.push_back(attributeSpec1);
            m_attributeSpecs.push_back(attributeSpec2);
            m_totalSize += attributeSpec1.sizeInBytes();
            m_totalSize += attributeSpec2.sizeInBytes();
        }
        
        VertexSpec::VertexSpec(const AttributeSpec& attributeSpec1, const AttributeSpec& attributeSpec2, const AttributeSpec& attributeSpec3) :
        m_totalSize(0) {
            m_attributeSpecs.push_back(attributeSpec1);
            m_attributeSpecs.push_back(attributeSpec2);
            m_attributeSpecs.push_back(attributeSpec3);
            m_totalSize += attributeSpec1.sizeInBytes();
            m_totalSize += attributeSpec2.sizeInBytes();
            m_totalSize += attributeSpec3.sizeInBytes();
        }
        
        VertexSpec::VertexSpec(const AttributeSpec& attributeSpec1, const AttributeSpec& attributeSpec2, const AttributeSpec& attributeSpec3, const AttributeSpec& attributeSpec4) :
        m_totalSize(0) {
            m_attributeSpecs.push_back(attributeSpec1);
            m_attributeSpecs.push_back(attributeSpec2);
            m_attributeSpecs.push_back(attributeSpec3);
            m_attributeSpecs.push_back(attributeSpec4);
            m_totalSize += attributeSpec1.sizeInBytes();
            m_totalSize += attributeSpec2.sizeInBytes();
            m_totalSize += attributeSpec3.sizeInBytes();
            m_totalSize += attributeSpec4.sizeInBytes();
        }
        
        const VertexSpec VertexSpec::P3() {
            return VertexSpec(AttributeSpec::P3());
        }
        
        const VertexSpec VertexSpec::P3C4() {
            return VertexSpec(AttributeSpec::P3(), AttributeSpec::C4());
        }
        
        const VertexSpec VertexSpec::P3T2() {
            return VertexSpec(AttributeSpec::P3(), AttributeSpec::T02());
        }
        
        const VertexSpec VertexSpec::P3N3T2() {
            return VertexSpec(AttributeSpec::P3(), AttributeSpec::N3(), AttributeSpec::T02());
        }
        
        const AttributeSpec& VertexSpec::operator[] (const size_t index) const {
            assert(index < m_attributeSpecs.size());
            return m_attributeSpecs[index];
        }
        
        size_t VertexSpec::size() const {
            return m_attributeSpecs.size();
        }
        
        void VertexSpec::setup(const size_t baseOffset) const {
            size_t offset = baseOffset;
            for (size_t i = 0; i < m_attributeSpecs.size(); i++) {
                const AttributeSpec& spec = m_attributeSpecs[i];
                spec.setup(i, m_totalSize, offset);
                offset += spec.sizeInBytes();
            }
        }
        
        void VertexSpec::cleanup() const {
            for (size_t i = 0; i < m_attributeSpecs.size(); i++) {
                const AttributeSpec& spec = m_attributeSpecs[i];
                spec.cleanup(i);
            }
        }

        VertexArrayRenderer::VertexArrayRenderer(const VertexSpec& vertexSpec, const GLenum primType) :
        m_vertexSpec(vertexSpec),
        m_primType(primType) {}

        VertexArrayRenderer::VertexArrayRenderer(const VertexSpec& vertexSpec, const GLenum primType, VertexArray& vertexArray) :
        m_vertexSpec(vertexSpec),
        m_primType(primType),
        m_vertexArray(vertexArray) {}
        
        VertexArrayRenderer::VertexArrayRenderer(const VertexSpec& vertexSpec, const GLenum primType, VertexArray& vertexArray, const IndexArray& indices, const CountArray& counts) :
        m_vertexSpec(vertexSpec),
        m_primType(primType),
        m_vertexArray(vertexArray),
        m_indices(indices),
        m_counts(counts) {
            assert(m_indices == m_counts);
        }
        
        VertexArrayRenderer::VertexArrayRenderer(VertexArrayRenderer& other) {
            using std::swap;
            swap(*this, other);
        }
        
        VertexArrayRenderer& VertexArrayRenderer::operator= (VertexArrayRenderer other) {
            swap(*this, other);
            return *this;
        }

        void VertexArrayRenderer::render() {
            assert(m_indices == m_counts);
            if (m_vertexArray.vertexCount() == 0)
                return;

            m_vertexArray.prepare();
            m_vertexSpec.setup(m_vertexArray.blockOffset());
            
            const size_t primCount = m_indices.size();
            if (primCount <= 1) {
                glDrawArrays(m_primType, 0, static_cast<GLsizei>(m_vertexArray.vertexCount()));
            } else {
                const GLint* indexArray = &m_indices[0];
                const GLsizei* countArray = &m_counts[0];
                glMultiDrawArrays(m_primType, indexArray, countArray, static_cast<GLint>(primCount));
            }
            
            m_vertexSpec.cleanup();
        }

    }
}
