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

#include "gl/FontTexture.h"

#include "gl/GlInterface.h"

#include "kd/contracts.h"

#include <cassert>
#include <cstring>

namespace tb::gl
{

FontTexture::FontTexture() = default;

FontTexture::FontTexture(
  const size_t cellCount, const size_t cellSize, const size_t margin)
  : m_size{computeTextureSize(cellCount, cellSize, margin)}
  , m_buffer{std::make_unique<char[]>(m_size * m_size)}
{
  std::memset(m_buffer.get(), 0, m_size * m_size);
}

FontTexture::FontTexture(const FontTexture& other)
  : m_size{other.m_size}
  , m_buffer{std::make_unique<char[]>(m_size * m_size)}
{
  std::memcpy(m_buffer.get(), other.m_buffer.get(), m_size * m_size);
}

FontTexture::FontTexture(FontTexture&& other) = default;

FontTexture& FontTexture::operator=(FontTexture other)
{
  using std::swap;
  swap(m_size, other.m_size);
  swap(m_buffer, other.m_buffer);
  swap(m_textureId, other.m_textureId);
  return *this;
}

FontTexture::~FontTexture() = default;

size_t FontTexture::size() const
{
  return m_size;
}

void FontTexture::activate(Gl& gl)
{
  if (m_textureId == 0)
  {
    contract_assert(m_buffer != nullptr);

    gl.genTextures(1, &m_textureId);
    gl.bindTexture(GL_TEXTURE_2D, m_textureId);
    gl.texParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.texParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl.texImage2D(
      GL_TEXTURE_2D,
      0,
      GL_LUMINANCE,
      static_cast<GLsizei>(m_size),
      static_cast<GLsizei>(m_size),
      0,
      GL_LUMINANCE,
      GL_UNSIGNED_BYTE,
      m_buffer.get());
    m_buffer.release();
  }

  contract_post(m_textureId > 0);
  gl.bindTexture(GL_TEXTURE_2D, m_textureId);
}

void FontTexture::deactivate(Gl& gl)
{
  gl.bindTexture(GL_TEXTURE_2D, 0);
}

void FontTexture::destroy(Gl& gl)
{
  if (m_textureId != 0)
  {
    gl.deleteTextures(1, &m_textureId);
    m_textureId = 0;
  }
}

size_t FontTexture::computeTextureSize(
  const size_t cellCount, const size_t cellSize, const size_t margin) const
{
  const auto minTextureSize = margin + cellCount * (cellSize + margin);
  size_t textureSize = 1;
  while (textureSize < minTextureSize)
  {
    textureSize = textureSize << 1;
  }
  return textureSize;
}

} // namespace tb::gl
