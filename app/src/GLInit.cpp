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

#include "GLInit.h"

#include "Exceptions.h"
#include "Renderer/GL.h"

namespace TrenchBroom {
    // Glew will undefine some of the names declared in GL.h, so we create new names here
    static Func0<void>& _glewInitialize = glewInitialize;
    
    static Func0<GLenum>& _glGetError = glGetError;
    static Func1<const GLubyte*, GLenum>& _glGetString = glGetString;
    
    static Func1<void, GLenum>& _glEnable = glEnable;
    static Func1<void, GLenum>& _glDisable = glDisable;
    static Func1<void, GLbitfield>& _glClear = glClear;
    static Func4<void, GLclampf, GLclampf, GLclampf, GLclampf>& _glClearColor = glClearColor;

    static Func4<void, GLint, GLint, GLsizei, GLsizei>& _glViewport = glViewport;

    static Func2<void, GLenum, GLenum>& _glBlendFunc = glBlendFunc;
    static Func1<void, GLenum>& _glShadeModel = glShadeModel;

    static Func1<void, GLenum>& _glDepthFunc = glDepthFunc;
    static Func1<void, GLboolean>& _glDepthMask = glDepthMask;
    static Func2<void, GLclampd, GLclampd>& _glDepthRange = glDepthRange;
    
    static Func1<void, GLfloat>& _glLineWidth = glLineWidth;
    static Func1<void, GLfloat>& _glPointSize = glPointSize;
    static Func2<void, GLenum, GLenum>& _glPolygonMode = glPolygonMode;
    static Func1<void, GLenum>& _glFrontFace = glFrontFace;
    
    static Func0<void>& _glLoadIdentity = glLoadIdentity;
    static Func1<void, const GLdouble*>& _glLoadMatrixd = glLoadMatrixd;
    static Func1<void, const GLfloat*>& _glLoadMatrixf = glLoadMatrixf;
    static Func1<void, GLenum>& _glMatrixMode = glMatrixMode;
    
    static Func2<void, GLenum, GLint*>& _glGetIntegerv = glGetIntegerv;
    
    static Func2<void, GLenum, GLfloat>& _glPixelStoref = glPixelStoref;
    static Func2<void, GLenum, GLint>& _glPixelStorei = glPixelStorei;
    
    static Func2<void, GLsizei, GLuint*>& _glGenTextures = glGenTextures;
    static Func2<void, GLsizei, const GLuint*>& _glDeleteTextures = glDeleteTextures;
    static Func2<void, GLenum, GLuint>& _glBindTexture = glBindTexture;
    static Func3<void, GLenum, GLenum, GLfloat>& _glTexParameterf = glTexParameterf;
    static Func3<void, GLenum, GLenum, GLint>& _glTexParameteri = glTexParameteri;
    static Func9<void, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*>& _glTexImage2D = glTexImage2D;
    static Func1<void, GLenum>& _glActiveTexture = glActiveTexture;
    
    static Func2<void, GLsizei, GLuint*>& _glGenBuffers = glGenBuffers;
    static Func2<void, GLsizei, const GLuint*>& _glDeleteBuffers = glDeleteBuffers;
    static Func2<void, GLenum, GLuint>& _glBindBuffer = glBindBuffer;
    static Func4<void, GLenum, GLsizeiptr, const GLvoid*, GLenum>& _glBufferData = glBufferData;
    static Func4<void, GLenum, GLintptr, GLsizeiptr, const GLvoid*>& _glBufferSubData = glBufferSubData;
    static Func2<GLvoid*, GLenum, GLenum>& _glMapBuffer = glMapBuffer;
    static Func1<GLboolean, GLenum>& _glUnmapBuffer = glUnmapBuffer;
    
    static Func1<void, GLuint>& _glEnableVertexAttribArray = glEnableVertexAttribArray;
    static Func1<void, GLuint>& _glDisableVertexAttribArray = glDisableVertexAttribArray;
    static Func1<void, GLenum>& _glEnableClientState = glEnableClientState;
    static Func1<void, GLenum>& _glDisableClientState = glDisableClientState;
    static Func1<void, GLenum>& _glClientActiveTexture = glClientActiveTexture;
    
