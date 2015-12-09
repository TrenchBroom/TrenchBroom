/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Functor.h"

#include <cstddef>
#include <vector>

namespace TrenchBroom {
#define GL_FALSE 0
#define GL_TRUE 1

#define GL_NO_ERROR 0
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_POLYGON 0x0009

#define GL_DEPTH_BUFFER_BIT 0x00000100

#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207

#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408

#define GL_INVALID_ENUM                     0x0500
#define GL_INVALID_VALUE                    0x0501
#define GL_INVALID_OPERATION                0x0502
#define GL_STACK_OVERFLOW                   0x0503
#define GL_STACK_UNDERFLOW                  0x0504
#define GL_OUT_OF_MEMORY                    0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION    0x0506
#define GL_CONTEXT_LOST                     0x0507
#define GL_TABLE_TOO_LARGE1                 0x8031

#define GL_CW 0x0900
#define GL_CCW 0x0901

#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_DEPTH_FUNC 0x0B74

#define GL_UNPACK_SWAP_BYTES 0x0CF0
#define GL_UNPACK_LSB_FIRST 0x0CF1
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_SWAP_BYTES 0x0D00
#define GL_PACK_LSB_FIRST 0x0D01
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_PACK_ALIGNMENT 0x0D05

#define GL_TEXTURE_2D 0x0DE1
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_DOUBLE 0x140A

#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701

#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A

#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02

#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP 0x2900
#define GL_REPEAT 0x2901
    
#define GL_VERTEX_ARRAY 0x8074
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_INDEX_ARRAY 0x8077
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_EDGE_FLAG_ARRAY 0x8079

#define GL_MULTISAMPLE 0x809D

#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MAX_LEVEL 0x813D

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3

#define GL_BUFFER_OBJECT_APPLE 0x85B3

#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_CURRENT_PROGRAM 0x8B8D

    typedef unsigned int GLenum;
    typedef unsigned int GLbitfield;
    typedef int GLsizei;
    typedef unsigned char GLboolean;
    typedef void GLvoid;
    
    typedef signed char GLbyte;
    typedef signed short GLshort;
    typedef signed int GLint;
    typedef signed long GLlong;
    typedef unsigned char GLubyte;
    typedef unsigned short GLushort;
    typedef unsigned int GLuint;
    typedef unsigned long GLulong;
    
    typedef float GLfloat;
    typedef double GLdouble;
    
    typedef float GLclampf;
    typedef double GLclampd;
    
    typedef ptrdiff_t GLintptr;
    typedef ptrdiff_t GLsizeiptr;
    
    typedef char GLchar;
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
    
    extern Func0<void> glewInitialize;
    
    extern Func0<GLenum> glGetError;
    extern Func1<const GLubyte*, GLenum> glGetString;

    extern Func1<void, GLenum> glEnable;
    extern Func1<void, GLenum> glDisable;
    extern Func1<void, GLbitfield> glClear;
    extern Func4<void, GLclampf, GLclampf, GLclampf, GLclampf> glClearColor;
    
    extern Func4<void, GLint, GLint, GLsizei, GLsizei> glViewport;
    
    extern Func2<void, GLenum, GLenum> glBlendFunc;
    extern Func1<void, GLenum> glShadeModel;
    
    extern Func1<void, GLenum> glDepthFunc;
    extern Func1<void, GLboolean> glDepthMask;
    extern Func2<void, GLclampd, GLclampd> glDepthRange;
    
    extern Func1<void, GLfloat> glLineWidth;
    extern Func1<void, GLfloat> glPointSize;
    extern Func2<void, GLenum, GLenum> glPolygonMode;
    extern Func1<void, GLenum> glFrontFace;
    
    extern Func0<void> glLoadIdentity;
    extern Func1<void, const GLdouble*> glLoadMatrixd;
    extern Func1<void, const GLfloat*> glLoadMatrixf;
    extern Func1<void, GLenum> glMatrixMode;

    extern Func2<void, GLenum, GLint*> glGetIntegerv;
    
    extern Func2<void, GLenum, GLfloat> glPixelStoref;
    extern Func2<void, GLenum, GLint> glPixelStorei;
    
    extern Func2<void, GLsizei, GLuint*> glGenTextures;
    extern Func2<void, GLsizei, const GLuint*> glDeleteTextures;
    extern Func2<void, GLenum, GLuint> glBindTexture;
    extern Func3<void, GLenum, GLenum, GLfloat> glTexParameterf;
    extern Func3<void, GLenum, GLenum, GLint> glTexParameteri;
    extern Func9<void, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*> glTexImage2D;
    extern Func1<void, GLenum> glActiveTexture;
    
    extern Func2<void, GLsizei, GLuint*> glGenBuffers;
    extern Func2<void, GLsizei, const GLuint*> glDeleteBuffers;
    extern Func2<void, GLenum, GLuint> glBindBuffer;
    extern Func4<void, GLenum, GLsizeiptr, const GLvoid*, GLenum> glBufferData;
    extern Func4<void, GLenum, GLintptr, GLsizeiptr, const GLvoid*> glBufferSubData;
    extern Func2<GLvoid*, GLenum, GLenum> glMapBuffer;
    extern Func1<GLboolean, GLenum> glUnmapBuffer;
    
