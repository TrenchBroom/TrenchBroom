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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_AttributeSpec_h
#define TrenchBroom_AttributeSpec_h

namespace TrenchBroom {
    namespace Renderer {
        template <GLenum T>
        struct DataTypeTraits {
            typedef GLvoid Type;
        };
        
        template <>
        struct DataTypeTraits<GL_BYTE> {
            typedef GLbyte Type;
        };
        
        template <>
        struct DataTypeTraits<GL_UNSIGNED_BYTE> {
            typedef GLubyte Type;
        };
        
        template <>
        struct DataTypeTraits<GL_SHORT> {
            typedef GLshort Type;
        };
        
        template <>
        struct DataTypeTraits<GL_UNSIGNED_SHORT> {
            typedef GLushort Type;
        };
        
        template <>
        struct DataTypeTraits<GL_INT> {
            typedef GLint Type;
        };
        
        template <>
        struct DataTypeTraits<GL_UNSIGNED_INT> {
            typedef GLuint Type;
        };
        
        template <>
        struct DataTypeTraits<GL_FLOAT> {
            typedef GLfloat Type;
        };
        
        template <>
        struct DataTypeTraits<GL_DOUBLE> {
            typedef GLdouble Type;
        };
        
        typedef enum {
            ATUser,
            ATPosition,
            ATNormal,
            ATColor,
            ATTexCoord0,
            ATTexCoord1,
            ATTexCoord2,
            ATTexCoord3
        } AttributeType;
        
        template <AttributeType type, GLenum D, size_t S>
        class AttributeSpec {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {}
            static void cleanup(const size_t index) {}
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATUser, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glEnableVertexAttribArray(static_cast<GLuint>(index));
                glVertexAttribPointer(static_cast<GLuint>(index), S, D, true, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glDisableVertexAttribArray(static_cast<GLuint>(index));
            }
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATPosition, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(S, D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glDisableClientState(GL_VERTEX_ARRAY);
            }
        };
        
        template <GLenum D, const size_t S>
        class AttributeSpec<ATNormal, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                assert(S == 3);
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glDisableClientState(GL_NORMAL_ARRAY);
            }
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATColor, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(S, D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glDisableClientState(GL_COLOR_ARRAY);
            }
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATTexCoord0, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glClientActiveTexture(GL_TEXTURE0);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(S, D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glClientActiveTexture(GL_TEXTURE0);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATTexCoord1, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glClientActiveTexture(GL_TEXTURE1);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(S, D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glClientActiveTexture(GL_TEXTURE1);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glClientActiveTexture(GL_TEXTURE0);
            }
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATTexCoord2, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glClientActiveTexture(GL_TEXTURE2);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(S, D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glClientActiveTexture(GL_TEXTURE2);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glClientActiveTexture(GL_TEXTURE0);
            }
        };
        
        template <GLenum D, size_t S>
        class AttributeSpec<ATTexCoord3, D, S> {
        public:
            typedef typename DataTypeTraits<D>::Type DataType;
            typedef Vec<DataType, S> ElementType;
            static const size_t Size = sizeof(DataType) * S;
            
            static void setup(const size_t index, const size_t stride, const size_t offset) {
                glClientActiveTexture(GL_TEXTURE3);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(S, D, static_cast<GLsizei>(stride), reinterpret_cast<GLvoid*>(offset));
            }
            
            static void cleanup(const size_t index) {
                glClientActiveTexture(GL_TEXTURE3);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glClientActiveTexture(GL_TEXTURE0);
            }
        };
        
        namespace AttributeSpecs {
            typedef AttributeSpec<ATPosition, GL_FLOAT, 2> P2;
            typedef AttributeSpec<ATPosition, GL_FLOAT, 3> P3;
            typedef AttributeSpec<ATNormal, GL_FLOAT, 3> N;
            typedef AttributeSpec<ATTexCoord0, GL_FLOAT, 2> T02;
            typedef AttributeSpec<ATTexCoord1, GL_FLOAT, 2> T12;
            typedef AttributeSpec<ATColor, GL_FLOAT, 4> C4;
        }
    }
}

#endif
