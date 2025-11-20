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

#include "FontGlyphBuilder.h"

#include "render/FontGlyph.h"
#include "render/FontTexture.h"

#include "kd/contracts.h"

#include <cassert>
#include <cstring>

namespace tb::render
{

FontGlyphBuilder::FontGlyphBuilder(
  const size_t maxAscend, size_t cellSize, const size_t margin, FontTexture& texture)
  : m_maxAscend{maxAscend}
  , m_cellSize{cellSize}
  , m_margin{margin}
  , m_textureSize{texture.m_size}
  , m_textureBuffer{texture.m_buffer.get()}
  , m_x{m_margin}
  , m_y{m_margin}
{
  contract_pre(m_textureBuffer != nullptr);
}

FontGlyph FontGlyphBuilder::createGlyph(
  const size_t left,
  const size_t top,
  const size_t width,
  const size_t height,
  const size_t advance,
  const char* glyphBuffer,
  const size_t pitch)
{

  if (m_x + m_cellSize + m_margin > m_textureSize)
  {
    m_x = m_margin;
    m_y += m_cellSize + m_margin;
  }

  drawGlyph(left, top, width, height, glyphBuffer, pitch);
  const auto glyph = FontGlyph{m_x, m_y, m_cellSize, m_cellSize, advance};
  m_x += m_cellSize + m_margin;
  return glyph;
}

void FontGlyphBuilder::drawGlyph(
  const size_t left,
  const size_t top,
  const size_t width,
  const size_t height,
  const char* glyphBuffer,
  const size_t pitch)
{
  const auto x = m_x + left;
  const auto y = m_y + m_maxAscend - top;

  for (size_t r = 0; r < height; ++r)
  {
    const auto index = (r + y) * m_textureSize + x;
    assert(index + width < m_textureSize * m_textureSize);
    std::memcpy(m_textureBuffer + index, glyphBuffer + r * pitch, width);
  }
}

} // namespace tb::render
