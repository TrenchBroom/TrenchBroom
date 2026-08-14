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

#include "mdl/ImageLoaderImpl.h"

#include "InitFreeImage.h"
#include "base/Macros.h"

#include "kd/contracts.h"

namespace tb::mdl
{

ImageLoaderImpl::ImageLoaderImpl(const char* begin, const char* end)
{
  InitFreeImage::initialize();

  // this is supremely evil, but FreeImage guarantees that it will not modify wrapped
  // memory
  auto* address = reinterpret_cast<BYTE*>(const_cast<char*>(begin));
  auto length = DWORD(end - begin);
  m_stream = FreeImage_OpenMemory(address, length);
  m_bitmap = FreeImage_LoadFromMemory(FIF_BMP, m_stream);
}

ImageLoaderImpl::~ImageLoaderImpl()
{
  if (m_bitmap)
  {
    FreeImage_Unload(m_bitmap);
    m_bitmap = nullptr;
  }
  if (m_stream)
  {
    FreeImage_CloseMemory(m_stream);
    m_stream = nullptr;
  }
}

size_t ImageLoaderImpl::paletteSize() const
{
  return size_t(FreeImage_GetColorsUsed(m_bitmap));
}

size_t ImageLoaderImpl::width() const
{
  return size_t(FreeImage_GetWidth(m_bitmap));
}

size_t ImageLoaderImpl::height() const
{
  return size_t(FreeImage_GetHeight(m_bitmap));
}

bool ImageLoaderImpl::hasPalette() const
{
  return FreeImage_GetPalette(m_bitmap) != nullptr;
}

bool ImageLoaderImpl::hasIndices() const
{
  return FreeImage_GetColorType(m_bitmap) == FIC_PALETTE;
}

bool ImageLoaderImpl::hasPixels() const
{
  return static_cast<bool>(FreeImage_HasPixels(m_bitmap) == TRUE);
}

std::vector<unsigned char> ImageLoaderImpl::loadPalette() const
{
  contract_pre(hasPalette());

  const auto* pal = FreeImage_GetPalette(m_bitmap);
  if (!pal)
  {
    return {};
  }

  auto result = std::vector<unsigned char>();
  result.reserve(paletteSize() * 3);
  for (size_t i = 0; i < paletteSize(); ++i)
  {
    result.push_back(static_cast<unsigned char>(pal[i].rgbRed));
    result.push_back(static_cast<unsigned char>(pal[i].rgbGreen));
    result.push_back(static_cast<unsigned char>(pal[i].rgbBlue));
  }

  return result;
}

std::vector<unsigned char> ImageLoaderImpl::loadPixels() const
{
  contract_pre(hasPixels());

  return hasIndices() ? loadIndexedPixels() : loadDirectPixels();
}

std::vector<unsigned char> ImageLoaderImpl::loadIndexedPixels() const
{
  const auto* palette = FreeImage_GetPalette(m_bitmap);
  contract_assert(palette != nullptr);

  auto result = std::vector<unsigned char>(width() * height() * 3);
  for (unsigned y = 0; y < height(); ++y)
  {
    for (unsigned x = 0; x < width(); ++x)
    {
      BYTE paletteIndex = 0;
      assertResult(FreeImage_GetPixelIndex(m_bitmap, x, y, &paletteIndex) == TRUE);
      contract_assert(paletteIndex < paletteSize());

      const auto pixelIndex = ((height() - y - 1) * width() + x) * 3;
      result[pixelIndex + 0] = static_cast<unsigned char>(palette[paletteIndex].rgbRed);
      result[pixelIndex + 1] = static_cast<unsigned char>(palette[paletteIndex].rgbGreen);
      result[pixelIndex + 2] = static_cast<unsigned char>(palette[paletteIndex].rgbBlue);
    }
  }
  return result;
}

std::vector<unsigned char> ImageLoaderImpl::loadDirectPixels() const
{
  auto result = std::vector<unsigned char>(width() * height() * 3);
  for (unsigned y = 0; y < height(); ++y)
  {
    for (unsigned x = 0; x < width(); ++x)
    {
      RGBQUAD pixel;
      assertResult(FreeImage_GetPixelColor(m_bitmap, x, y, &pixel) == TRUE);

      const auto pixelIndex = ((height() - y - 1) * width() + x) * 3;
      result[pixelIndex + 0] = static_cast<unsigned char>(pixel.rgbRed);
      result[pixelIndex + 1] = static_cast<unsigned char>(pixel.rgbGreen);
      result[pixelIndex + 2] = static_cast<unsigned char>(pixel.rgbBlue);
    }
  }

  return result;
}

} // namespace tb::mdl
