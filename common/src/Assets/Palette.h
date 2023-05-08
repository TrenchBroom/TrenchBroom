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

#include "Color.h"

#include <kdl/reflection_decl.h>
#include <kdl/result_forward.h>

#include <cassert>
#include <iosfwd>
#include <memory>
#include <vector>

namespace TrenchBroom::IO
{
class File;
class Reader;
} // namespace TrenchBroom::IO

namespace TrenchBroom::Assets
{
struct PaletteData;
class TextureBuffer;

enum class PaletteTransparency
{
  Opaque,
  Index255Transparent
};

class Palette
{
private:
  std::shared_ptr<PaletteData> m_data;

public:
  explicit Palette(std::shared_ptr<PaletteData> m_data);

  /**
   * Reads `pixelCount` bytes from `reader` where each byte is a palette index,
   * and writes `pixelCount` * 4 bytes to `rgbaImage` using the palette to convert
   * the image to RGBA.
   *
   * Must not be called if `initialized()` is false.
   *
   * @param reader the reader to read from; the position will be advanced
   * @param pixelCount number of pixels (bytes) to read
   * @param rgbaImage the destination buffer, size must be exactly `pixelCount` * 4 bytes
   * @param transparency controls whether or not the palette contains a transparent index
   * @param averageColor output parameter for the average color of the generated pixel
   * buffer
   * @return true if the given index buffer did contain a transparent index, unless the
   * transparency parameter indicates that the image is opaque
   *
   * @throws ReaderException if reader doesn't have pixelCount bytes available
   */
  bool indexedToRgba(
    IO::Reader& reader,
    size_t pixelCount,
    TextureBuffer& rgbaImage,
    PaletteTransparency transparency,
    Color& averageColor) const;
};

struct LoadPaletteError
{
  std::string msg;

  kdl_reflect_decl(LoadPaletteError, msg);
};

kdl::result<Palette, LoadPaletteError> makePalette(
  const std::vector<unsigned char>& data);

kdl::result<Palette, LoadPaletteError> loadPalette(const IO::File& file);
kdl::result<Palette, LoadPaletteError> loadPalette(IO::Reader& reader);

} // namespace TrenchBroom::Assets