    static Func6<void, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*>& _glVertexAttribPointer = glVertexAttribPointer;
    static Func4<void, GLint, GLenum, GLsizei, const GLvoid*>& _glVertexPointer = glVertexPointer;
    static Func3<void, GLenum, GLsizei, const GLvoid*>& _glNormalPointer = glNormalPointer;
    static Func4<void, GLint, GLenum, GLsizei, const GLvoid*>& _glColorPointer = glColorPointer;
    static Func4<void, GLint, GLenum, GLsizei, const GLvoid*>& _glTexCoordPointer = glTexCoordPointer;
    
    static Func3<void, GLenum, GLint, GLsizei>& _glDrawArrays = glDrawArrays;
    static Func4<void, GLenum, const GLint*, const GLsizei*, GLsizei>& _glMultiDrawArrays = glMultiDrawArrays;
    static Func4<void, GLenum, GLsizei, GLenum, const GLvoid*>& _glDrawElements = glDrawElements;
    static Func6<void, GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*>& _glDrawRangeElements = glDrawRangeElements;
    static Func5<void, GLenum, const GLsizei*, GLenum, const GLvoid**, GLsizei>& _glMultiDrawElements = glMultiDrawElements;

    static Func1<GLuint, GLenum>& _glCreateShader = glCreateShader;
    static Func1<void, GLuint>& _glDeleteShader = glDeleteShader;
    static Func4<void, GLuint, GLsizei, const GLchar**, const GLint*>& _glShaderSource = glShaderSource;
    static Func1<void, GLuint>& _glCompileShader = glCompileShader;
    static Func3<void, GLuint, GLenum, GLint*>& _glGetShaderiv = glGetShaderiv;
    static Func4<void, GLuint, GLsizei, GLsizei*, GLchar*>& _glGetShaderInfoLog = glGetShaderInfoLog;
    static Func2<void, GLuint, GLuint>& _glAttachShader = glAttachShader;
    static Func2<void, GLuint, GLuint>& _glDetachShader = glDetachShader;
    
    static Func0<GLuint>& _glCreateProgram = glCreateProgram;
    static Func1<void, GLuint>& _glDeleteProgram = glDeleteProgram;
    static Func1<void, GLuint>& _glLinkProgram = glLinkProgram;
    static Func3<void, GLuint, GLenum, GLint*>& _glGetProgramiv = glGetProgramiv;
    static Func4<void, GLuint, GLsizei, GLsizei*, GLchar*>& _glGetProgramInfoLog = glGetProgramInfoLog;
    static Func1<void, GLuint>& _glUseProgram = glUseProgram;
    
    static Func2<void, GLint, GLfloat>& _glUniform1f = glUniform1f;
    static Func3<void, GLint, GLfloat, GLfloat>& _glUniform2f = glUniform2f;
    static Func4<void, GLint, GLfloat, GLfloat, GLfloat>& _glUniform3f = glUniform3f;
    static Func5<void, GLint, GLfloat, GLfloat, GLfloat, GLfloat>& _glUniform4f = glUniform4f;
    static Func2<void, GLint, GLint>& _glUniform1i = glUniform1i;
    static Func3<void, GLint, GLint, GLint>& _glUniform2i = glUniform2i;
    static Func4<void, GLint, GLint, GLint, GLint>& _glUniform3i = glUniform3i;
    static Func5<void, GLint, GLint, GLint, GLint, GLint>& _glUniform4i = glUniform4i;
    static Func2<void, GLint, GLuint>& _glUniform1ui = glUniform1ui;
    static Func3<void, GLint, GLuint, GLuint>& _glUniform2ui = glUniform2ui;
    static Func4<void, GLint, GLuint, GLuint, GLuint>& _glUniform3ui = glUniform3ui;
    static Func5<void, GLint, GLuint, GLuint, GLuint, GLuint>& _glUniform4ui = glUniform4ui;
    
