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

#include "GL/glew.h"

/*
 void glDeleteTextures(	GLsizei  	n,
 const GLuint *  	textures);
 */

class CGLMock {
public:
    MOCK_METHOD1(Enable, void(GLenum cap));
    
    MOCK_METHOD2(GenTextures, void(GLsizei n, GLuint* textures));
    MOCK_METHOD2(DeleteTextures, void(GLsizei n, const GLuint* textures));
    MOCK_METHOD2(BindTexture, void(GLenum target, GLuint texture));
    MOCK_METHOD3(TexParameterf, void(GLenum target, GLenum pname, GLfloat param));
    MOCK_METHOD3(TexParameteri, void(GLenum target, GLenum pname, GLint param));
    MOCK_METHOD9(TexImage2D, void(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data));
    MOCK_METHOD1(ActiveTexture, void(GLenum texture));
    
    MOCK_METHOD2(GenBuffers, void(GLsizei n, GLuint* buffers));
    MOCK_METHOD2(DeleteBuffers, void(GLsizei n, GLuint* buffers));
    MOCK_METHOD2(BindBuffer, void(GLenum type, GLuint bufferId));
    MOCK_METHOD4(BufferData, void(GLenum type, GLsizeiptr size, const GLvoid* buffer, GLenum usage));
    MOCK_METHOD2(MapBuffer, void*(GLenum type, GLenum access));
    MOCK_METHOD1(UnmapBuffer, void(GLenum type));
    
    MOCK_METHOD1(EnableVertexAttribArray, void(GLuint index));
    MOCK_METHOD1(DisableVertexAttribArray, void(GLuint index));
    MOCK_METHOD1(EnableClientState, void(GLenum cap));
    MOCK_METHOD1(DisableClientState, void(GLenum cap));
    MOCK_METHOD1(ClientActiveTexture, void(GLenum texture));
    
    MOCK_METHOD6(VertexAttribPointer, void(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer));
    MOCK_METHOD4(VertexPointer, void(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer));
    MOCK_METHOD3(NormalPointer, void(GLenum type, GLsizei stride, const GLvoid* pointer));
    MOCK_METHOD4(ColorPointer, void(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer));
    MOCK_METHOD4(TexCoordPointer, void(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer));
    
    MOCK_METHOD3(DrawArrays, void(GLenum mode, GLint first, GLsizei count));
    MOCK_METHOD4(MultiDrawArrays, void(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primCount));
    
    MOCK_METHOD1(CreateShader, GLuint(GLenum type));
    MOCK_METHOD1(DeleteShader, void(GLuint shader));
    MOCK_METHOD4(ShaderSource, void(GLuint shader, GLsizei count, const GLchar** string, const GLint* length));
    MOCK_METHOD1(CompileShader, void(GLuint shader));
    MOCK_METHOD3(GetShaderiv, void(GLuint shader, GLenum pname, GLint* params));
    MOCK_METHOD4(GetShaderInfoLog, void(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog));
    MOCK_METHOD2(AttachShader, void(GLuint program, GLuint shader));
    MOCK_METHOD2(DetachShader, void(GLuint program, GLuint shader));
    
    MOCK_METHOD0(CreateProgram, GLuint());
    MOCK_METHOD1(DeleteProgram, void(GLuint program));
    MOCK_METHOD1(LinkProgram, void(GLuint program));
    MOCK_METHOD3(GetProgramiv, void(GLuint program, GLenum pname, GLint* params));
    MOCK_METHOD4(GetProgramInfoLog, void(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog));
    MOCK_METHOD1(UseProgram, void(GLuint program));
    
