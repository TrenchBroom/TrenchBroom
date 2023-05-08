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

#include "IO/ImageLoader.h"

#include <FreeImage.h>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
class Path;

class InitFreeImage
{
private:
  InitFreeImage();
  ~InitFreeImage();

public:
  static void initialize();
};

class ImageLoaderImpl
{
private:
  FIMEMORY* m_stream;
  FIBITMAP* m_bitmap;

public:
  ImageLoaderImpl(const ImageLoader::Format format, const Path& path);
  ImageLoaderImpl(const ImageLoader::Format format, const char* begin, const char* end);
  ~ImageLoaderImpl();

  size_t paletteSize() const;
  size_t bitsPerPixel() const;
  size_t width() const;
  size_t height() const;
  size_t byteWidth() const;
  size_t scanWidth() const;

  bool hasPalette() const;
  bool hasIndices() const;
  bool hasPixels() const;

  std::vector<unsigned char> loadPalette() const;
  std::vector<unsigned char> loadIndices() const;
  std::vector<unsigned char> loadPixels(const ImageLoader::PixelFormat format) const;

private:
  std::vector<unsigned char> loadIndexedPixels(const size_t pSize) const;
  std::vector<unsigned char> loadPixels(const size_t pSize) const;
  static FREE_IMAGE_FORMAT translateFormat(const ImageLoader::Format format);
  static size_t pixelSize(const ImageLoader::PixelFormat format);
};
} // namespace IO
} // namespace TrenchBroom
