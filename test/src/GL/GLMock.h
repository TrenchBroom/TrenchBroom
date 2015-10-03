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

#ifndef TrenchBroom_GLMock_h
#define TrenchBroom_GLMock_h

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Renderer/GL.h"

namespace TrenchBroom {
    class GLMock {
    public:
        GLenum GetError() { return GL_NO_ERROR; }
        const GLubyte* GetString(GLenum) { return NULL; }
        
        MOCK_METHOD0(GlewInitialize, void());
        
        MOCK_METHOD1(Enable, void(GLenum));
        MOCK_METHOD1(Disable, void(GLenum));
        MOCK_METHOD1(Clear, void(GLbitfield));
        MOCK_METHOD4(ClearColor, void(GLclampf, GLclampf, GLclampf, GLclampf));

        MOCK_METHOD4(Viewport, void(GLint, GLint, GLsizei, GLsizei));
        
        MOCK_METHOD2(BlendFunc, void(GLenum, GLenum));
        MOCK_METHOD1(ShadeModel, void(GLenum));
        
        MOCK_METHOD1(DepthFunc, void(GLenum));
        MOCK_METHOD1(DepthMask, void(GLboolean));
        MOCK_METHOD2(DepthRange, void(GLclampd, GLclampd));

        MOCK_METHOD1(LineWidth, void(GLfloat));
        MOCK_METHOD2(PolygonMode, void(GLenum, GLenum));
        MOCK_METHOD1(FrontFace, void(GLenum));
        
        MOCK_METHOD0(LoadIdentity, void());
        MOCK_METHOD1(LoadMatrixd, void(const GLdouble*));
        MOCK_METHOD1(LoadMatrixf, void(const GLfloat*));
        MOCK_METHOD1(MatrixMode, void(GLenum));
        
        MOCK_METHOD2(GetIntegerv, void(GLenum, GLint*));
        
        MOCK_METHOD2(PixelStoref, void(GLenum, GLfloat));
        MOCK_METHOD2(PixelStorei, void(GLenum, GLint));
        
        MOCK_METHOD2(GenTextures, void(GLsizei, GLuint*));
        MOCK_METHOD2(DeleteTextures, void(GLsizei, const GLuint*));
        MOCK_METHOD2(BindTexture, void(GLenum, GLuint));
        MOCK_METHOD3(TexParameterf, void(GLenum, GLenum, GLfloat));
        MOCK_METHOD3(TexParameteri, void(GLenum, GLenum, GLint));
        MOCK_METHOD9(TexImage2D, void(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*));
        MOCK_METHOD1(ActiveTexture, void(GLenum));
        
        MOCK_METHOD2(GenBuffers, void(GLsizei, GLuint*));
        MOCK_METHOD2(DeleteBuffers, void(GLsizei, const GLuint*));
        MOCK_METHOD2(BindBuffer, void(GLenum, GLuint));
        MOCK_METHOD4(BufferData, void(GLenum, GLsizeiptr, const GLvoid*, GLenum));
        MOCK_METHOD4(BufferSubData, void(GLenum, GLintptr, GLsizeiptr, const GLvoid*));
        MOCK_METHOD2(MapBuffer, void*(GLenum, GLenum));
        MOCK_METHOD1(UnmapBuffer, GLboolean(GLenum));
        
        MOCK_METHOD1(EnableVertexAttribArray, void(GLuint));
        MOCK_METHOD1(DisableVertexAttribArray, void(GLuint));
        MOCK_METHOD1(EnableClientState, void(GLenum));
        MOCK_METHOD1(DisableClientState, void(GLenum));
        MOCK_METHOD1(ClientActiveTexture, void(GLenum));
        
        MOCK_METHOD6(VertexAttribPointer, void(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*));
        MOCK_METHOD4(VertexPointer, void(GLint, GLenum, GLsizei, const GLvoid*));
        MOCK_METHOD3(NormalPointer, void(GLenum, GLsizei, const GLvoid*));
        MOCK_METHOD4(ColorPointer, void(GLint, GLenum, GLsizei, const GLvoid*));
        MOCK_METHOD4(TexCoordPointer, void(GLint, GLenum, GLsizei, const GLvoid*));
        
        MOCK_METHOD3(DrawArrays, void(GLenum, GLint, GLsizei));
        MOCK_METHOD4(MultiDrawArrays, void(GLenum, const GLint*, const GLsizei*, GLsizei));
        
