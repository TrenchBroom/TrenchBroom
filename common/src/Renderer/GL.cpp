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

#include "Exceptions.h"

#include <string>

namespace TrenchBroom
{
void glCheckError(const std::string& msg)
{
  const GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    throw RenderException(
      "OpenGL error: " + std::to_string(error) + " (" + glGetErrorMessage(error) + ") "
      + msg);
  }
}

std::string glGetErrorMessage(const GLenum code)
{
  switch (code)
  {
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

GLenum glGetEnum(const std::string& name)
{
  if (name == "GL_ONE")
  {
    return GL_ONE;
  }
  else if (name == "GL_ZERO")
  {
    return GL_ZERO;
  }
  else if (name == "GL_SRC_COLOR")
  {
    return GL_SRC_COLOR;
  }
  else if (name == "GL_DST_COLOR")
  {
    return GL_DST_COLOR;
  }
  else if (name == "GL_ONE_MINUS_SRC_COLOR")
  {
    return GL_ONE_MINUS_SRC_COLOR;
  }
  else if (name == "GL_ONE_MINUS_DST_COLOR")
  {
    return GL_ONE_MINUS_DST_COLOR;
  }
  else if (name == "GL_SRC_ALPHA")
  {
    return GL_SRC_ALPHA;
  }
  else if (name == "GL_DST_ALPHA")
  {
    return GL_DST_ALPHA;
  }
  else if (name == "GL_ONE_MINUS_SRC_ALPHA")
  {
    return GL_ONE_MINUS_SRC_ALPHA;
  }
  else if (name == "GL_ONE_MINUS_DST_ALPHA")
  {
    return GL_ONE_MINUS_DST_ALPHA;
  }
  else if (name == "GL_SRC_ALPHA_SATURATE")
  {
    return GL_SRC_ALPHA_SATURATE;
  }
  else
  {
    throw RenderException("Unknown GL enum: " + name);
  }
}

std::string glGetEnumName(const GLenum _enum)
{
  switch (_enum)
  {
  case GL_ONE:
    return "GL_ONE";
  case GL_ZERO:
    return "GL_ZERO";
  case GL_SRC_COLOR:
    return "GL_SRC_COLOR";
  case GL_DST_COLOR:
    return "GL_DST_COLOR";
  case GL_ONE_MINUS_SRC_COLOR:
    return "GL_ONE_MINUS_SRC_COLOR";
  case GL_ONE_MINUS_DST_COLOR:
    return "GL_ONE_MINUS_DST_COLOR";
  case GL_SRC_ALPHA:
    return "GL_SRC_ALPHA";
  case GL_DST_ALPHA:
    return "GL_DST_ALPHA";
  case GL_ONE_MINUS_SRC_ALPHA:
    return "GL_ONE_MINUS_SRC_ALPHA";
  case GL_ONE_MINUS_DST_ALPHA:
    return "GL_ONE_MINUS_DST_ALPHA";
  case GL_SRC_ALPHA_SATURATE:
    return "GL_SRC_ALPHA_SATURATE";
  default:
    return "Unknown OpenGL enum";
  }
}
} // namespace TrenchBroom
