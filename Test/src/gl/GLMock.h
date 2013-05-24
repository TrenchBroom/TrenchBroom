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
};

extern CGLMock GLMock;

#undef glGenBuffers
#define glGenBuffers        GLMock.GenBuffers
#undef glDeleteBuffers
#define glDeleteBuffers     GLMock.DeleteBuffers
#undef glBindBuffer
#define glBindBuffer        GLMock.BindBuffer
#undef glBufferData
#define glBufferData        GLMock.BufferData

#endif
