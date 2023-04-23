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

#include "ReadWalTexture.h"

#include "Assets/Texture.h"
#include "Ensure.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <kdl/result.h>

#include <cassert>

namespace TrenchBroom::IO
{
namespace WalLayout
{
const size_t TextureNameLength = 32;
}

namespace
{

size_t readMipOffsets(
  const size_t maxMipLevels,
  size_t offsets[],
  const size_t width,
  const size_t height,
  Reader& reader)
{
  const auto mipLevels = std::min(
    std::min(size_t(std::log2(width)), size_t(std::log2(height))) + 1, maxMipLevels);

  for (size_t i = 0; i < mipLevels; ++i)
  {
    offsets[i] = reader.readSize<uint32_t>();
  }

  // make sure the reader position is correct afterwards
  reader.seekForward((maxMipLevels - mipLevels) * sizeof(uint32_t));

  return mipLevels;
}

std::tuple<Assets::TextureBufferList, bool> readMips(
  const Assets::Palette& palette,
  const size_t mipLevels,
  const size_t offsets[],
  const size_t width,
  const size_t height,
  Reader& reader,
  Color& averageColor,
  const Assets::PaletteTransparency transparency)
{
  static auto tempColor = Color{};

  auto buffers = Assets::TextureBufferList{};
  Assets::setMipBufferSize(buffers, mipLevels, width, height, GL_RGBA);

  auto hasTransparency = false;
  for (size_t i = 0; i < mipLevels; ++i)
  {
    const auto offset = offsets[i];
    reader.seekFromBegin(offset);
    const auto curWidth = width / (size_t(1) << i);
    const auto curHeight = height / (size_t(1) << i);
    const auto size = curWidth * curHeight;

    hasTransparency =
      hasTransparency
      || (palette.indexedToRgba(reader, size, buffers[i], transparency, tempColor) && i == 0);
    if (i == 0)
    {
      averageColor = tempColor;
    }
  }
  return {std::move(buffers), hasTransparency};
}

kdl::result<Assets::Texture, ReadTextureError> readQ2Wal(
  std::string name, Reader& reader, const std::optional<Assets::Palette>& palette)
{
  static const auto MaxMipLevels = size_t(4);
  auto averageColor = Color{};
  size_t offsets[MaxMipLevels];

  // https://github.com/id-Software/Quake-2-Tools/blob/master/qe4/qfiles.h#L142

  try
  {
    reader.seekForward(WalLayout::TextureNameLength);
    const auto width = reader.readSize<uint32_t>();
    const auto height = reader.readSize<uint32_t>();

    if (!checkTextureDimensions(width, height))
    {
      return ReadTextureError{std::move(name), "Invalid texture dimensions"};
    }

    const auto mipLevels = readMipOffsets(MaxMipLevels, offsets, width, height, reader);

    /* const std::string animname = */ reader.readString(WalLayout::TextureNameLength);
    const auto flags = reader.readInt<int32_t>();
    const auto contents = reader.readInt<int32_t>();
    const auto value = reader.readInt<int32_t>();
    const auto gameData = Assets::Q2Data{flags, contents, value};

    if (!palette)
    {
      throw AssetException{"Missing palette"};
    }

    auto [buffers, hasTransparency] = readMips(
      *palette,
      mipLevels,
      offsets,
      width,
      height,
      reader,
      averageColor,
      Assets::PaletteTransparency::Opaque);

    unused(hasTransparency);

    return Assets::Texture{
      std::move(name),
      width,
      height,
      averageColor,
      std::move(buffers),
      GL_RGBA,
      Assets::TextureType::Opaque,
      gameData};
  }
  catch (const ReaderException& e)
  {
    return ReadTextureError{std::move(name), e.what()};
  }
}

kdl::result<Assets::Texture, ReadTextureError> readDkWal(std::string name, Reader& reader)
{
  static const auto MaxMipLevels = size_t(9);
  auto averageColor = Color{};
  size_t offsets[MaxMipLevels];

  // https://gist.github.com/DanielGibson/a53c74b10ddd0a1f3d6ab42909d5b7e1

  try
  {
    const auto version = reader.readChar<char>();
    ensure(version == 3, "wal texture has version 3");

    reader.seekForward(WalLayout::TextureNameLength);
    reader.seekForward(3); // garbage

    const auto width = reader.readSize<uint32_t>();
    const auto height = reader.readSize<uint32_t>();

    if (!checkTextureDimensions(width, height))
    {
      return ReadTextureError{std::move(name), "Invalid texture dimensions"};
    }

    const auto mipLevels = readMipOffsets(MaxMipLevels, offsets, width, height, reader);

    /* const auto animname = */ reader.readString(WalLayout::TextureNameLength);
    const auto flags = reader.readInt<int32_t>();
    const auto contents = reader.readInt<int32_t>();

    auto paletteReader = reader.subReaderFromCurrent(3 * 256);
    reader.seekForward(3 * 256); // seek past palette
    const auto value = reader.readInt<int32_t>();
    const auto gameData = Assets::Q2Data{flags, contents, value};

    return Assets::loadPalette(paletteReader)
      .transform([&](const auto& palette) {
        auto [buffers, hasTransparency] = readMips(
          palette,
          mipLevels,
          offsets,
          width,
          height,
          reader,
          averageColor,
          Assets::PaletteTransparency::Index255Transparent);

        return Assets::Texture{
          std::move(name),
          width,
          height,
          averageColor,
          std::move(buffers),
          GL_RGBA,
          hasTransparency ? Assets::TextureType::Masked : Assets::TextureType::Opaque,
          gameData};
      })
      .or_else([&](const auto& error) {
        return kdl::result<Assets::Texture, ReadTextureError>{
          ReadTextureError{std::move(name), error.msg}};
      });
    ;
  }
  catch (const ReaderException& e)
  {
    return ReadTextureError{std::move(name), e.what()};
  }
}

} // namespace

kdl::result<Assets::Texture, ReadTextureError> readWalTexture(
  std::string name, Reader& reader, const std::optional<Assets::Palette>& palette)
{
  try
  {
    const auto version = reader.readChar<char>();
    reader.seekFromBegin(0);

    if (version == 3)
    {
      return readDkWal(std::move(name), reader);
    }
    return readQ2Wal(std::move(name), reader, palette);
  }
  catch (const Exception& e)
  {
    return ReadTextureError{std::move(name), e.what()};
  }
}

} // namespace TrenchBroom::IO
