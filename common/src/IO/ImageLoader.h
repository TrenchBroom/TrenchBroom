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

#include <memory>
#include <vector>

namespace TrenchBroom {
namespace IO {
class ImageLoaderImpl;
class Path;

class ImageLoader {
public:
  enum Format {
    PCX,
    BMP
  };

  enum PixelFormat {
    RGB,
    RGBA
  };

private:
  // we're using the PIMPL idiom here to insulate the clients from the FreeImage headers
  std::unique_ptr<ImageLoaderImpl> m_impl;

public:
  ImageLoader(const Format format, const Path& path);
  ImageLoader(const Format format, const char* begin, const char* end);
  ~ImageLoader();

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
  std::vector<unsigned char> loadPixels(const PixelFormat format) const;

private:
  ImageLoader(const ImageLoader& other);
  ImageLoader& operator=(const ImageLoader& other);
};
} // namespace IO
} // namespace TrenchBroom
