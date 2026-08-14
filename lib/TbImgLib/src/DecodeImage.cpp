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

#include "img/DecodeImage.h"

#include "FreeImage.h"
#include "InitFreeImage.h"

#include "kd/contracts.h"
#include "kd/resource.h"

#include <fmt/format.h>

#include <stdexcept>
#include <utility>

namespace tb::img
{

namespace
{

/**
 * The byte order of a 32bpp FIBITMAP is defined by the macros FI_RGBA_RED,
 * FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA, and is a platform-dependent build
 * setting of the FreeImage library, not a runtime property of any given image. From
 * looking at FreeImage.h, there are only two possible orders; this reports whether the
 * native order is BGRA rather than RGBA, so callers can normalize to canonical RGBA.
 */
constexpr bool nativeOrderIsBgra()
{
  if constexpr (
    FI_RGBA_RED == 0 && FI_RGBA_GREEN == 1 && FI_RGBA_BLUE == 2 && FI_RGBA_ALPHA == 3)
  {
    return false;
  }

  if constexpr (
    FI_RGBA_BLUE == 0 && FI_RGBA_GREEN == 1 && FI_RGBA_RED == 2 && FI_RGBA_ALPHA == 3)
  {
    return true;
  }

  throw std::runtime_error{"Expected FreeImage to use RGBA or BGRA"};
}

} // namespace

Result<Image> decodeImage(const unsigned char* begin, const size_t size)
{
  try
  {
    InitFreeImage::initialize();

    auto imageMemory = kdl::resource{
      FreeImage_OpenMemory(const_cast<unsigned char*>(begin), static_cast<DWORD>(size)),
      FreeImage_CloseMemory};

    if (!imageMemory)
    {
      return Error{"FreeImage could not open memory"};
    }

    const auto imageFormat = FreeImage_GetFileTypeFromMemory(*imageMemory);
    if (imageFormat == FIF_UNKNOWN)
    {
      return Error{"Unknown image format"};
    }

    auto bitmap = kdl::resource{
      FreeImage_LoadFromMemory(imageFormat, *imageMemory), FreeImage_Unload};

    if (!bitmap)
    {
      return Error{"FreeImage could not load image data"};
    }

    const auto width = size_t(FreeImage_GetWidth(*bitmap));
    const auto height = size_t(FreeImage_GetHeight(*bitmap));

    if (width == 0 || height == 0)
    {
      return Error{fmt::format("Invalid image dimensions: {}*{}", width, height)};
    }

    // This is supposed to indicate whether any pixels are transparent (alpha < 100%)
    const auto hasTransparency = FreeImage_IsTransparent(*bitmap) == TRUE;

    if (
      FreeImage_GetColorType(*bitmap) != FIC_RGBALPHA
      || FreeImage_GetLine(*bitmap) / FreeImage_GetWidth(*bitmap) != 4)
    {
      bitmap = FreeImage_ConvertTo32Bits(*bitmap);
    }

    if (!bitmap)
    {
      return Error{"Unsupported pixel format"};
    }

    contract_assert(FreeImage_GetLine(*bitmap) / FreeImage_GetWidth(*bitmap) == 4);

    auto pixels = std::vector<unsigned char>(width * height * 4);
    const auto outBytesPerRow = int(width * 4);

    FreeImage_ConvertToRawBits(
      pixels.data(),
      *bitmap,
      outBytesPerRow,
      32,
      FI_RGBA_RED_MASK,
      FI_RGBA_GREEN_MASK,
      FI_RGBA_BLUE_MASK,
      TRUE);

    if constexpr (nativeOrderIsBgra())
    {
      for (size_t i = 0; i < pixels.size(); i += 4)
      {
        std::swap(pixels[i], pixels[i + 2]);
      }
    }

    return Image{width, height, std::move(pixels), hasTransparency};
  }
  catch (const std::exception& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::img
