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

#include "vm/vec.h"

#include <FreeImage.h>
#include <algorithm>
#include <iostream>

namespace TrenchBroom::Assets
{

TextureBuffer::TextureBuffer() = default;

/**
 * Note, buffer is created defult-initialized (i.e., uninitialized) on purpose.
 */
TextureBuffer::TextureBuffer(const size_t size)
  : m_buffer{new unsigned char[size]}
  , m_size{size}
{
}

const unsigned char* TextureBuffer::data() const
{
  return m_buffer.get();
}

unsigned char* TextureBuffer::data()
{
  return m_buffer.get();
}

size_t TextureBuffer::size() const
{
  return m_size;
}

std::ostream& operator<<(std::ostream& lhs, const TextureBuffer& rhs)
{
  lhs << "TextureBuffer{" << rhs.size() << " bytes}";
  return lhs;
}

vm::vec2s sizeAtMipLevel(const size_t width, const size_t height, const size_t level)
{
  assert(width > 0);
  assert(height > 0);

  // from Issues 6 in:
  // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_non_power_of_two.txt
  return vm::vec2s{
    std::max(size_t(1), width >> level), std::max(size_t(1), height >> level)};
}

bool isCompressedFormat(const GLenum format)
{
  return format >= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
         && format <= GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
}

size_t blockSizeForFormat(const GLenum format)
{
  switch (format)
  {
  case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    return 8U;
  case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
  case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
    return 16U;
  }
  ensure(false, "unknown compressed format");
  return 0U;
}

size_t bytesPerPixelForFormat(const GLenum format)
{
  switch (format)
  {
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
  TextureBufferList& buffers,
  const size_t mipLevels,
  const size_t width,
  const size_t height,
  const GLenum format)
{
  const auto compressed = isCompressedFormat(format);
  const auto bytesPerPixel = compressed ? 0U : bytesPerPixelForFormat(format);
  const auto blockSize = compressed ? blockSizeForFormat(format) : 0U;

  buffers.resize(mipLevels);
  for (size_t level = 0u; level < buffers.size(); ++level)
  {
    const auto mipSize = sizeAtMipLevel(width, height, level);
    const auto numBytes = compressed ? (
                            blockSize * std::max(size_t(1), mipSize.x() / 4)
                            * std::max(size_t(1), mipSize.y() / 4))
                                     : (bytesPerPixel * mipSize.x() * mipSize.y());
    buffers[level] = TextureBuffer{numBytes};
  }
}

void resizeMips(
  TextureBufferList& buffers, const vm::vec2s& oldSize, const vm::vec2s& newSize)
{
  if (oldSize != newSize)
  {
    for (size_t i = 0; i < buffers.size(); ++i)
    {
      const auto div = size_t(1) << i;
      const auto oldWidth = int(oldSize.x() / div);
      const auto oldHeight = int(oldSize.y() / div);
      const auto oldPitch = oldWidth * 3;
      auto* oldPtr = buffers[i].data();

      auto* oldBitmap = FreeImage_ConvertFromRawBits(
        oldPtr, oldWidth, oldHeight, oldPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
      ensure(oldBitmap != nullptr, "oldBitmap is null");

      const auto newWidth = int(newSize.x() / div);
      const auto newHeight = int(newSize.y() / div);
      const auto newPitch = newWidth * 3;
      auto* newBitmap = FreeImage_Rescale(oldBitmap, newWidth, newHeight, FILTER_BICUBIC);
      ensure(newBitmap != nullptr, "newBitmap is null");

      buffers[i] = TextureBuffer{3 * newSize.x() * newSize.y()};
      auto* newPtr = buffers[i].data();

      FreeImage_ConvertToRawBits(
        newPtr, newBitmap, newPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
      FreeImage_Unload(oldBitmap);
      FreeImage_Unload(newBitmap);
    }
  }
}
} // namespace TrenchBroom::Assets
