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

#include "Color.h"
#include "LoadM8Texture.h"
#include "fs/Reader.h"
#include "fs/ReaderException.h"
#include "mdl/Palette.h"
#include "mdl/Texture.h"
#include "mdl/TextureBuffer.h"

#include "kd/result.h"

#include <string>

namespace tb::mdl
{
namespace M8Layout
{
constexpr int Version = 2;
constexpr size_t TextureNameLength = 32;
constexpr size_t AnimNameLength = 32;
constexpr size_t MipLevels = 16;
constexpr size_t PaletteSize = 768;
} // namespace M8Layout


Result<Texture> loadM8Texture(fs::Reader& reader)
{
  try
  {
    const auto version = reader.readInt<int32_t>();
    if (version != M8Layout::Version)
    {
      return Error{"Unknown M8 texture version: " + std::to_string(version)};
    }

    reader.seekForward(M8Layout::TextureNameLength);

    auto widths = std::vector<size_t>{};
    auto heights = std::vector<size_t>{};
    auto offsets = std::vector<size_t>{}; // offsets from the beginning of the file

    widths.reserve(M8Layout::MipLevels);
    heights.reserve(M8Layout::MipLevels);
    offsets.reserve(M8Layout::MipLevels);

    for (size_t i = 0; i < M8Layout::MipLevels; ++i)
    {
      widths.push_back(reader.readSize<uint32_t>());
    }
    for (size_t i = 0; i < M8Layout::MipLevels; ++i)
    {
      heights.push_back(reader.readSize<uint32_t>());
    }
    for (size_t i = 0; i < M8Layout::MipLevels; ++i)
    {
      offsets.push_back(reader.readSize<uint32_t>());
    }

    reader.seekForward(M8Layout::AnimNameLength);

    auto paletteReader = reader.subReaderFromCurrent(M8Layout::PaletteSize);
    reader.seekForward(M8Layout::PaletteSize);

    return loadPalette(paletteReader, PaletteColorFormat::Rgb)
           | kdl::transform([&](const auto& palette) {
               reader.seekForward(4); // flags
               reader.seekForward(4); // contents
               reader.seekForward(4); // value

               auto mip0AverageColor = Color{RgbaF{}};
               auto buffers = TextureBufferList{};
               for (size_t mipLevel = 0; mipLevel < M8Layout::MipLevels; ++mipLevel)
               {
                 const auto w = widths[mipLevel];
                 const auto h = heights[mipLevel];

                 if (w == 0 || h == 0)
                 {
                   break;
                 }

                 reader.seekFromBegin(offsets[mipLevel]);

                 auto rgbaImage = TextureBuffer{4 * w * h};

                 auto averageColor = Color{RgbaF{}};
                 palette.indexedToRgba(
                   reader,
                   w * h,
                   rgbaImage,
                   PaletteTransparency::Opaque,
                   averageColor);
                 buffers.emplace_back(std::move(rgbaImage));

                 if (mipLevel == 0)
                 {
                   mip0AverageColor = averageColor;
                 }
               }

               return Texture{
                 widths[0],
                 heights[0],
                 mip0AverageColor,
                 GL_RGBA,
                 TextureMask::Off,
                 NoEmbeddedDefaults{},
                 std::move(buffers)};
             });
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
