/*
 Copyright (C) 2010 Kristian Duske

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

#include "gl/GlUtils.h"

#include "gl/GlInterface.h"

#include "kd/contracts.h"

#include <fmt/format.h>

#include <string>

namespace tb::gl
{
namespace
{
constexpr auto EdgeOffset = 0.0001;
}

GLenum getEnum(const std::string& name)
{
  if (name == "GL_ONE")
  {
    return GL_ONE;
  }
  if (name == "GL_ZERO")
  {
    return GL_ZERO;
  }
  if (name == "GL_SRC_COLOR")
  {
    return GL_SRC_COLOR;
  }
  if (name == "GL_DST_COLOR")
  {
    return GL_DST_COLOR;
  }
  if (name == "GL_ONE_MINUS_SRC_COLOR")
  {
    return GL_ONE_MINUS_SRC_COLOR;
  }
  if (name == "GL_ONE_MINUS_DST_COLOR")
  {
    return GL_ONE_MINUS_DST_COLOR;
  }
  if (name == "GL_SRC_ALPHA")
  {
    return GL_SRC_ALPHA;
  }
  if (name == "GL_DST_ALPHA")
  {
    return GL_DST_ALPHA;
  }
  if (name == "GL_ONE_MINUS_SRC_ALPHA")
  {
    return GL_ONE_MINUS_SRC_ALPHA;
  }
  if (name == "GL_ONE_MINUS_DST_ALPHA")
  {
    return GL_ONE_MINUS_DST_ALPHA;
  }
  if (name == "GL_SRC_ALPHA_SATURATE")
  {
    return GL_SRC_ALPHA_SATURATE;
  }

  contract_assert(false);
}

std::string glGetEnumName(const GLenum enum_)
{
  switch (enum_)
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

void glSetEdgeOffset(Gl& gl, const double f)
{
  gl.depthRange(0.0, 1.0 - EdgeOffset * f);
}

void glResetEdgeOffset(Gl& gl)
{
  gl.depthRange(EdgeOffset, 1.0);
}

} // namespace tb::gl
