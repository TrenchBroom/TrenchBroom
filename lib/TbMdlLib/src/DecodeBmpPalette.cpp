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

#include "mdl/DecodeBmpPalette.h"

#include "InitFreeImage.h"
#include "base/Macros.h"

#include "kd/contracts.h"
#include "kd/resource.h"

#include <FreeImage.h>

namespace tb::mdl
{

namespace
{

std::vector<unsigned char> decodePalette(FIBITMAP* bitmap)
{
  const auto* pal = FreeImage_GetPalette(bitmap);
  contract_assert(pal != nullptr);

  const auto paletteSize = size_t(FreeImage_GetColorsUsed(bitmap));

  auto result = std::vector<unsigned char>();
  result.reserve(paletteSize * 3);
  for (size_t i = 0; i < paletteSize; ++i)
  {
    result.push_back(static_cast<unsigned char>(pal[i].rgbRed));
    result.push_back(static_cast<unsigned char>(pal[i].rgbGreen));
    result.push_back(static_cast<unsigned char>(pal[i].rgbBlue));
  }

  return result;
}

std::vector<unsigned char> decodeDirectPixels(
  FIBITMAP* bitmap, const size_t width, const size_t height)
{
  auto result = std::vector<unsigned char>(width * height * 3);
  for (unsigned y = 0; y < height; ++y)
  {
    for (unsigned x = 0; x < width; ++x)
    {
      RGBQUAD pixel;
      assertResult(FreeImage_GetPixelColor(bitmap, x, y, &pixel) == TRUE);

      const auto pixelIndex = ((height - y - 1) * width + x) * 3;
      result[pixelIndex + 0] = static_cast<unsigned char>(pixel.rgbRed);
      result[pixelIndex + 1] = static_cast<unsigned char>(pixel.rgbGreen);
      result[pixelIndex + 2] = static_cast<unsigned char>(pixel.rgbBlue);
    }
  }

  return result;
}

} // namespace

std::vector<unsigned char> decodeBmpPalette(const char* begin, const char* end)
{
  InitFreeImage::initialize();

  // this is supremely evil, but FreeImage guarantees that it will not modify wrapped
  // memory
  auto* address = reinterpret_cast<BYTE*>(const_cast<char*>(begin));
  const auto length = DWORD(end - begin);

  auto stream =
    kdl::resource{FreeImage_OpenMemory(address, length), FreeImage_CloseMemory};
  auto bitmap =
    kdl::resource{FreeImage_LoadFromMemory(FIF_BMP, *stream), FreeImage_Unload};

  if (FreeImage_GetPalette(*bitmap) != nullptr)
  {
    return decodePalette(*bitmap);
  }

  const auto width = size_t(FreeImage_GetWidth(*bitmap));
  const auto height = size_t(FreeImage_GetHeight(*bitmap));
  return decodeDirectPixels(*bitmap, width, height);
}

} // namespace tb::mdl