    static Func3<void, GLint, GLsizei, const GLfloat*>& _glUniform1fv = glUniform1fv;
    static Func3<void, GLint, GLsizei, const GLfloat*>& _glUniform2fv = glUniform2fv;
    static Func3<void, GLint, GLsizei, const GLfloat*>& _glUniform3fv = glUniform3fv;
    static Func3<void, GLint, GLsizei, const GLfloat*>& _glUniform4fv = glUniform4fv;
    static Func3<void, GLint, GLsizei, const GLint*>& _glUniform1iv = glUniform1iv;
    static Func3<void, GLint, GLsizei, const GLint*>& _glUniform2iv = glUniform2iv;
    static Func3<void, GLint, GLsizei, const GLint*>& _glUniform3iv = glUniform3iv;
    static Func3<void, GLint, GLsizei, const GLint*>& _glUniform4iv = glUniform4iv;
    static Func3<void, GLint, GLsizei, const GLuint*>& _glUniform1uiv = glUniform1uiv;
    static Func3<void, GLint, GLsizei, const GLuint*>& _glUniform2uiv = glUniform2uiv;
    static Func3<void, GLint, GLsizei, const GLuint*>& _glUniform3uiv = glUniform3uiv;
    static Func3<void, GLint, GLsizei, const GLuint*>& _glUniform4uiv = glUniform4uiv;
    
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix2fv = glUniformMatrix2fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix3fv = glUniformMatrix3fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix4fv = glUniformMatrix4fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix2x3fv = glUniformMatrix2x3fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix3x2fv = glUniformMatrix3x2fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix2x4fv = glUniformMatrix2x4fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix4x2fv = glUniformMatrix4x2fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix3x4fv = glUniformMatrix3x4fv;
    static Func4<void, GLint, GLsizei, GLboolean, const GLfloat*>& _glUniformMatrix4x3fv = glUniformMatrix4x3fv;
    
    static Func2<GLint, GLuint, const GLchar*>& _glGetUniformLocation = glGetUniformLocation;
    
#ifdef __APPLE__
    static Func2<void, GLenum, GLint>& _glFinishObjectAPPLE = glFinishObjectAPPLE;
#endif
}

#include <GL/glew.h>

