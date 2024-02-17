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

#include "Renderer/GL.h"

#include "vecmath/forward.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace Assets
{
class TextureBuffer
{
private:
  std::unique_ptr<unsigned char[]> m_buffer;
  size_t m_size;

public:
  explicit TextureBuffer();
  explicit TextureBuffer(size_t size);

  const unsigned char* data() const;
  unsigned char* data();

  size_t size() const;
};
using TextureBufferList = std::vector<TextureBuffer>;

vm::vec2s sizeAtMipLevel(size_t width, size_t height, size_t level);
bool isCompressedFormat(GLenum format);
size_t blockSizeForFormat(GLenum format);
size_t bytesPerPixelForFormat(GLenum format);
void setMipBufferSize(
  TextureBufferList& buffers,
  size_t mipLevels,
  size_t width,
  size_t height,
  GLenum format);

void resizeMips(
  TextureBufferList& buffers, const vm::vec2s& oldSize, const vm::vec2s& newSize);
} // namespace Assets
} // namespace TrenchBroom
