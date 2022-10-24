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

#include "PrimType.h"

#include "Macros.h"
#include "Renderer/GL.h"

namespace TrenchBroom
{
namespace Renderer
{
unsigned int toGL(const PrimType primType)
{
  switch (primType)
  {
  case PrimType::Points:
    return GL_POINTS;
  case PrimType::Lines:
    return GL_LINES;
  case PrimType::Triangles:
    return GL_TRIANGLES;
  case PrimType::Quads:
    return GL_QUADS;
  case PrimType::LineStrip:
    return GL_LINE_STRIP;
  case PrimType::LineLoop:
    return GL_LINE_LOOP;
  case PrimType::TriangleFan:
    return GL_TRIANGLE_FAN;
  case PrimType::TriangleStrip:
    return GL_TRIANGLE_STRIP;
  case PrimType::QuadStrip:
    return GL_QUAD_STRIP;
  case PrimType::Polygon:
    return GL_POLYGON;
    switchDefault();
  }
}
} // namespace Renderer
} // namespace TrenchBroom
