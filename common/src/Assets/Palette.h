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
#include "Result.h"

#include "kdl/reflection_decl.h"

#include <cassert>
#include <filesystem>
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
class TextureBuffer;

struct PaletteData
{
  /**
   * 1024 bytes, RGBA order.
   */
  std::vector<unsigned char> opaqueData;
  /**
   * 1024 bytes, RGBA order.
   */
  std::vector<unsigned char> index255TransparentData;

  kdl_reflect_decl(PaletteData, opaqueData, index255TransparentData);
};

enum class PaletteTransparency
{
  Opaque,
  Index255Transparent
};

enum class PaletteColorFormat
{
  Rgb,
  Rgba,
};

std::ostream& operator<<(std::ostream& lhs, PaletteColorFormat rhs);

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

  friend bool operator==(const Palette& lhs, const Palette& rhs);
  friend bool operator!=(const Palette& lhs, const Palette& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const Palette& rhs);
};

Result<Palette> makePalette(
  const std::vector<unsigned char>& data, PaletteColorFormat colorFormat);

Result<Palette> loadPalette(const IO::File& file, const std::filesystem::path& path);
Result<Palette> loadPalette(IO::Reader& reader, PaletteColorFormat colorFormat);

} // namespace TrenchBroom::Assets
