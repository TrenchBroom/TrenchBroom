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

#include "MipTextureReader.h"

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Color.h"
#include "Ensure.h"
#include "IO/File.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <string>

namespace TrenchBroom
{
namespace IO
{
namespace MipLayout
{
static constexpr size_t TextureNameLength = 16;
}

MipTextureReader::MipTextureReader(
  const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger)
  : TextureReader(nameStrategy, fs, logger)
{
}

MipTextureReader::~MipTextureReader() = default;

size_t MipTextureReader::mipFileSize(
  const size_t width, const size_t height, const size_t mipLevels)
{
  size_t result = 0;
  for (size_t i = 0; i < mipLevels; ++i)
  {
    result += mipSize(width, height, i);
  }
  return result;
}

std::string MipTextureReader::getTextureName(const BufferedReader& reader)
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

Assets::Texture MipTextureReader::doReadTexture(std::shared_ptr<File> file) const
{
  static const size_t MipLevels = 4;

  Color averageColor;
  Assets::TextureBufferList buffers(MipLevels);
  size_t offset[MipLevels];

  ensure(!file->path().isEmpty(), "MipTextureReader::doReadTexture requires a path");

  const auto path = file->path();
  const auto basename = path.lastComponent().deleteExtension().asString();
  const auto name = textureName(basename, path);
  try
  {
    auto reader = file->reader().buffer();

    // This is unused, we use the one from the wad directory (they're usually the same,
    // but could be different in broken .wad's.)
    reader.readString(MipLayout::TextureNameLength);

    const auto width = reader.readSize<int32_t>();
    const auto height = reader.readSize<int32_t>();

    if (!checkTextureDimensions(width, height))
    {
      throw AssetException("Invalid texture dimensions");
    }

    for (size_t i = 0; i < MipLevels; ++i)
    {
      offset[i] = reader.readSize<int32_t>();
    }

    const auto transparent = (!name.empty() && name.at(0) == '{')
                               ? Assets::PaletteTransparency::Index255Transparent
                               : Assets::PaletteTransparency::Opaque;

    Assets::setMipBufferSize(buffers, MipLevels, width, height, GL_RGBA);
    auto palette = doGetPalette(reader, offset, width, height);

    if (!palette.initialized())
    {
      throw AssetException("Palette is not initialized");
    }

    for (size_t i = 0; i < MipLevels; ++i)
    {
      reader.seekFromBegin(offset[i]);
      const size_t size = mipSize(width, height, i);

      Color tempColor;
      palette.indexedToRgba(reader, size, buffers[i], transparent, tempColor);
      if (i == 0)
      {
        averageColor = tempColor;
      }
    }

    const auto type = (transparent == Assets::PaletteTransparency::Index255Transparent)
                        ? Assets::TextureType::Masked
                        : Assets::TextureType::Opaque;
    return Assets::Texture(
      name, width, height, averageColor, std::move(buffers), GL_RGBA, type);
  }
  catch (const ReaderException& e)
  {
    throw AssetException(e.what());
  }
}
} // namespace IO
} // namespace TrenchBroom
