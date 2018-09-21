/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_GL_h
#define TrenchBroom_GL_h

#include "Exceptions.h"
#include "StringUtils.h"

#include <cstddef>
#include <vector>

#include <GL/glew.h>

namespace TrenchBroom {
    typedef GLenum PrimType;
    
    typedef std::vector<GLint>   GLIndices;
    typedef std::vector<GLsizei> GLCounts;

    void glCheckError(const String& msg);
    String glGetErrorMessage(GLenum code);

// #define GL_DEBUG 1
// #define GL_LOG 1
    
#if !defined(NDEBUG) && defined(GL_DEBUG) // in debug mode
    #if defined(GL_LOG)
        #define glAssert(C) { std::cout << #C << std::endl; glCheckError("before " #C); (C); glCheckError("after " #C); }
    #else
        #define glAssert(C) { glCheckError("before " #C); (C); glCheckError("after " #C); }
    #endif
#else
    #define glAssert(C) { (C); }
#endif

    template <GLenum T> struct GLType               { typedef GLvoid    Type; };
    template <> struct GLType<GL_BYTE>              { typedef GLbyte    Type; };
    template <> struct GLType<GL_UNSIGNED_BYTE>     { typedef GLubyte   Type; };
    template <> struct GLType<GL_SHORT>             { typedef GLshort   Type; };
    template <> struct GLType<GL_UNSIGNED_SHORT>    { typedef GLushort  Type; };
    template <> struct GLType<GL_INT>               { typedef GLint     Type; };
    template <> struct GLType<GL_UNSIGNED_INT>      { typedef GLuint    Type; };
    template <> struct GLType<GL_FLOAT>             { typedef GLfloat   Type; };
    template <> struct GLType<GL_DOUBLE>            { typedef GLdouble  Type; };
    
    template <typename T> struct GLEnum { static const GLenum Value = GL_INVALID_ENUM; };
    template <> struct GLEnum<GLbyte>   { static const GLenum Value = GL_BYTE; };
    template <> struct GLEnum<GLubyte>  { static const GLenum Value = GL_UNSIGNED_BYTE; };
    template <> struct GLEnum<GLshort>  { static const GLenum Value = GL_SHORT; };
    template <> struct GLEnum<GLushort> { static const GLenum Value = GL_UNSIGNED_SHORT; };
    template <> struct GLEnum<GLint>    { static const GLenum Value = GL_INT; };
    template <> struct GLEnum<GLuint>   { static const GLenum Value = GL_UNSIGNED_INT; };
    template <> struct GLEnum<GLfloat>  { static const GLenum Value = GL_FLOAT; };
    template <> struct GLEnum<GLdouble> { static const GLenum Value = GL_DOUBLE; };
    
    template <typename T> GLenum glType() { return GLEnum<T>::Value; }
}

#endif
