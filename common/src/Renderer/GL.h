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

#include <string>
#include <vector>

#include <GL/glew.h>

namespace TrenchBroom {
    using GLIndices = std::vector<GLint>;
    using GLCounts = std::vector<GLsizei>;

    void glCheckError(const std::string& msg);
    std::string glGetErrorMessage(GLenum code);

    GLenum glGetEnum(const std::string& name);
    std::string glGetEnumName(GLenum _enum);

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

    template <GLenum T> struct GLType               { using Type = GLvoid;   };
    template <> struct GLType<GL_BYTE>              { using Type = GLbyte;   };
    template <> struct GLType<GL_UNSIGNED_BYTE>     { using Type = GLubyte;  };
    template <> struct GLType<GL_SHORT>             { using Type = GLshort;  };
    template <> struct GLType<GL_UNSIGNED_SHORT>    { using Type = GLushort; };
    template <> struct GLType<GL_INT>               { using Type = GLint;    };
    template <> struct GLType<GL_UNSIGNED_INT>      { using Type = GLuint;   };
    template <> struct GLType<GL_FLOAT>             { using Type = GLfloat;  };
    template <> struct GLType<GL_DOUBLE>            { using Type = GLdouble; };

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
