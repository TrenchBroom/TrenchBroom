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

#include "TextureBuffer.h"

#include "Ensure.h"

#include <vecmath/vec.h>

#include <FreeImage.h>

#include <algorithm> // for std::max

namespace TrenchBroom {
namespace Assets {
TextureBuffer::TextureBuffer()
  : m_buffer()
  , m_size(0) {}

/**
 * Note, buffer is created defult-initialized (i.e., uninitialized) on purpose.
 */
TextureBuffer::TextureBuffer(const size_t size)
  : m_buffer(new unsigned char[size])
  , m_size(size) {}

const unsigned char* TextureBuffer::data() const {
  return m_buffer.get();
}

unsigned char* TextureBuffer::data() {
  return m_buffer.get();
}

size_t TextureBuffer::size() const {
  return m_size;
}

vm::vec2s sizeAtMipLevel(const size_t width, const size_t height, const size_t level) {
  assert(width > 0);
  assert(height > 0);

  // from Issues 6 in:
  // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_non_power_of_two.txt
  return vm::vec2s(std::max(size_t(1), width >> level), std::max(size_t(1), height >> level));
}

size_t bytesPerPixelForFormat(const GLenum format) {
  switch (format) {
    case GL_RGB:
    case GL_BGR:
      return 3U;
    case GL_RGBA:
    case GL_BGRA:
      return 4U;
  }
  ensure(false, "unknown format");
  return 0U;
}

void setMipBufferSize(
  TextureBufferList& buffers, const size_t mipLevels, const size_t width, const size_t height,
  const GLenum format) {
  const size_t bytesPerPixel = bytesPerPixelForFormat(format);

  buffers.resize(mipLevels);
  for (size_t level = 0u; level < buffers.size(); ++level) {
    const auto mipSize = sizeAtMipLevel(width, height, level);
    const auto numBytes = bytesPerPixel * mipSize.x() * mipSize.y();
    buffers[level] = TextureBuffer(numBytes);
  }
}

void resizeMips(TextureBufferList& buffers, const vm::vec2s& oldSize, const vm::vec2s& newSize) {
  if (oldSize == newSize)
    return;

  for (size_t i = 0; i < buffers.size(); ++i) {
    const auto div = size_t(1) << i;
    const auto oldWidth = static_cast<int>(oldSize.x() / div);
    const auto oldHeight = static_cast<int>(oldSize.y() / div);
    const auto oldPitch = oldWidth * 3;
    auto* oldPtr = buffers[i].data();

    auto* oldBitmap = FreeImage_ConvertFromRawBits(
      oldPtr, oldWidth, oldHeight, oldPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
    ensure(oldBitmap != nullptr, "oldBitmap is null");

    const auto newWidth = static_cast<int>(newSize.x() / div);
    const auto newHeight = static_cast<int>(newSize.y() / div);
    const auto newPitch = newWidth * 3;
    auto* newBitmap = FreeImage_Rescale(oldBitmap, newWidth, newHeight, FILTER_BICUBIC);
    ensure(newBitmap != nullptr, "newBitmap is null");

    buffers[i] = TextureBuffer(3 * newSize.x() * newSize.y());
    auto* newPtr = buffers[i].data();

    FreeImage_ConvertToRawBits(newPtr, newBitmap, newPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
    FreeImage_Unload(oldBitmap);
    FreeImage_Unload(newBitmap);
  }
}
} // namespace Assets
} // namespace TrenchBroom
