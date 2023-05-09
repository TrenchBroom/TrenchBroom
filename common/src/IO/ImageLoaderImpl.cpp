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

#include "ImageLoaderImpl.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "Macros.h"

namespace TrenchBroom
{
namespace IO
{
InitFreeImage::InitFreeImage()
{
  FreeImage_Initialise(true);
}

InitFreeImage::~InitFreeImage()
{
  FreeImage_DeInitialise();
}

void InitFreeImage::initialize()
{
  static InitFreeImage initFreeImage;
}

ImageLoaderImpl::ImageLoaderImpl(
  const ImageLoader::Format format, const std::filesystem::path& path)
  : m_stream(nullptr)
  , m_bitmap(nullptr)
{
  InitFreeImage::initialize();
  const FREE_IMAGE_FORMAT fifFormat = translateFormat(format);
  if (fifFormat == FIF_UNKNOWN)
  {
    throw FileFormatException("Unknown image format");
  }

  m_bitmap = FreeImage_Load(fifFormat, path.string().c_str());
}

ImageLoaderImpl::ImageLoaderImpl(
  const ImageLoader::Format format, const char* begin, const char* end)
  : m_stream(nullptr)
  , m_bitmap(nullptr)
{
  InitFreeImage::initialize();
  const FREE_IMAGE_FORMAT fifFormat = translateFormat(format);
  if (fifFormat == FIF_UNKNOWN)
  {
    throw FileFormatException("Unknown image format");
  }

  // this is supremely evil, but FreeImage guarantees that it will not modify wrapped
  // memory
  BYTE* address = reinterpret_cast<BYTE*>(const_cast<char*>(begin));
  DWORD length = static_cast<DWORD>(end - begin);
  m_stream = FreeImage_OpenMemory(address, length);
  m_bitmap = FreeImage_LoadFromMemory(fifFormat, m_stream);
}

ImageLoaderImpl::~ImageLoaderImpl()
{
  if (m_bitmap != nullptr)
  {
    FreeImage_Unload(m_bitmap);
    m_bitmap = nullptr;
  }
  if (m_stream != nullptr)
  {
    FreeImage_CloseMemory(m_stream);
    m_stream = nullptr;
  }
}

size_t ImageLoaderImpl::paletteSize() const
{
  return static_cast<size_t>(FreeImage_GetColorsUsed(m_bitmap));
}

size_t ImageLoaderImpl::bitsPerPixel() const
{
  return static_cast<size_t>(FreeImage_GetBPP(m_bitmap));
}

size_t ImageLoaderImpl::width() const
{
  return static_cast<size_t>(FreeImage_GetWidth(m_bitmap));
}

size_t ImageLoaderImpl::height() const
{
  return static_cast<size_t>(FreeImage_GetHeight(m_bitmap));
}

size_t ImageLoaderImpl::byteWidth() const
{
  return static_cast<size_t>(FreeImage_GetLine(m_bitmap));
}

size_t ImageLoaderImpl::scanWidth() const
{
  return static_cast<size_t>(FreeImage_GetPitch(m_bitmap));
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
  assert(hasPalette());
  const RGBQUAD* pal = FreeImage_GetPalette(m_bitmap);
  if (pal == nullptr)
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

std::vector<unsigned char> ImageLoaderImpl::loadIndices() const
{
  assert(hasIndices());

  auto result = std::vector<unsigned char>(width() * height());
  for (unsigned y = 0; y < height(); ++y)
  {
    for (unsigned x = 0; x < width(); ++x)
    {
      BYTE index = 0;
      assertResult(FreeImage_GetPixelIndex(m_bitmap, x, y, &index) == TRUE);
      result[(height() - y - 1) * width() + x] = static_cast<unsigned char>(index);
    }
  }

  return result;
}

std::vector<unsigned char> ImageLoaderImpl::loadPixels(
  const ImageLoader::PixelFormat format) const
{
  assert(hasPixels());
  const size_t pSize = pixelSize(format);
  if (hasIndices())
  {
    return loadIndexedPixels(pSize);
  }
  else
  {
    return loadPixels(pSize);
  }
}

std::vector<unsigned char> ImageLoaderImpl::loadIndexedPixels(const size_t pSize) const
{
  assert(pSize == 3);
  const RGBQUAD* pal = FreeImage_GetPalette(m_bitmap);
  ensure(pal != nullptr, "pal is null");

  std::vector<unsigned char> result(width() * height() * pSize);
  for (unsigned y = 0; y < height(); ++y)
  {
    for (unsigned x = 0; x < width(); ++x)
    {
      BYTE paletteIndex = 0;
      assertResult(FreeImage_GetPixelIndex(m_bitmap, x, y, &paletteIndex) == TRUE);
      assert(paletteIndex < paletteSize());

      const size_t pixelIndex = ((height() - y - 1) * width() + x) * pSize;
      result[pixelIndex + 0] = static_cast<unsigned char>(pal[paletteIndex].rgbRed);
      result[pixelIndex + 1] = static_cast<unsigned char>(pal[paletteIndex].rgbGreen);
      result[pixelIndex + 2] = static_cast<unsigned char>(pal[paletteIndex].rgbBlue);
    }
  }
  return result;
}

std::vector<unsigned char> ImageLoaderImpl::loadPixels(const size_t pSize) const
{
  std::vector<unsigned char> result(width() * height() * pSize);
  for (unsigned y = 0; y < height(); ++y)
  {
    for (unsigned x = 0; x < width(); ++x)
    {
      RGBQUAD pixel;
      assertResult(FreeImage_GetPixelColor(m_bitmap, x, y, &pixel) == TRUE);

      const size_t pixelIndex = ((height() - y - 1) * width() + x) * pSize;
      result[pixelIndex + 0] = static_cast<unsigned char>(pixel.rgbRed);
      result[pixelIndex + 1] = static_cast<unsigned char>(pixel.rgbGreen);
      result[pixelIndex + 2] = static_cast<unsigned char>(pixel.rgbBlue);
      if (pSize > 3)
      {
        result[pixelIndex + 3] = static_cast<unsigned char>(pixel.rgbReserved);
      }
    }
  }

  return result;
}

FREE_IMAGE_FORMAT ImageLoaderImpl::translateFormat(const ImageLoader::Format format)
{
  switch (format)
  {
  case ImageLoader::PCX:
    return FIF_PCX;
  case ImageLoader::BMP:
    return FIF_BMP;
    switchDefault();
  }
}

size_t ImageLoaderImpl::pixelSize(const ImageLoader::PixelFormat format)
{
  switch (format)
  {
  case ImageLoader::RGB:
    return 3;
  case ImageLoader::RGBA:
    return 4;
    switchDefault();
  }
}
} // namespace IO
} // namespace TrenchBroom
