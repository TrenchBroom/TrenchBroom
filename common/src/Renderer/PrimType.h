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

#pragma once

#include <cstddef>

namespace TrenchBroom {
namespace Renderer {
enum class PrimType
{
  Points,
  Lines,
  Triangles,
  Quads,
  LineStrip,
  LineLoop,
  TriangleFan,
  TriangleStrip,
  QuadStrip,
  Polygon
};

constexpr std::size_t PrimTypeCount = 10u;
constexpr PrimType PrimTypeValues[PrimTypeCount] = {
  PrimType::Points,    PrimType::Lines,    PrimType::Triangles,   PrimType::Quads,
  PrimType::LineStrip, PrimType::LineLoop, PrimType::TriangleFan, PrimType::TriangleStrip,
  PrimType::QuadStrip, PrimType::Polygon};

/**
 * Maps the given primitive type to its corresponding OpenGL enum.
 */
unsigned int toGL(PrimType primType);
} // namespace Renderer
} // namespace TrenchBroom
