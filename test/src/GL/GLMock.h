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

#ifndef TrenchBroom_GLMock_h
#define TrenchBroom_GLMock_h

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "GL/glew.h"

class CGLMock {
public:
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
};

extern CGLMock* GLMock;

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


#endif
