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

#include "FontGlyph.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom
{
namespace Renderer
{
FontGlyph::FontGlyph(
  const size_t x, const size_t y, const size_t w, const size_t h, const size_t a)
  : m_x(static_cast<float>(x))
  , m_y(static_cast<float>(y))
  , m_w(static_cast<float>(w))
  , m_h(static_cast<float>(h))
  , m_a(static_cast<int>(a))
{
}

void FontGlyph::appendVertices(
  std::vector<vm::vec2f>& vertices,
  const int xOffset,
  const int yOffset,
  const size_t textureSize,
  const bool clockwise) const
{
  const auto fxOffset = static_cast<float>(xOffset);
  const auto fyOffset = static_cast<float>(yOffset);
  const auto ftextureSize = static_cast<float>(textureSize);

  if (clockwise)
  {
    vertices.push_back(vm::vec2f(fxOffset, fyOffset));
    vertices.push_back(vm::vec2f(m_x, m_y + m_h) / ftextureSize);

    vertices.push_back(vm::vec2f(fxOffset, fyOffset + m_h));
    vertices.push_back(vm::vec2f(m_x, m_y) / ftextureSize);

    vertices.push_back(vm::vec2f(fxOffset + m_w, fyOffset + m_h));
    vertices.push_back(vm::vec2f(m_x + m_w, m_y) / ftextureSize);

    vertices.push_back(vm::vec2f(fxOffset + m_w, fyOffset));
    vertices.push_back(vm::vec2f(m_x + m_w, m_y + m_h) / ftextureSize);
  }
  else
  {
    vertices.push_back(vm::vec2f(fxOffset, fyOffset));
    vertices.push_back(vm::vec2f(m_x, m_y + m_h) / ftextureSize);

    vertices.push_back(vm::vec2f(fxOffset + m_w, fyOffset));
    vertices.push_back(vm::vec2f(m_x + m_w, m_y + m_h) / ftextureSize);

    vertices.push_back(vm::vec2f(fxOffset + m_w, fyOffset + m_h));
    vertices.push_back(vm::vec2f(m_x + m_w, m_y) / ftextureSize);

    vertices.push_back(vm::vec2f(fxOffset, fyOffset + m_h));
    vertices.push_back(vm::vec2f(m_x, m_y) / ftextureSize);
  }
}

int FontGlyph::advance() const
{
  return m_a;
}
} // namespace Renderer
} // namespace TrenchBroom
