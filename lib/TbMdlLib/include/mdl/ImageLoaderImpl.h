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

#pragma once

#include "mdl/ImageLoader.h"

#include <FreeImage.h>

#include <vector>

namespace tb::mdl
{

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
  FIMEMORY* m_stream = nullptr;
  FIBITMAP* m_bitmap = nullptr;

public:
  ImageLoaderImpl(const char* begin, const char* end);
  ~ImageLoaderImpl();

  size_t width() const;
  size_t height() const;

  bool hasPalette() const;

  std::vector<unsigned char> loadPalette() const;
  std::vector<unsigned char> loadPixels() const;

private:
  size_t paletteSize() const;
  bool hasIndices() const;
  bool hasPixels() const;

  std::vector<unsigned char> loadIndexedPixels() const;
  std::vector<unsigned char> loadDirectPixels() const;
};

} // namespace tb::mdl
