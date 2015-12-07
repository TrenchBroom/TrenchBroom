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

#include "GL.h"

namespace TrenchBroom {
    void glCheckError(const String& msg) {
        const GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            RenderException e;
            e << "OpenGL error: " << error << " (" << glGetErrorMessage(error) << ") " << msg;
            throw e;
        }
    }

    String glGetErrorMessage(const GLenum code) {
        switch (code) {
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION";
            case GL_STACK_OVERFLOW:
                return "GL_STACK_OVERFLOW";
            case GL_STACK_UNDERFLOW:
                return "GL_STACK_UNDERFLOW";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION";
            case GL_CONTEXT_LOST:
                return "GL_CONTEXT_LOST";
            case GL_TABLE_TOO_LARGE1:
                return "GL_TABLE_TOO_LARGE1";
            default:
                return "UNKNOWN";
        }
    }

    Func0<void> glewInitialize;
    
    Func0<GLenum> glGetError;
    Func1<const GLubyte*, GLenum> glGetString;
    
    Func1<void, GLenum> glEnable;
    Func1<void, GLenum> glDisable;
    Func1<void, GLbitfield> glClear;
    Func4<void, GLclampf, GLclampf, GLclampf, GLclampf> glClearColor;

    Func4<void, GLint, GLint, GLsizei, GLsizei> glViewport;

    Func2<void, GLenum, GLenum> glBlendFunc;
    Func1<void, GLenum> glShadeModel;

    Func1<void, GLenum> glDepthFunc;
    Func1<void, GLboolean> glDepthMask;
    Func2<void, GLclampd, GLclampd> glDepthRange;
    
    Func1<void, GLfloat> glLineWidth;
    Func1<void, GLfloat> glPointSize;
    Func2<void, GLenum, GLenum> glPolygonMode;
    Func1<void, GLenum> glFrontFace;
    
    Func0<void> glLoadIdentity;
    Func1<void, const GLdouble*> glLoadMatrixd;
    Func1<void, const GLfloat*> glLoadMatrixf;
    Func1<void, GLenum> glMatrixMode;
    
    Func2<void, GLenum, GLint*> glGetIntegerv;
    
    Func2<void, GLenum, GLfloat> glPixelStoref;
    Func2<void, GLenum, GLint> glPixelStorei;
    
    Func2<void, GLsizei, GLuint*> glGenTextures;
    Func2<void, GLsizei, const GLuint*> glDeleteTextures;
    Func2<void, GLenum, GLuint> glBindTexture;
    Func3<void, GLenum, GLenum, GLfloat> glTexParameterf;
    Func3<void, GLenum, GLenum, GLint> glTexParameteri;
    Func9<void, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*> glTexImage2D;
    Func1<void, GLenum> glActiveTexture;
    
    Func2<void, GLsizei, GLuint*> glGenBuffers;
    Func2<void, GLsizei, const GLuint*> glDeleteBuffers;
    Func2<void, GLenum, GLuint> glBindBuffer;
    Func4<void, GLenum, GLsizeiptr, const GLvoid*, GLenum> glBufferData;
    Func4<void, GLenum, GLintptr, GLsizeiptr, const GLvoid*> glBufferSubData;
    Func2<GLvoid*, GLenum, GLenum> glMapBuffer;
    Func1<GLboolean, GLenum> glUnmapBuffer;
    
    Func1<void, GLuint> glEnableVertexAttribArray;
    Func1<void, GLuint> glDisableVertexAttribArray;
    Func1<void, GLenum> glEnableClientState;
    Func1<void, GLenum> glDisableClientState;
    Func1<void, GLenum> glClientActiveTexture;
    
    Func6<void, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*> glVertexAttribPointer;
    Func4<void, GLint, GLenum, GLsizei, const GLvoid*> glVertexPointer;
    Func3<void, GLenum, GLsizei, const GLvoid*> glNormalPointer;
    Func4<void, GLint, GLenum, GLsizei, const GLvoid*> glColorPointer;
    Func4<void, GLint, GLenum, GLsizei, const GLvoid*> glTexCoordPointer;
    
    Func3<void, GLenum, GLint, GLsizei> glDrawArrays;
    Func4<void, GLenum, const GLint*, const GLsizei*, GLsizei> glMultiDrawArrays;
    Func4<void, GLenum, GLsizei, GLenum, const GLvoid*> glDrawElements;
    Func6<void, GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*> glDrawRangeElements;
    Func5<void, GLenum, const GLsizei*, GLenum, const GLvoid**, GLsizei> glMultiDrawElements;
    
    Func1<GLuint, GLenum> glCreateShader;
    Func1<void, GLuint> glDeleteShader;
    Func4<void, GLuint, GLsizei, const GLchar**, const GLint*> glShaderSource;
    Func1<void, GLuint> glCompileShader;
    Func3<void, GLuint, GLenum, GLint*> glGetShaderiv;
    Func4<void, GLuint, GLsizei, GLsizei*, GLchar*> glGetShaderInfoLog;
    Func2<void, GLuint, GLuint> glAttachShader;
    Func2<void, GLuint, GLuint> glDetachShader;
    
    Func0<GLuint> glCreateProgram;
    Func1<void, GLuint> glDeleteProgram;
    Func1<void, GLuint> glLinkProgram;
    Func3<void, GLuint, GLenum, GLint*> glGetProgramiv;
    Func4<void, GLuint, GLsizei, GLsizei*, GLchar*> glGetProgramInfoLog;
    Func1<void, GLuint> glUseProgram;
    
    Func2<void, GLint, GLfloat> glUniform1f;
    Func3<void, GLint, GLfloat, GLfloat> glUniform2f;
    Func4<void, GLint, GLfloat, GLfloat, GLfloat> glUniform3f;
    Func5<void, GLint, GLfloat, GLfloat, GLfloat, GLfloat> glUniform4f;
    Func2<void, GLint, GLint> glUniform1i;
    Func3<void, GLint, GLint, GLint> glUniform2i;
    Func4<void, GLint, GLint, GLint, GLint> glUniform3i;
    Func5<void, GLint, GLint, GLint, GLint, GLint> glUniform4i;
    Func2<void, GLint, GLuint> glUniform1ui;
    Func3<void, GLint, GLuint, GLuint> glUniform2ui;
    Func4<void, GLint, GLuint, GLuint, GLuint> glUniform3ui;
    Func5<void, GLint, GLuint, GLuint, GLuint, GLuint> glUniform4ui;
    
    Func3<void, GLint, GLsizei, const GLfloat*> glUniform1fv;
    Func3<void, GLint, GLsizei, const GLfloat*> glUniform2fv;
    Func3<void, GLint, GLsizei, const GLfloat*> glUniform3fv;
    Func3<void, GLint, GLsizei, const GLfloat*> glUniform4fv;
    Func3<void, GLint, GLsizei, const GLint*> glUniform1iv;
    Func3<void, GLint, GLsizei, const GLint*> glUniform2iv;
    Func3<void, GLint, GLsizei, const GLint*> glUniform3iv;
    Func3<void, GLint, GLsizei, const GLint*> glUniform4iv;
    Func3<void, GLint, GLsizei, const GLuint*> glUniform1uiv;
    Func3<void, GLint, GLsizei, const GLuint*> glUniform2uiv;
    Func3<void, GLint, GLsizei, const GLuint*> glUniform3uiv;
    Func3<void, GLint, GLsizei, const GLuint*> glUniform4uiv;
    
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix2fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix3fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix4fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix2x3fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix3x2fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix2x4fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix4x2fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix3x4fv;
    Func4<void, GLint, GLsizei, GLboolean, const GLfloat*> glUniformMatrix4x3fv;
    
    Func2<GLint, GLuint, const GLchar*> glGetUniformLocation;
    
#ifdef __APPLE__
    Func2<void, GLenum, GLint> glFinishObjectAPPLE;
#endif
}