    extern Func1<void, GLuint> glEnableVertexAttribArray;
    extern Func1<void, GLuint> glDisableVertexAttribArray;
    extern Func1<void, GLenum> glEnableClientState;
    extern Func1<void, GLenum> glDisableClientState;
    extern Func1<void, GLenum> glClientActiveTexture;
    
    extern Func6<void, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*> glVertexAttribPointer;
    extern Func4<void, GLint, GLenum, GLsizei, const GLvoid*> glVertexPointer;
    extern Func3<void, GLenum, GLsizei, const GLvoid*> glNormalPointer;
    extern Func4<void, GLint, GLenum, GLsizei, const GLvoid*> glColorPointer;
    extern Func4<void, GLint, GLenum, GLsizei, const GLvoid*> glTexCoordPointer;
    
    extern Func3<void, GLenum, GLint, GLsizei> glDrawArrays;
    extern Func4<void, GLenum, const GLint*, const GLsizei*, GLsizei> glMultiDrawArrays;
    extern Func4<void, GLenum, GLsizei, GLenum, const GLvoid*> glDrawElements;
    extern Func6<void, GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*> glDrawRangeElements;
    extern Func5<void, GLenum, const GLsizei*, GLenum, const GLvoid**, GLsizei> glMultiDrawElements;

    extern Func1<GLuint, GLenum> glCreateShader;
    extern Func1<void, GLuint> glDeleteShader;
    extern Func4<void, GLuint, GLsizei, const GLchar**, const GLint*> glShaderSource;
    extern Func1<void, GLuint> glCompileShader;
    extern Func3<void, GLuint, GLenum, GLint*> glGetShaderiv;
    extern Func4<void, GLuint, GLsizei, GLsizei*, GLchar*> glGetShaderInfoLog;
    extern Func2<void, GLuint, GLuint> glAttachShader;
    extern Func2<void, GLuint, GLuint> glDetachShader;
    
    extern Func0<GLuint> glCreateProgram;
    extern Func1<void, GLuint> glDeleteProgram;
    extern Func1<void, GLuint> glLinkProgram;
    extern Func3<void, GLuint, GLenum, GLint*> glGetProgramiv;
    extern Func4<void, GLuint, GLsizei, GLsizei*, GLchar*> glGetProgramInfoLog;
    extern Func1<void, GLuint> glUseProgram;
    
    extern Func2<void, GLint, GLfloat> glUniform1f;
    extern Func3<void, GLint, GLfloat, GLfloat> glUniform2f;
    extern Func4<void, GLint, GLfloat, GLfloat, GLfloat> glUniform3f;
    extern Func5<void, GLint, GLfloat, GLfloat, GLfloat, GLfloat> glUniform4f;
    extern Func2<void, GLint, GLint> glUniform1i;
    extern Func3<void, GLint, GLint, GLint> glUniform2i;
    extern Func4<void, GLint, GLint, GLint, GLint> glUniform3i;
    extern Func5<void, GLint, GLint, GLint, GLint, GLint> glUniform4i;
    extern Func2<void, GLint, GLuint> glUniform1ui;
    extern Func3<void, GLint, GLuint, GLuint> glUniform2ui;
    extern Func4<void, GLint, GLuint, GLuint, GLuint> glUniform3ui;
    extern Func5<void, GLint, GLuint, GLuint, GLuint, GLuint> glUniform4ui;
    
    extern Func3<void, GLint, GLsizei, const GLfloat*> glUniform1fv;
    extern Func3<void, GLint, GLsizei, const GLfloat*> glUniform2fv;
    extern Func3<void, GLint, GLsizei, const GLfloat*> glUniform3fv;
    extern Func3<void, GLint, GLsizei, const GLfloat*> glUniform4fv;
    extern Func3<void, GLint, GLsizei, const GLint*> glUniform1iv;
    extern Func3<void, GLint, GLsizei, const GLint*> glUniform2iv;
    extern Func3<void, GLint, GLsizei, const GLint*> glUniform3iv;
    extern Func3<void, GLint, GLsizei, const GLint*> glUniform4iv;
    extern Func3<void, GLint, GLsizei, const GLuint*> glUniform1uiv;
    extern Func3<void, GLint, GLsizei, const GLuint*> glUniform2uiv;
    extern Func3<void, GLint, GLsizei, const GLuint*> glUniform3uiv;
    extern Func3<void, GLint, GLsizei, const GLuint*> glUniform4uiv;
    
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix2fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix3fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix4fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix2x3fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix3x2fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix2x4fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix4x2fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix3x4fv;
    extern Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix4x3fv;
    
    extern Func2<GLint, GLuint, const GLchar*> glGetUniformLocation;

#ifdef __APPLE__
    extern Func2<void, GLenum, GLint> glFinishObjectAPPLE;
#endif
}

#endif
