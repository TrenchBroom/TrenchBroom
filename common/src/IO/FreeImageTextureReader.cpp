/*
 Copyright (C) 2010-2016 Kristian Duske

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

#include "FreeImageTextureReader.h"

#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Ensure.h"
#include "Exceptions.h"
#include "FreeImage.h"
#include "IO/File.h"
#include "IO/ImageLoaderImpl.h"

#include <kdl/invoke.h>

#include <stdexcept>

namespace TrenchBroom
{
namespace IO
{

Color FreeImageTextureReader::getAverageColor(
  const Assets::TextureBuffer& buffer, const GLenum format)
{
  ensure(format == GL_RGBA || format == GL_BGRA, "expected RGBA or BGRA");

  const unsigned char* const data = buffer.data();
  const std::size_t bufferSize = buffer.size();

  Color average;
  for (std::size_t i = 0; i < bufferSize; i += 4)
  {
    average = average + Color(data[i], data[i + 1], data[i + 2], data[i + 3]);
  }
  const std::size_t numPixels = bufferSize / 4;
  average = average / static_cast<float>(numPixels);

  return average;
}

/**
 * The byte order of a 32bpp FIBITMAP is defined by the macros FI_RGBA_RED,
 * FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA.
 * From looking at FreeImage.h, there are only two possible orders,
 * so we can handle both possible orders and map them to the relevant GL_RGBA
 * or GL_BGRA constant.
 */
static constexpr GLenum freeImage32BPPFormatToGLFormat()
{
  if constexpr (
    FI_RGBA_RED == 0 && FI_RGBA_GREEN == 1 && FI_RGBA_BLUE == 2 && FI_RGBA_ALPHA == 3)
  {

    return GL_RGBA;
  }
  else if constexpr (
    FI_RGBA_BLUE == 0 && FI_RGBA_GREEN == 1 && FI_RGBA_RED == 2 && FI_RGBA_ALPHA == 3)
  {

    return GL_BGRA;
  }
  else
  {
    throw std::runtime_error("Expected FreeImage to use RGBA or BGRA");
  }
}

Assets::Texture FreeImageTextureReader::readTextureFromMemory(
  const std::string& name, const uint8_t* begin, const size_t size)
{
  InitFreeImage::initialize();

  auto* imageMemory =
    FreeImage_OpenMemory(const_cast<uint8_t*>(begin), static_cast<DWORD>(size));
  auto memoryGuard = kdl::invoke_later{[&]() { FreeImage_CloseMemory(imageMemory); }};

  const auto imageFormat = FreeImage_GetFileTypeFromMemory(imageMemory);
  auto* image = FreeImage_LoadFromMemory(imageFormat, imageMemory);
  auto imageGuard = kdl::invoke_later{[&]() { FreeImage_Unload(image); }};

  if (image == nullptr)
  {
    throw AssetException("FreeImage could not load image data");
  }

  const auto imageWidth = static_cast<size_t>(FreeImage_GetWidth(image));
  const auto imageHeight = static_cast<size_t>(FreeImage_GetHeight(image));

  if (!checkTextureDimensions(imageWidth, imageHeight))
  {
    throw AssetException("Invalid texture dimensions");
  }

  // This is supposed to indicate whether any pixels are transparent (alpha < 100%)
  const auto masked = FreeImage_IsTransparent(image);

  constexpr auto mipCount = 1u;
  constexpr auto format = freeImage32BPPFormatToGLFormat();

  auto buffers = Assets::TextureBufferList{mipCount};
  Assets::setMipBufferSize(buffers, mipCount, imageWidth, imageHeight, format);

  if (
    FreeImage_GetColorType(image) != FIC_RGBALPHA
    || FreeImage_GetLine(image) / FreeImage_GetWidth(image) != 4)
  {
    FreeImage_Unload(std::exchange(image, FreeImage_ConvertTo32Bits(image)));
  }

  if (image == nullptr)
  {
    throw AssetException("Unsupported pixel format");
  }

  ensure(
    FreeImage_GetLine(image) / FreeImage_GetWidth(image) == 4,
    "expected to have converted image to 32-bit");

  auto* outBytes = buffers.at(0).data();
  const auto outBytesPerRow = static_cast<int>(imageWidth * 4);

  FreeImage_ConvertToRawBits(
    outBytes,
    image,
    outBytesPerRow,
    32,
    FI_RGBA_RED_MASK,
    FI_RGBA_GREEN_MASK,
    FI_RGBA_BLUE_MASK,
    TRUE);

  const auto textureType = Assets::Texture::selectTextureType(masked);
  const Color averageColor = getAverageColor(buffers.at(0), format);

  return Assets::Texture{
    name, imageWidth, imageHeight, averageColor, std::move(buffers), format, textureType};
}

FreeImageTextureReader::FreeImageTextureReader(
  const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger)
  : TextureReader(nameStrategy, fs, logger)
{
}

Assets::Texture FreeImageTextureReader::doReadTexture(std::shared_ptr<File> file) const
{
  auto reader = file->reader().buffer();

  InitFreeImage::initialize();

  const auto& path = file->path();
  const auto* begin = reader.begin();
  const auto* end = reader.end();
  const auto imageSize = static_cast<size_t>(end - begin);
  auto* imageBegin = reinterpret_cast<BYTE*>(const_cast<char*>(begin));

  return readTextureFromMemory(textureName(path), imageBegin, imageSize);
}
} // namespace IO
} // namespace TrenchBroom
