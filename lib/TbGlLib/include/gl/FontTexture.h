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

#pragma once

#include "gl/GlUtils.h"

#include <memory>

namespace tb::gl
{
class FontGlyphBuilder;
class Gl;

class FontTexture
{
private:
  size_t m_size = 0;
  std::unique_ptr<char[]> m_buffer;
  GLuint m_textureId = 0;

  friend class FontGlyphBuilder;

public:
  FontTexture();
  FontTexture(size_t cellCount, size_t cellSize, size_t margin);
  FontTexture(const FontTexture& other);
  FontTexture(FontTexture&& other);
  FontTexture& operator=(FontTexture other);
  ~FontTexture();

  size_t size() const;

  void activate(Gl& gl);
  void deactivate(Gl& gl);

  void destroy(Gl& gl);

private:
  size_t computeTextureSize(size_t cellCount, size_t cellSize, size_t margin) const;
};

} // namespace tb::gl
