/*
 Copyright (C) 2010-2016 Kristian Duske

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

#include "ReadMipTexture.h"

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Color.h"
#include "Ensure.h"
#include "Error.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <kdl/result.h>

namespace TrenchBroom::IO
{
namespace MipLayout
{
static constexpr size_t TextureNameLength = 16;
}

namespace
{

using GetMipPalette = std::function<Result<Assets::Palette>(Reader& reader)>;

Result<Assets::Palette> readHlMipPalette(Reader& reader)
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
  return Assets::makePalette(data);
}

Result<Assets::Texture, ReadTextureError> readMipTexture(
  std::string name, Reader& reader, const GetMipPalette& getMipPalette)
{
  static const auto MipLevels = size_t(4);

  auto averageColor = Color{};
  auto buffers = Assets::TextureBufferList{MipLevels};
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
      return ReadTextureError{std::move(name), "Invalid texture dimeions"};
    }

    for (size_t i = 0; i < MipLevels; ++i)
    {
      offset[i] = reader.readSize<int32_t>();
    }

    const auto transparent = (!name.empty() && name.at(0) == '{')
                               ? Assets::PaletteTransparency::Index255Transparent
                               : Assets::PaletteTransparency::Opaque;

    Assets::setMipBufferSize(buffers, MipLevels, width, height, GL_RGBA);
    return getMipPalette(reader)
      .and_then([&](const auto& palette) {
        for (size_t i = 0; i < MipLevels; ++i)
        {
          reader.seekFromBegin(offset[i]);
          const auto size = mipSize(width, height, i);

          auto tempColor = Color{};
          palette.indexedToRgba(reader, size, buffers[i], transparent, tempColor);
          if (i == 0)
          {
            averageColor = tempColor;
          }
        }

        const auto type =
          (transparent == Assets::PaletteTransparency::Index255Transparent)
            ? Assets::TextureType::Masked
            : Assets::TextureType::Opaque;

        return Result<Assets::Texture>{Assets::Texture{
          std::move(name),
          width,
          height,
          averageColor,
          std::move(buffers),
          GL_RGBA,
          type}};
      })
      .or_else([&](const auto& e) {
        return Result<Assets::Texture, ReadTextureError>{
          ReadTextureError{std::move(name), e.msg}};
      });
  }
  catch (const ReaderException& e)
  {
    return ReadTextureError{std::move(name), e.what()};
  }
}

} // namespace

std::string readMipTextureName(Reader& reader)
{
  try
  {
    auto nameReader = reader.buffer();
    return nameReader.readString(MipLayout::TextureNameLength);
  }
  catch (const ReaderException&)
  {
    return "";
  }
}

Result<Assets::Texture, ReadTextureError> readIdMipTexture(
  std::string name, Reader& reader, const Assets::Palette& palette)
{
  return readMipTexture(std::move(name), reader, [&](Reader&) { return palette; });
}

Result<Assets::Texture, ReadTextureError> readHlMipTexture(
  std::string name, Reader& reader)
{
  return readMipTexture(std::move(name), reader, readHlMipPalette);
}

} // namespace TrenchBroom::IO
