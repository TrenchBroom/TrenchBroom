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
#include "Error.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/ImageLoader.h"
#include "IO/Reader.h"

#include <kdl/reflection_impl.h>
#include <kdl/result.h>
#include <kdl/string_format.h>

#include <cstring>
#include <ostream>
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

Palette::Palette(std::shared_ptr<PaletteData> data)
  : m_data{std::move(data)}
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

Result<Palette> makePalette(
  const std::vector<unsigned char>& data, const PaletteColorFormat colorFormat)
{
  auto result = std::make_shared<PaletteData>();

  switch (colorFormat)
  {
  case PaletteColorFormat::Rgb:
    // transform data to RGBA
    result->opaqueData.reserve(data.size() / 3 * 4);

    for (size_t i = 0; i < data.size() / 3; ++i)
    {
      const auto r = data[3 * i + 0];
      const auto g = data[3 * i + 1];
      const auto b = data[3 * i + 2];

      result->opaqueData.push_back(r);
      result->opaqueData.push_back(g);
      result->opaqueData.push_back(b);
      result->opaqueData.push_back(0xFF);
    }

    if (!result->opaqueData.empty())
    {
      // build index255TransparentData from opaqueData
      result->index255TransparentData = result->opaqueData;
      result->index255TransparentData.back() = 0;
    }
    break;
  case PaletteColorFormat::Rgba:
    // The data is already in RGBA format, don't process it
    result->opaqueData = data;
    result->index255TransparentData = data;
    break;
  }

  return Palette{std::move(result)};
}

namespace
{

Result<Palette> loadLmp(IO::Reader& reader)
{
  auto data = std::vector<unsigned char>(reader.size());
  reader.read(data.data(), data.size());
  return makePalette(data, PaletteColorFormat::Rgb);
}

Result<Palette> loadPcx(IO::Reader& reader)
{
  auto data = std::vector<unsigned char>(768);
  reader.seekFromEnd(data.size());
  reader.read(data.data(), data.size());
  return makePalette(data, PaletteColorFormat::Rgb);
}

Result<Palette> loadBmp(IO::Reader& reader)
{
  auto bufferedReader = reader.buffer();
  auto imageLoader =
    IO::ImageLoader{IO::ImageLoader::BMP, bufferedReader.begin(), bufferedReader.end()};
  auto data = imageLoader.hasPalette() ? imageLoader.loadPalette()
                                       : imageLoader.loadPixels(IO::ImageLoader::RGB);
  return makePalette(data, PaletteColorFormat::Rgb);
}

} // namespace

Result<Palette> loadPalette(const IO::File& file, const std::filesystem::path& path)
{
  try
  {
    const auto extension = kdl::str_to_lower(path.extension().string());
    if (extension == ".lmp")
    {
      auto reader = file.reader().buffer();
      return loadLmp(reader);
    }
    if (extension == ".pcx")
    {
      auto reader = file.reader().buffer();
      return loadPcx(reader);
    }
    if (extension == ".bmp")
    {
      auto reader = file.reader().buffer();
      return loadBmp(reader);
    }

    return Error{
      "Could not load palette file '" + path.string() + "': Unknown palette format"};
  }
  catch (const Exception& e)
  {
    return Error{"Could not load palette file '" + path.string() + "': " + e.what()};
  }
}

Result<Palette> loadPalette(IO::Reader& reader, const PaletteColorFormat colorFormat)
{
  try
  {
    auto data = std::vector<unsigned char>(reader.size());
    reader.read(data.data(), data.size());
    return makePalette(data, colorFormat);
  }
  catch (const Exception& e)
  {
    using namespace std::string_literals;
    return Error{"Could not load palette: "s + e.what()};
  }
}

} // namespace TrenchBroom::Assets
