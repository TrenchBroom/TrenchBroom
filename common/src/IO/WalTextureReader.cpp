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

#include "WalTextureReader.h"

#include "Assets/Texture.h"
#include "Ensure.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <kdl/result.h>

#include <iostream>
#include <string>

namespace TrenchBroom
{
namespace IO
{
namespace WalLayout
{
const size_t TextureNameLength = 32;
}

WalTextureReader::WalTextureReader(
  GetTextureName getTextureName, const FileSystem& fs, Logger& logger)
  : WalTextureReader{std::move(getTextureName), fs, std::nullopt, logger}
{
}

WalTextureReader::WalTextureReader(
  GetTextureName getTextureName,
  const FileSystem& fs,
  std::optional<Assets::Palette> palette,
  Logger& logger)
  : TextureReader{std::move(getTextureName), fs, logger}
  , m_palette{std::move(palette)}
{
}

Assets::Texture WalTextureReader::doReadTexture(std::shared_ptr<File> file) const
{
  const auto& path = file->path();
  auto reader = file->reader().buffer();

  try
  {
    const auto version = reader.readChar<char>();
    reader.seekFromBegin(0);

    if (version == 3)
    {
      return readDkWal(reader, path);
    }
    else
    {
      return readQ2Wal(reader, path);
    }
  }
  catch (const ReaderException&)
  {
    return Assets::Texture{textureName(path), 16, 16};
  }
}

Assets::Texture WalTextureReader::readQ2Wal(
  BufferedReader& reader, const Path& path) const
{
  static const auto MaxMipLevels = size_t(4);
  static auto averageColor = Color{};
  static auto buffers = Assets::TextureBufferList{MaxMipLevels};
  static size_t offsets[MaxMipLevels];

  // https://github.com/id-Software/Quake-2-Tools/blob/master/qe4/qfiles.h#L142

  const auto name = reader.readString(WalLayout::TextureNameLength);
  const auto width = reader.readSize<uint32_t>();
  const auto height = reader.readSize<uint32_t>();

  if (!checkTextureDimensions(width, height))
  {
    return Assets::Texture{textureName(path), 16, 16};
  }

  const auto mipLevels = readMipOffsets(MaxMipLevels, offsets, width, height, reader);

  /* const std::string animname = */ reader.readString(WalLayout::TextureNameLength);
  const auto flags = reader.readInt<int32_t>();
  const auto contents = reader.readInt<int32_t>();
  const auto value = reader.readInt<int32_t>();
  const auto gameData = Assets::Q2Data{flags, contents, value};

  if (!m_palette)
  {
    return Assets::Texture{
      textureName(name, path),
      width,
      height,
      GL_RGB,
      Assets::TextureType::Opaque,
      gameData};
  }

  Assets::setMipBufferSize(buffers, mipLevels, width, height, GL_RGBA);
  readMips(
    *m_palette,
    mipLevels,
    offsets,
    width,
    height,
    reader,
    buffers,
    averageColor,
    Assets::PaletteTransparency::Opaque);

  return Assets::Texture{
    textureName(name, path),
    width,
    height,
    averageColor,
    std::move(buffers),
    GL_RGBA,
    Assets::TextureType::Opaque,
    gameData};
}

Assets::Texture WalTextureReader::readDkWal(
  BufferedReader& reader, const Path& path) const
{
  static const auto MaxMipLevels = size_t(9);
  static auto averageColor = Color{};
  static auto buffers = Assets::TextureBufferList{MaxMipLevels};
  static size_t offsets[MaxMipLevels];

  // https://gist.github.com/DanielGibson/a53c74b10ddd0a1f3d6ab42909d5b7e1

  const auto version = reader.readChar<char>();
  ensure(version == 3, "Unknown WAL texture version");

  const auto name = reader.readString(WalLayout::TextureNameLength);
  reader.seekForward(3); // garbage

  const auto width = reader.readSize<uint32_t>();
  const auto height = reader.readSize<uint32_t>();

  if (!checkTextureDimensions(width, height))
  {
    return Assets::Texture{textureName(path), 16, 16};
  }

  const auto mipLevels = readMipOffsets(MaxMipLevels, offsets, width, height, reader);
  Assets::setMipBufferSize(buffers, mipLevels, width, height, GL_RGBA);

  /* const auto animname = */ reader.readString(WalLayout::TextureNameLength);
  const auto flags = reader.readInt<int32_t>();
  const auto contents = reader.readInt<int32_t>();

  auto paletteReader = reader.subReaderFromCurrent(3 * 256);
  reader.seekForward(3 * 256); // seek past palette
  const auto value = reader.readInt<int32_t>();
  const auto gameData = Assets::Q2Data{flags, contents, value};

  return Assets::loadPalette(paletteReader)
    .transform([&](auto embeddedPalette) {
      const auto hasTransparency = readMips(
        embeddedPalette,
        mipLevels,
        offsets,
        width,
        height,
        reader,
        buffers,
        averageColor,
        Assets::PaletteTransparency::Index255Transparent);

      return Assets::Texture{
        textureName(name, path),
        width,
        height,
        averageColor,
        std::move(buffers),
        GL_RGBA,
        hasTransparency ? Assets::TextureType::Masked : Assets::TextureType::Opaque,
        gameData};
    })
    .if_error([](const auto& e) { throw AssetException{e.msg.c_str()}; })
    .value();
}

size_t WalTextureReader::readMipOffsets(
  const size_t maxMipLevels,
  size_t offsets[],
  const size_t width,
  const size_t height,
  Reader& reader) const
{
  size_t mipLevels = 0;
  for (size_t i = 0; i < maxMipLevels; ++i)
  {
    offsets[i] = reader.readSize<uint32_t>();
    ++mipLevels;
    if (width / (size_t(1) << i) == 1 || height / (size_t(1) << i) == 1)
    {
      break;
    }
  }

  // make sure the reader position is correct afterwards
  reader.seekForward((maxMipLevels - mipLevels) * sizeof(uint32_t));

  return mipLevels;
}

bool WalTextureReader::readMips(
  const Assets::Palette& palette,
  const size_t mipLevels,
  const size_t offsets[],
  const size_t width,
  const size_t height,
  BufferedReader& reader,
  Assets::TextureBufferList& buffers,
  Color& averageColor,
  const Assets::PaletteTransparency transparency)
{
  static auto tempColor = Color{};

  auto hasTransparency = false;
  for (size_t i = 0; i < mipLevels; ++i)
  {
    const auto offset = offsets[i];
    reader.seekFromBegin(offset);
    const auto curWidth = width / (size_t(1) << i);
    const auto curHeight = height / (size_t(1) << i);
    const auto size = curWidth * curHeight;

    // FIXME: Confirm this is actually happening because of bad data and not a bug.
    // FIXME: Corrupt or missing mips should be deleted, rather than uploaded with
    // garbage.
    if (!reader.canRead(size))
    {
      std::cerr << "WalTextureReader::readMips: buffer overrun\n";
      return false;
    }

    hasTransparency =
      hasTransparency
      || (palette.indexedToRgba(reader, size, buffers[i], transparency, tempColor) && i == 0);
    if (i == 0)
    {
      averageColor = tempColor;
    }
  }
  return hasTransparency;
}
} // namespace IO
} // namespace TrenchBroom
