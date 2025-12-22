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
#include "LoadMipTexture.h"
#include "fs/Reader.h"
#include "fs/ReaderException.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Palette.h"
#include "mdl/Texture.h"
#include "mdl/TextureBuffer.h"

#include "kd/result.h"

#include <fmt/format.h>

namespace tb::mdl
{
namespace MipLayout
{
static constexpr size_t TextureNameLength = 16;
}

namespace
{

using GetMipPalette = std::function<Result<Palette>(fs::Reader& reader)>;

Result<Palette> readHlMipPalette(fs::Reader& reader)
{
  reader.seekFromBegin(0);
  reader.seekFromBegin(MipLayout::TextureNameLength);

  const auto width = reader.readSize<int32_t>();
  const auto height = reader.readSize<int32_t>();
  const auto mip0Offset = reader.readSize<int32_t>();

  // forward to the address of the color count
  reader.seekFromBegin(mip0Offset + (width * height * 85 >> 6));
  const auto colorCount = reader.readSize<uint16_t>();

  // palette data starts right after the color count
  auto data = std::vector<unsigned char>(colorCount * 3);
  reader.read(data.data(), data.size());
  return makePalette(data, PaletteColorFormat::Rgb);
}

Result<Texture> readMipTexture(
  fs::Reader& reader, const GetMipPalette& getMipPalette, const TextureMask mask)
{
  static const auto MipLevels = size_t(4);

  auto averageColor = Color{RgbaF{}};
  auto buffers = TextureBufferList{MipLevels};
  size_t offset[MipLevels];

  try
  {
    // This is unused, we use the one from the wad directory (they're usually the same,
    // but could be different in broken .wad's.)
    reader.readString(MipLayout::TextureNameLength);

    const auto width = reader.readSize<int32_t>();
    const auto height = reader.readSize<int32_t>();

    if (!checkTextureDimensions(width, height))
    {
      return Error{fmt::format("Invalid texture dimensions: {}*{}", width, height)};
    }

    for (size_t i = 0; i < MipLevels; ++i)
    {
      offset[i] = reader.readSize<int32_t>();
    }

    const auto transparency = mask == TextureMask::On
                                ? PaletteTransparency::Index255Transparent
                                : PaletteTransparency::Opaque;

    setMipBufferSize(buffers, MipLevels, width, height, GL_RGBA);
    return getMipPalette(reader) | kdl::transform([&](const auto& palette) {
             for (size_t i = 0; i < MipLevels; ++i)
             {
               reader.seekFromBegin(offset[i]);
               const auto size = mipSize(width, height, i);

               auto tempColor = Color{RgbaF{}};
               palette.indexedToRgba(reader, size, buffers[i], transparency, tempColor);
               if (i == 0)
               {
                 averageColor = tempColor;
               }
             }

             return Texture{
               width,
               height,
               averageColor,
               GL_RGBA,
               mask,
               NoEmbeddedDefaults{},
               std::move(buffers)};
           });
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace

std::string readMipTextureName(fs::Reader& reader)
{
  try
  {
    auto nameReader = reader.buffer();
    return nameReader.readString(MipLayout::TextureNameLength);
  }
  catch (const fs::ReaderException&)
  {
    return "";
  }
}

Result<Texture> loadIdMipTexture(
  fs::Reader& reader, const Palette& palette, const TextureMask mask)
{
  return readMipTexture(reader, [&](fs::Reader&) { return palette; }, mask);
}

Result<Texture> loadHlMipTexture(fs::Reader& reader, const TextureMask mask)
{
  return readMipTexture(reader, readHlMipPalette, mask);
}

} // namespace tb::mdl