        MOCK_METHOD1(CreateShader, GLuint(GLenum));
        MOCK_METHOD1(DeleteShader, void(GLuint));
        MOCK_METHOD4(ShaderSource, void(GLuint, GLsizei, const GLchar**, const GLint*));
        MOCK_METHOD1(CompileShader, void(GLuint));
        MOCK_METHOD3(GetShaderiv, void(GLuint, GLenum, GLint*));
        MOCK_METHOD4(GetShaderInfoLog, void(GLuint, GLsizei, GLsizei*, GLchar*));
        MOCK_METHOD2(AttachShader, void(GLuint, GLuint));
        MOCK_METHOD2(DetachShader, void(GLuint, GLuint));
        
        MOCK_METHOD0(CreateProgram, GLuint());
        MOCK_METHOD1(DeleteProgram, void(GLuint));
        MOCK_METHOD1(LinkProgram, void(GLuint));
        MOCK_METHOD3(GetProgramiv, void(GLuint, GLenum, GLint*));
        MOCK_METHOD4(GetProgramInfoLog, void(GLuint, GLsizei, GLsizei*, GLchar*));
        MOCK_METHOD1(UseProgram, void(GLuint));
        
        MOCK_METHOD2(Uniform1f, void(GLint, GLfloat));
        MOCK_METHOD3(Uniform2f, void(GLint, GLfloat, GLfloat));
        MOCK_METHOD4(Uniform3f, void(GLint, GLfloat, GLfloat, GLfloat));
        MOCK_METHOD5(Uniform4f, void(GLint, GLfloat, GLfloat, GLfloat, GLfloat));
        MOCK_METHOD2(Uniform1i, void(GLint, GLint));
        MOCK_METHOD3(Uniform2i, void(GLint, GLint, GLint));
        MOCK_METHOD4(Uniform3i, void(GLint, GLint, GLint, GLint));
        MOCK_METHOD5(Uniform4i, void(GLint, GLint, GLint, GLint, GLint));
        MOCK_METHOD2(Uniform1ui, void(GLint, GLuint));
        MOCK_METHOD3(Uniform2ui, void(GLint, GLuint, GLuint));
        MOCK_METHOD4(Uniform3ui, void(GLint, GLuint, GLuint, GLuint));
        MOCK_METHOD5(Uniform4ui, void(GLint, GLuint, GLuint, GLuint, GLuint));
        
        MOCK_METHOD3(Uniform1fv, void(GLint, GLsizei, const GLfloat*));
        MOCK_METHOD3(Uniform2fv, void(GLint, GLsizei, const GLfloat*));
        MOCK_METHOD3(Uniform3fv, void(GLint, GLsizei, const GLfloat*));
        MOCK_METHOD3(Uniform4fv, void(GLint, GLsizei, const GLfloat*));
        MOCK_METHOD3(Uniform1iv, void(GLint, GLsizei, const GLint*));
        MOCK_METHOD3(Uniform2iv, void(GLint, GLsizei, const GLint*));
        MOCK_METHOD3(Uniform3iv, void(GLint, GLsizei, const GLint*));
        MOCK_METHOD3(Uniform4iv, void(GLint, GLsizei, const GLint*));
        MOCK_METHOD3(Uniform1uiv, void(GLint, GLsizei, const GLuint*));
        MOCK_METHOD3(Uniform2uiv, void(GLint, GLsizei, const GLuint*));
        MOCK_METHOD3(Uniform3uiv, void(GLint, GLsizei, const GLuint*));
        MOCK_METHOD3(Uniform4uiv, void(GLint, GLsizei, const GLuint*));
        
        MOCK_METHOD4(UniformMatrix2fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix3fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix4fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix2x3fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix3x2fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix2x4fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix4x2fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix3x4fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        MOCK_METHOD4(UniformMatrix4x3fv, void(GLint, GLsizei, GLboolean, const GLfloat*));
        
        MOCK_METHOD2(GetUniformLocation, GLint(GLuint, const GLchar*));
        
#ifdef __APPLE__
        void FinishObjectAPPLE(GLenum, GLint) {}
#endif
        
        GLMock();
        virtual ~GLMock() {}
    };
}

#endif
