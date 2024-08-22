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

#include "ReadFreeImageTexture.h"

#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Ensure.h"
#include "Error.h"
#include "FreeImage.h"
#include "IO/ImageLoaderImpl.h"
#include "IO/MaterialUtils.h"
#include "IO/Reader.h"

#include "kdl/invoke.h"
#include "kdl/resource.h"
#include "kdl/result.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

namespace
{
/**
 * The byte order of a 32bpp FIBITMAP is defined by the macros FI_RGBA_RED,
 * FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA.
 * From looking at FreeImage.h, there are only two possible orders,
 * so we can handle both possible orders and map them to the relevant GL_RGBA
 * or GL_BGRA constant.
 */
constexpr GLenum freeImage32BPPFormatToGLFormat()
{
  if constexpr (
    FI_RGBA_RED == 0 && FI_RGBA_GREEN == 1 && FI_RGBA_BLUE == 2 && FI_RGBA_ALPHA == 3)
  {

    return GL_RGBA;
  }

  if constexpr (
    FI_RGBA_BLUE == 0 && FI_RGBA_GREEN == 1 && FI_RGBA_RED == 2 && FI_RGBA_ALPHA == 3)
  {

    return GL_BGRA;
  }

  throw std::runtime_error{"Expected FreeImage to use RGBA or BGRA"};
}

} // namespace

Color getAverageColor(const Assets::TextureBuffer& buffer, const GLenum format)
{
  ensure(format == GL_RGBA || format == GL_BGRA, "format is GL_RGBA or GL_BGRA");

  const auto r = size_t(format == GL_RGBA ? 0 : 2);
  const auto g = size_t(format == GL_RGBA ? 1 : 1);
  const auto b = size_t(format == GL_RGBA ? 2 : 0);
  const auto a = size_t(3);

  const auto* const data = buffer.data();
  const auto bufferSize = buffer.size();
  const auto numPixels = bufferSize / 4;

  const auto stride = numPixels <= 4192 ? 1 : numPixels / 64;
  const auto numSamples = numPixels / stride;

  auto average = Color{};
  for (std::size_t i = 0; i < numSamples; ++i)
  {
    const auto pixel = i * 4 * stride;
    average =
      average + Color{data[pixel + r], data[pixel + g], data[pixel + b], data[pixel + a]};
  }
  average = average / static_cast<float>(numSamples);

  return average;
}

Result<Assets::Texture> readFreeImageTextureFromMemory(
  const uint8_t* begin, const size_t size)
{
  try
  {
    InitFreeImage::initialize();

    auto imageMemory = kdl::resource{
      FreeImage_OpenMemory(const_cast<uint8_t*>(begin), static_cast<DWORD>(size)),
      FreeImage_CloseMemory};

    const auto imageFormat = FreeImage_GetFileTypeFromMemory(*imageMemory);
    auto image = kdl::resource{
      FreeImage_LoadFromMemory(imageFormat, *imageMemory), FreeImage_Unload};

    if (!image)
    {
      return Error{"FreeImage could not load image data"};
    }

    const auto imageWidth = size_t(FreeImage_GetWidth(*image));
    const auto imageHeight = size_t(FreeImage_GetHeight(*image));

    if (!checkTextureDimensions(imageWidth, imageHeight))
    {
      return Error{
        fmt::format("Invalid texture dimensions: {}*{}", imageWidth, imageHeight)};
    }

    // This is supposed to indicate whether any pixels are transparent (alpha < 100%)
    const auto masked = FreeImage_IsTransparent(*image);

    constexpr auto mipCount = 1u;
    constexpr auto format = freeImage32BPPFormatToGLFormat();

    auto buffers = Assets::TextureBufferList{mipCount};
    Assets::setMipBufferSize(buffers, mipCount, imageWidth, imageHeight, format);

    if (
      FreeImage_GetColorType(*image) != FIC_RGBALPHA
      || FreeImage_GetLine(*image) / FreeImage_GetWidth(*image) != 4)
    {
      image = FreeImage_ConvertTo32Bits(*image);
    }

    if (!image)
    {
      return Error{"Unsupported pixel format"};
    }

    assert(FreeImage_GetLine(*image) / FreeImage_GetWidth(*image) == 4);

    auto* outBytes = buffers.at(0).data();
    const auto outBytesPerRow = int(imageWidth * 4);

    FreeImage_ConvertToRawBits(
      outBytes,
      *image,
      outBytesPerRow,
      32,
      FI_RGBA_RED_MASK,
      FI_RGBA_GREEN_MASK,
      FI_RGBA_BLUE_MASK,
      TRUE);


    const auto textureMask = masked ? Assets::TextureMask::On : Assets::TextureMask::Off;
    const auto averageColor = getAverageColor(buffers.at(0), format);

    return Assets::Texture{
      imageWidth,
      imageHeight,
      averageColor,
      format,
      textureMask,
      Assets::NoEmbeddedDefaults{},
      std::move(buffers)};
  }
  catch (const std::exception& e)
  {
    return Error{e.what()};
  }
}

Result<Assets::Texture> readFreeImageTexture(Reader& reader)
{
  auto bufferedReader = reader.buffer();
  const auto* begin = bufferedReader.begin();
  const auto* end = bufferedReader.end();
  const auto imageSize = size_t(end - begin);
  auto* imageBegin = reinterpret_cast<BYTE*>(const_cast<char*>(begin));

  return readFreeImageTextureFromMemory(imageBegin, imageSize);
}

namespace
{
std::vector<std::string> getSupportedFreeImageExtensions()
{
  auto result = std::vector<std::string>{};

  const auto count = FreeImage_GetFIFCount();
  assert(count >= 0);

  for (int i = 0; i < count; ++i)
  {
    const auto format = static_cast<FREE_IMAGE_FORMAT>(i);
    if (FreeImage_IsPluginEnabled(format))
    {
      const auto extensionListStr =
        kdl::str_to_lower(std::string{FreeImage_GetFIFExtensionList(format)});
      result = kdl::vec_concat(
        std::move(result),
        kdl::vec_transform(
          kdl::str_split(extensionListStr, ","),
          [](const auto& extension) { return "." + extension; }));
    }
  }

  return result;
}
} // namespace

bool isSupportedFreeImageExtension(const std::string_view extension)
{
  InitFreeImage::initialize();

  static const auto extensions = getSupportedFreeImageExtensions();
  return kdl::vec_contains(extensions, kdl::str_to_lower(extension));
}

} // namespace TrenchBroom::IO
