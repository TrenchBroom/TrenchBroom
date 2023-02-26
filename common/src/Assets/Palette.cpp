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

#include "Palette.h"

#include "Assets/TextureBuffer.h"
#include "Ensure.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/ImageLoader.h"
#include "IO/Reader.h"

#include <kdl/string_format.h>

#include <cstring>
#include <string>

namespace TrenchBroom::Assets
{

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
};

static std::shared_ptr<PaletteData> makePaletteData(
  const std::vector<unsigned char>& data)
{
  if (data.size() != 768 && data.size() != 1024)
  {
    throw AssetException{
      "Could not load palette, expected 768 or 1024 bytes, got "
      + std::to_string(data.size())};
  }

  auto result = PaletteData{};

  if (data.size() == 1024)
  {
    // The data is already in RGBA format, don't process it
    result.opaqueData = data;
    result.index255TransparentData = data;
  }
  else
  {
    result.opaqueData.reserve(1024);

    for (size_t i = 0; i < 256; ++i)
    {
      const auto r = data[3 * i + 0];
      const auto g = data[3 * i + 1];
      const auto b = data[3 * i + 2];

      result.opaqueData.push_back(r);
      result.opaqueData.push_back(g);
      result.opaqueData.push_back(b);
      result.opaqueData.push_back(0xFF);
    }

    // build index255TransparentData from opaqueData
    result.index255TransparentData = result.opaqueData;
    result.index255TransparentData[1023] = 0;
  }

  return std::make_shared<PaletteData>(std::move(result));
}

Palette::Palette(const std::vector<unsigned char>& data)
  : m_data{makePaletteData(data)}
{
}

bool Palette::indexedToRgba(
  IO::Reader& reader,
  const size_t pixelCount,
  TextureBuffer& rgbaImage,
  const PaletteTransparency transparency,
  Color& averageColor) const
{
  ensure(rgbaImage.size() == 4 * pixelCount, "incorrect destination buffer size");

  const unsigned char* paletteData = (transparency == PaletteTransparency::Opaque)
                                       ? m_data->opaqueData.data()
                                       : m_data->index255TransparentData.data();

  // Write rgba pixels
  auto* const rgbaData = rgbaImage.data();
  for (size_t i = 0; i < pixelCount; ++i)
  {
    const int index = reader.readInt<unsigned char>();

    std::memcpy(rgbaData + (i * 4), &paletteData[index * 4], 4);
  }

  // Check average color
  uint32_t colorSum[3] = {0, 0, 0};
  for (size_t i = 0; i < pixelCount; ++i)
  {
    colorSum[0] += uint32_t(rgbaData[(i * 4) + 0]);
    colorSum[1] += uint32_t(rgbaData[(i * 4) + 1]);
    colorSum[2] += uint32_t(rgbaData[(i * 4) + 2]);
  }
  averageColor = Color{
    float(colorSum[0]) / (255.0f * float(pixelCount)),
    float(colorSum[1]) / (255.0f * float(pixelCount)),
    float(colorSum[2]) / (255.0f * float(pixelCount)),
    1.0f};

  // Check for transparency
  auto hasTransparency = false;
  if (transparency == PaletteTransparency::Index255Transparent)
  {
    // Take the bitwise AND of the alpha channel of all pixels
    unsigned char andAlpha = 0xFF;
    for (size_t i = 0; i < pixelCount; ++i)
    {
      andAlpha = static_cast<unsigned char>(andAlpha & rgbaData[4 * i + 3]);
    }
    hasTransparency = (andAlpha != 0xFF);
  }

  return hasTransparency;
}

namespace
{

Palette loadLmp(IO::Reader& reader)
{
  auto data = std::vector<unsigned char>(reader.size());
  reader.read(data.data(), data.size());
  return Palette{data};
}

Palette loadPcx(IO::Reader& reader)
{
  auto data = std::vector<unsigned char>(768);
  reader.seekFromEnd(data.size());
  reader.read(data.data(), data.size());
  return Palette{data};
}

Palette loadBmp(IO::Reader& reader)
{
  auto bufferedReader = reader.buffer();
  auto imageLoader =
    IO::ImageLoader{IO::ImageLoader::BMP, bufferedReader.begin(), bufferedReader.end()};
  auto data = imageLoader.hasPalette() ? imageLoader.loadPalette()
                                       : imageLoader.loadPixels(IO::ImageLoader::RGB);
  return Palette{data};
}

} // namespace

Palette loadPalette(const IO::File& file)
{
  try
  {
    const auto extension = kdl::str_to_lower(file.path().extension());
    if (extension == "lmp")
    {
      auto reader = file.reader().buffer();
      return loadLmp(reader);
    }
    if (extension == "pcx")
    {
      auto reader = file.reader().buffer();
      return loadPcx(reader);
    }
    if (extension == "bmp")
    {
      auto reader = file.reader().buffer();
      return loadBmp(reader);
    }

    throw AssetException{
      "Could not load palette file '" + file.path().asString()
      + "': Unknown palette format"};
  }
  catch (const FileSystemException& e)
  {
    throw AssetException{
      "Could not load palette file '" + file.path().asString() + "': " + e.what()};
  }
}

Palette loadPalette(IO::Reader& reader)
{
  auto data = std::vector<unsigned char>(reader.size());
  reader.read(data.data(), data.size());
  return Palette{data};
}

} // namespace TrenchBroom::Assets
