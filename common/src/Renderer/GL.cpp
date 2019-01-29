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
            case GL_TABLE_TOO_LARGE:
                return "GL_TABLE_TOO_LARGE";
            default:
                return "UNKNOWN";
        }
    }

    GLenum glGetEnum(const String& name) {
        if (name == "GL_ONE") {
            return GL_ONE;
        } else if (name == "GL_ZERO") {
            return GL_ZERO;
        } else if (name == "GL_SRC_COLOR") {
            return GL_SRC_COLOR;
        } else if (name == "GL_DST_COLOR") {
            return GL_DST_COLOR;
        } else if (name == "GL_ONE_MINUS_SRC_COLOR") {
            return GL_ONE_MINUS_SRC_COLOR;
        } else if (name == "GL_ONE_MINUS_DST_COLOR") {
            return GL_ONE_MINUS_DST_COLOR;
        } else if (name == "GL_SRC_ALPHA") {
            return GL_SRC_ALPHA;
        } else if (name == "GL_ONE_MINUS_SRC_ALPHA") {
            return GL_ONE_MINUS_SRC_ALPHA;
        } else {
            throw RenderException() << "Unknown GL enum: " << name;
        }
    }
}