namespace TrenchBroom {
    static void initRemainingFunctions() {
        _glGetError.bindFunc(&::glGetError);
        _glGetString.bindFunc(&::glGetString);
        
        _glEnable.bindFunc(&::glEnable);
        _glDisable.bindFunc(&::glDisable);
        _glClear.bindFunc(&::glClear);
        _glClearColor.bindFunc(&::glClearColor);
        
        _glViewport.bindFunc(&::glViewport);
        
        _glBlendFunc.bindFunc(&::glBlendFunc);
        _glShadeModel.bindFunc(&::glShadeModel);
        
        _glDepthFunc.bindFunc(&::glDepthFunc);
        _glDepthMask.bindFunc(&::glDepthMask);
        _glDepthRange.bindFunc(&::glDepthRange);
        
        _glLineWidth.bindFunc(&::glLineWidth);
        _glPointSize.bindFunc(&::glPointSize);
        _glPolygonMode.bindFunc(&::glPolygonMode);
        _glFrontFace.bindFunc(&::glFrontFace);
        
        _glLoadIdentity.bindFunc(&::glLoadIdentity);
        _glLoadMatrixd.bindFunc(&::glLoadMatrixd);
        _glLoadMatrixf.bindFunc(&::glLoadMatrixf);
        _glMatrixMode.bindFunc(&::glMatrixMode);
        
        _glGetIntegerv.bindFunc(&::glGetIntegerv);
        
        _glPixelStoref.bindFunc(&::glPixelStoref);
        _glPixelStorei.bindFunc(&::glPixelStorei);
        
        _glGenTextures.bindFunc(&::glGenTextures);
        _glDeleteTextures.bindFunc(&::glDeleteTextures);
        _glBindTexture.bindFunc(&::glBindTexture);
        _glTexParameterf.bindFunc(&::glTexParameterf);
        _glTexParameteri.bindFunc(&::glTexParameteri);
        _glTexImage2D.bindFunc(&::glTexImage2D);
        _glActiveTexture.bindFunc(glActiveTexture);
        
        _glGenBuffers.bindFunc(glGenBuffers);
        _glDeleteBuffers.bindFunc(glDeleteBuffers);
        _glBindBuffer.bindFunc(glBindBuffer);
        _glBufferData.bindFunc(glBufferData);
        _glBufferSubData.bindFunc(glBufferSubData);
        _glMapBuffer.bindFunc(glMapBuffer);
        _glUnmapBuffer.bindFunc(glUnmapBuffer);
        
        _glEnableVertexAttribArray.bindFunc(glEnableVertexAttribArray);
        _glDisableVertexAttribArray.bindFunc(glDisableVertexAttribArray);
        _glEnableClientState.bindFunc(&::glEnableClientState);
        _glDisableClientState.bindFunc(&::glDisableClientState);
        _glClientActiveTexture.bindFunc(glClientActiveTexture);
        
        _glVertexAttribPointer.bindFunc(glVertexAttribPointer);
        _glVertexPointer.bindFunc(&::glVertexPointer);
        _glNormalPointer.bindFunc(&::glNormalPointer);
        _glColorPointer.bindFunc(&::glColorPointer);
        _glTexCoordPointer.bindFunc(&::glTexCoordPointer);
        
        _glDrawArrays.bindFunc(&::glDrawArrays);
        _glMultiDrawArrays.bindFunc(glMultiDrawArrays);
        _glDrawElements.bindFunc(&::glDrawElements);
        _glDrawRangeElements.bindFunc(glDrawRangeElements);
        _glMultiDrawElements.bindFunc(glMultiDrawElements);
        
        _glCreateShader.bindFunc(glCreateShader);
        _glDeleteShader.bindFunc(glDeleteShader);
        _glShaderSource.bindFunc(glShaderSource);
        _glCompileShader.bindFunc(glCompileShader);
        _glGetShaderiv.bindFunc(glGetShaderiv);
        _glGetShaderInfoLog.bindFunc(glGetShaderInfoLog);
        _glAttachShader.bindFunc(glAttachShader);
        _glDetachShader.bindFunc(glDetachShader);
        
        _glCreateProgram.bindFunc(glCreateProgram);
        _glDeleteProgram.bindFunc(glDeleteProgram);
        _glLinkProgram.bindFunc(glLinkProgram);
        _glGetProgramiv.bindFunc(glGetProgramiv);
        _glGetProgramInfoLog.bindFunc(glGetProgramInfoLog);
        _glUseProgram.bindFunc(glUseProgram);
        
        _glUniform1f.bindFunc(glUniform1f);
        _glUniform2f.bindFunc(glUniform2f);
        _glUniform3f.bindFunc(glUniform3f);
        _glUniform4f.bindFunc(glUniform4f);
        _glUniform1i.bindFunc(glUniform1i);
        _glUniform2i.bindFunc(glUniform2i);
        _glUniform3i.bindFunc(glUniform3i);
        _glUniform4i.bindFunc(glUniform4i);
        _glUniform1ui.bindFunc(glUniform1ui);
        _glUniform2ui.bindFunc(glUniform2ui);
        _glUniform3ui.bindFunc(glUniform3ui);
        _glUniform4ui.bindFunc(glUniform4ui);
        
        _glUniform1fv.bindFunc(glUniform1fv);
        _glUniform2fv.bindFunc(glUniform2fv);
        _glUniform3fv.bindFunc(glUniform3fv);
        _glUniform4fv.bindFunc(glUniform4fv);
        _glUniform1iv.bindFunc(glUniform1iv);
        _glUniform2iv.bindFunc(glUniform2iv);
        _glUniform3iv.bindFunc(glUniform3iv);
        _glUniform4iv.bindFunc(glUniform4iv);
        _glUniform1uiv.bindFunc(glUniform1uiv);
        _glUniform2uiv.bindFunc(glUniform2uiv);
        _glUniform3uiv.bindFunc(glUniform3uiv);
        _glUniform4uiv.bindFunc(glUniform4uiv);
        
        _glUniformMatrix2fv.bindFunc(glUniformMatrix2fv);
        _glUniformMatrix3fv.bindFunc(glUniformMatrix3fv);
        _glUniformMatrix4fv.bindFunc(glUniformMatrix4fv);
        _glUniformMatrix2x3fv.bindFunc(glUniformMatrix2x3fv);
        _glUniformMatrix3x2fv.bindFunc(glUniformMatrix3x2fv);
        _glUniformMatrix2x4fv.bindFunc(glUniformMatrix2x4fv);
        _glUniformMatrix4x2fv.bindFunc(glUniformMatrix4x2fv);
        _glUniformMatrix3x4fv.bindFunc(glUniformMatrix3x4fv);
        _glUniformMatrix4x3fv.bindFunc(glUniformMatrix4x3fv);
        
        _glGetUniformLocation.bindFunc(glGetUniformLocation);
        
#ifdef __APPLE__
        _glFinishObjectAPPLE.bindFunc(glFinishObjectAPPLE);
#endif
    }
    
    static void initializeGlew() {
        glewExperimental = GL_TRUE;
        const GLenum glewState = glewInit();
        if (glewState != GLEW_OK) {
            RenderException e;
            e << "Error initializing glew: " << glewGetErrorString(glewState);
            throw e;
        }

        static bool initialized = false;
        if (!initialized) {
            initRemainingFunctions();
            initialized = true;
        }
    }
    
    void initGLFunctions() {
      _glewInitialize.bindFunc(&initializeGlew);
    }
}
