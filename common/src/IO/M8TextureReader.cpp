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

#include "M8TextureReader.h"

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Ensure.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <iostream>
#include <string>

namespace TrenchBroom::IO
{
namespace M8Layout
{
constexpr int Version = 2;
constexpr size_t TextureNameLength = 32;
constexpr size_t AnimNameLength = 32;
constexpr size_t MipLevels = 16;
constexpr size_t PaletteSize = 768;
} // namespace M8Layout

M8TextureReader::M8TextureReader(
  const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger)
  : TextureReader{nameStrategy, fs, logger}
{
}

Assets::Texture M8TextureReader::doReadTexture(std::shared_ptr<File> file) const
{
  const auto& path = file->path();
  auto reader = file->reader().buffer();
  try
  {
    const auto version = reader.readInt<int32_t>();
    if (version != M8Layout::Version)
    {
      return Assets::Texture{textureName(path), 16, 16};
    }

    const auto name = reader.readString(M8Layout::TextureNameLength);

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
    const auto palette = Assets::loadPalette(paletteReader);

    reader.seekForward(4); // flags
    reader.seekForward(4); // contents
    reader.seekForward(4); // value

    auto mip0AverageColor = Color{};
    auto buffers = Assets::TextureBufferList{};
    for (size_t mipLevel = 0; mipLevel < M8Layout::MipLevels; ++mipLevel)
    {
      const auto w = widths[mipLevel];
      const auto h = heights[mipLevel];

      if (w == 0 || h == 0)
      {
        break;
      }

      reader.seekFromBegin(offsets[mipLevel]);

      auto rgbaImage = Assets::TextureBuffer{4 * w * h};

      auto averageColor = Color{};
      palette.indexedToRgba(
        reader, w * h, rgbaImage, Assets::PaletteTransparency::Opaque, averageColor);
      buffers.emplace_back(std::move(rgbaImage));

      if (mipLevel == 0)
      {
        mip0AverageColor = averageColor;
      }
    }

    return Assets::Texture{
      textureName(name, path),
      widths[0],
      heights[0],
      mip0AverageColor,
      std::move(buffers),
      GL_RGBA,
      Assets::TextureType::Opaque};
  }
  catch (const ReaderException&)
  {
    return Assets::Texture{textureName(path), 16, 16};
  }
}

} // namespace TrenchBroom::IO