    MOCK_METHOD2(Uniform1f, void(GLint location, GLfloat v0));
    MOCK_METHOD3(Uniform2f, void(GLint location, GLfloat v0, GLfloat v1));
    MOCK_METHOD4(Uniform3f, void(GLint location, GLfloat v0, GLfloat v1, GLfloat v2));
    MOCK_METHOD5(Uniform4f, void(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
    MOCK_METHOD2(Uniform1i, void(GLint location, GLint v0));
    MOCK_METHOD3(Uniform2i, void(GLint location, GLint v0, GLint v1));
    MOCK_METHOD4(Uniform3i, void(GLint location, GLint v0, GLint v1, GLint v2));
    MOCK_METHOD5(Uniform4i, void(GLint location, GLint v0, GLint v1, GLint v2, GLint v3));
    MOCK_METHOD2(Uniform1ui, void(GLint location, GLuint v0));
    MOCK_METHOD3(Uniform2ui, void(GLint location, GLuint v0, GLuint v1));
    MOCK_METHOD4(Uniform3ui, void(GLint location, GLuint v0, GLuint v1, GLuint v2));
    MOCK_METHOD5(Uniform4ui, void(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3));

    MOCK_METHOD3(Uniform1fv, void(GLint location, GLsizei count, const GLfloat* value));
    MOCK_METHOD3(Uniform2fv, void(GLint location, GLsizei count, const GLfloat* value));
    MOCK_METHOD3(Uniform3fv, void(GLint location, GLsizei count, const GLfloat* value));
    MOCK_METHOD3(Uniform4fv, void(GLint location, GLsizei count, const GLfloat* value));
    MOCK_METHOD3(Uniform1iv, void(GLint location, GLsizei count, const GLint* value));
    MOCK_METHOD3(Uniform2iv, void(GLint location, GLsizei count, const GLint* value));
    MOCK_METHOD3(Uniform3iv, void(GLint location, GLsizei count, const GLint* value));
    MOCK_METHOD3(Uniform4iv, void(GLint location, GLsizei count, const GLint* value));
    MOCK_METHOD3(Uniform1uiv, void(GLint location, GLsizei count, const GLuint* value));
    MOCK_METHOD3(Uniform2uiv, void(GLint location, GLsizei count, const GLuint* value));
    MOCK_METHOD3(Uniform3uiv, void(GLint location, GLsizei count, const GLuint* value));
    MOCK_METHOD3(Uniform4uiv, void(GLint location, GLsizei count, const GLuint* value));

    MOCK_METHOD4(UniformMatrix2fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix3fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix4fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix2x3fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix3x2fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix2x4fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix4x2fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix3x4fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    MOCK_METHOD4(UniformMatrix4x3fv, void(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
    
    MOCK_METHOD2(GetUniformLocation, GLint(GLuint program, const GLchar* name));
    
    void FinishObjectAPPLE(GLuint object, GLuint name) {}
    GLenum GetError() {return GL_NO_ERROR;}
};

extern CGLMock* GLMock;

#undef glEnable
#define glEnable                    GLMock->Enable

#undef glGenTextures
#define glGenTextures               GLMock->GenTextures
#undef glDeleteTextures
#define glDeleteTextures            GLMock->DeleteTextures
#undef glBindTexture
#define glBindTexture               GLMock->BindTexture
#undef glTexParameterf
#define glTexParameterf             GLMock->TexParameterf
#undef glTexParameteri
#define glTexParameteri             GLMock->TexParameteri
#undef glTexImage2D
#define glTexImage2D                GLMock->TexImage2D
#undef glActiveTexture
#define glActiveTexture             GLMock->ActiveTexture


#undef glGenBuffers
#define glGenBuffers                GLMock->GenBuffers
#undef glDeleteBuffers
#define glDeleteBuffers             GLMock->DeleteBuffers
#undef glBindBuffer
#define glBindBuffer                GLMock->BindBuffer
#undef glBufferData
#define glBufferData                GLMock->BufferData
#undef glMapBuffer
#define glMapBuffer                 GLMock->MapBuffer
#undef glUnmapBuffer
#define glUnmapBuffer               GLMock->UnmapBuffer

#undef glEnableVertexAttribArray
#define glEnableVertexAttribArray   GLMock->EnableVertexAttribArray
#undef glDisableVertexAttribArray
#define glDisableVertexAttribArray  GLMock->DisableVertexAttribArray
#undef glEnableClientState
#define glEnableClientState         GLMock->EnableClientState
#undef glDisableClientState
#define glDisableClientState        GLMock->DisableClientState
#undef glClientActiveTexture
#define glClientActiveTexture       GLMock->ClientActiveTexture

#undef glVertexAttribPointer
#define glVertexAttribPointer       GLMock->VertexAttribPointer
#undef glVertexPointer
#define glVertexPointer             GLMock->VertexPointer
#undef glNormalPointer
#define glNormalPointer             GLMock->NormalPointer
#undef glColorPointer
#define glColorPointer              GLMock->ColorPointer
#undef glTexCoordPointer
#define glTexCoordPointer           GLMock->TexCoordPointer

#undef glDrawArrays
#define glDrawArrays                GLMock->DrawArrays
#undef glMultiDrawArrays
#define glMultiDrawArrays           GLMock->MultiDrawArrays

#undef glCreateShader
#define glCreateShader              GLMock->CreateShader
#undef glDeleteShader
#define glDeleteShader              GLMock->DeleteShader
#undef glShaderSource
#define glShaderSource              GLMock->ShaderSource
#undef glCompileShader
#define glCompileShader             GLMock->CompileShader
#undef glGetShaderiv
#define glGetShaderiv               GLMock->GetShaderiv
#undef glGetShaderInfoLog
#define glGetShaderInfoLog          GLMock->GetShaderInfoLog
#undef glAttachShader
#define glAttachShader              GLMock->AttachShader
#undef glDetachShader
#define glDetachShader              GLMock->DetachShader
#undef glCreateProgram

#define glCreateProgram             GLMock->CreateProgram
#undef glDeleteProgram
#define glDeleteProgram             GLMock->DeleteProgram
#undef glLinkProgram
#define glLinkProgram               GLMock->LinkProgram
#undef glGetProgramiv
#define glGetProgramiv              GLMock->GetProgramiv
#undef glGetProgramInfoLog
#define glGetProgramInfoLog         GLMock->GetProgramInfoLog
#undef glUseProgram
#define glUseProgram                GLMock->UseProgram

#undef glUniform1f
#define glUniform1f                 GLMock->Uniform1f
#undef glUniform2f
#define glUniform2f                 GLMock->Uniform2f
#undef glUniform3f
#define glUniform3f                 GLMock->Uniform3f
#undef glUniform4f
#define glUniform4f                 GLMock->Uniform4f
#undef glUniform1i
#define glUniform1i                 GLMock->Uniform1i
#undef glUniformMatrix2fv
#define glUniformMatrix2fv          GLMock->UniformMatrix2fv
#undef glUniformMatrix3fv
#define glUniformMatrix3fv          GLMock->UniformMatrix3fv
#undef glUniformMatrix4fv
#define glUniformMatrix4fv          GLMock->UniformMatrix4fv

#undef glGetUniformLocation
#define glGetUniformLocation        GLMock->GetUniformLocation

#undef glFinishObjectAPPLE
#define glFinishObjectAPPLE         GLMock->FinishObjectAPPLE

#undef glGetError
#define glGetError                  GLMock->GetError
#endif
