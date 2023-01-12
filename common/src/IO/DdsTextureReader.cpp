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

#include "DdsTextureReader.h"

#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Ensure.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/ImageLoaderImpl.h"
#include "IO/ReaderException.h"

#include <stdexcept>

namespace TrenchBroom
{
namespace IO
{
namespace DdsLayout
{
static const size_t Ident = ((' ' << 24) + ('S' << 16) + ('D' << 8) + 'D');
static const size_t IdentDx10 = (('0' << 24) + ('1' << 16) + ('X' << 8) + 'D');
static const size_t BasicHeaderLengthWithIdent = 128;
static const size_t PixelFormatOffset = 76;
static const size_t Dx10HeaderLength = 20;

static const size_t DdpfAlphaPixels = 1 << 0;
static const size_t DdpfFourcc = 1 << 2;
static const size_t DdpfRgb = 1 << 6;

static const size_t Ddcaps2Cubemap = 1 << 9;
static const size_t Ddcaps2CubemapPX = 1 << 10;
static const size_t Ddcaps2CubemapNX = 1 << 11;
static const size_t Ddcaps2CubemapPY = 1 << 12;
static const size_t Ddcaps2CubemapNY = 1 << 13;
static const size_t Ddcaps2CubemapPZ = 1 << 14;
static const size_t Ddcaps2CubemapNZ = 1 << 15;
static const size_t Ddcaps2CubemapAllFacesMask =
  (Ddcaps2CubemapPX | Ddcaps2CubemapNX | Ddcaps2CubemapPY | Ddcaps2CubemapNY
   | Ddcaps2CubemapPZ | Ddcaps2CubemapNZ);
static const size_t Ddcaps2Volume = 1 << 21;

static const size_t FourccDXT1 = (('1' << 24) + ('T' << 16) + ('X' << 8) + 'D');
static const size_t FourccDXT3 = (('3' << 24) + ('T' << 16) + ('X' << 8) + 'D');
static const size_t FourccDXT5 = (('5' << 24) + ('T' << 16) + ('X' << 8) + 'D');

static const size_t D3d10ResourceMiscCubemap = 1 << 2;
static const size_t D3d10ResourceDimensionTexture2D = 3;

static const size_t DxgiFormatR8G8B8A8Typeless = 27;
static const size_t DxgiFormatR8G8B8A8Unorm = 28;
static const size_t DxgiFormatR8G8B8A8UnormSrgb = 29;
static const size_t DxgiFormatR8G8B8A8Uint = 30;
static const size_t DxgiFormatR8G8B8A8Snorm = 31;
static const size_t DxgiFormatR8G8B8A8Sint = 32;
static const size_t DxgiFormatBC1Typeless = 70;
static const size_t DxgiFormatBC1Unorm = 71;
static const size_t DxgiFormatBC1UnormSrgb = 72;
static const size_t DxgiFormatBC2Typeless = 73;
static const size_t DxgiFormatBC2Unorm = 74;
static const size_t DxgiFormatBC2UnormSrgb = 75;
static const size_t DxgiFormatBC3Typeless = 76;
static const size_t DxgiFormatBC3Unorm = 77;
static const size_t DxgiFormatBC3UnormSrgb = 78;
static const size_t DxgiFormatB8G8R8A8Typeless = 90;
static const size_t DxgiFormatB8G8R8A8UnormSrgb = 91;
static const size_t DxgiFormatB8G8R8X8Typeless = 92;
static const size_t DxgiFormatB8G8R8X8UnormSrgb = 93;
} // namespace DdsLayout

Color DdsTextureReader::getAverageColor(
  const Assets::TextureBuffer& buffer, const GLenum format)
{
  if (format != GL_RGBA && format != GL_BGRA)
  {
    return Color{};
  }

  const unsigned char* const data = buffer.data();
  const std::size_t bufferSize = buffer.size();

  Color average;
  for (std::size_t i = 0; i < bufferSize; i += 4)
  {
    average = average + Color(data[i], data[i + 1], data[i + 2], data[i + 3]);
  }
  const std::size_t numPixels = bufferSize / 4;
  average = average / static_cast<float>(numPixels);

  return average;
}

static GLenum convertDx10FormatToGLFormat(const size_t dx10Format)
{
  switch (dx10Format)
  {
  case DdsLayout::DxgiFormatR8G8B8A8Typeless:
  case DdsLayout::DxgiFormatR8G8B8A8Unorm:
  case DdsLayout::DxgiFormatR8G8B8A8UnormSrgb:
  case DdsLayout::DxgiFormatR8G8B8A8Uint:
  case DdsLayout::DxgiFormatR8G8B8A8Snorm:
  case DdsLayout::DxgiFormatR8G8B8A8Sint:
    return GL_RGBA;
  case DdsLayout::DxgiFormatBC1Typeless:
  case DdsLayout::DxgiFormatBC1Unorm:
  case DdsLayout::DxgiFormatBC1UnormSrgb:
    return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
  case DdsLayout::DxgiFormatBC2Typeless:
  case DdsLayout::DxgiFormatBC2Unorm:
  case DdsLayout::DxgiFormatBC2UnormSrgb:
    return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
  case DdsLayout::DxgiFormatBC3Typeless:
  case DdsLayout::DxgiFormatBC3Unorm:
  case DdsLayout::DxgiFormatBC3UnormSrgb:
    return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
  case DdsLayout::DxgiFormatB8G8R8A8Typeless:
  case DdsLayout::DxgiFormatB8G8R8A8UnormSrgb:
  case DdsLayout::DxgiFormatB8G8R8X8Typeless:
  case DdsLayout::DxgiFormatB8G8R8X8UnormSrgb:
    return GL_BGRA;
  default:
    return 0;
  }
}


DdsTextureReader::DdsTextureReader(
  const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger)
  : TextureReader(nameStrategy, fs, logger)
{
}

Assets::Texture DdsTextureReader::doReadTexture(std::shared_ptr<File> file) const
{
  const auto& path = file->path();
  auto reader = file->reader().buffer();

  try
  {
    const auto ident = reader.readSize<uint32_t>();
    if (ident != DdsLayout::Ident)
    {
      return Assets::Texture(textureName(path), 16, 16);
    }

    /*const auto size =*/ reader.readSize<uint32_t>();
    const auto flags = reader.readSize<uint32_t>();
    const auto height = reader.readSize<uint32_t>();
    const auto width = reader.readSize<uint32_t>();
    const auto pitch = reader.readSize<uint32_t>();
    /*const auto depth =*/ reader.readSize<uint32_t>();
    const auto mipMapsCount = reader.readSize<uint32_t>();

    if (!checkTextureDimensions(width, height))
    {
      return Assets::Texture(textureName(path), 16, 16);
    }

    reader.seekFromBegin(DdsLayout::PixelFormatOffset);
    /*const auto ddpfSize =*/ reader.readSize<uint32_t>();
    const auto ddpfFlags = reader.readSize<uint32_t>();
    const auto ddpfFourcc = reader.readSize<uint32_t>();
    const auto ddpfRgbBitcount = reader.readSize<uint32_t>();
    const auto ddpfRBitMask = reader.readSize<uint32_t>();
    const auto ddpfGBitMask = reader.readSize<uint32_t>();
    const auto ddpfBBitMask = reader.readSize<uint32_t>();
    const auto ddpfABitMask = reader.readSize<uint32_t>();

    /*const auto caps =*/ reader.readSize<uint32_t>();
    const auto caps2 = reader.readSize<uint32_t>();

    reader.seekFromBegin(DdsLayout::BasicHeaderLengthWithIdent);

    const bool hasFourcc = (ddpfFlags & DdsLayout::DdpfFourcc) == DdsLayout::DdpfFourcc;
    const bool isDx10File = hasFourcc && ddpfFourcc == DdsLayout::IdentDx10;

    const size_t dx10Format = isDx10File ? reader.readSize<uint32_t>() : 0;
    const size_t dx10resDimension = isDx10File ? reader.readSize<uint32_t>() : 0;
    const size_t dx10MiscFlags = isDx10File ? reader.readSize<uint32_t>() : 0;

    GLenum format = 0;

    if (isDx10File)
    {
      if (
        dx10resDimension == DdsLayout::D3d10ResourceDimensionTexture2D
        && !(dx10MiscFlags & DdsLayout::D3d10ResourceMiscCubemap))
      {
        reader.seekFromBegin(
          DdsLayout::BasicHeaderLengthWithIdent + DdsLayout::Dx10HeaderLength);
        format = convertDx10FormatToGLFormat(dx10Format);
      }
    }
    else
    {
      if (!(caps2 & (DdsLayout::Ddcaps2Cubemap | DdsLayout::Ddcaps2Volume)))
      {
        if (hasFourcc)
        {
          if (ddpfFourcc == DdsLayout::FourccDXT1)
          {
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
          }
          else if (ddpfFourcc == DdsLayout::FourccDXT3)
          {
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
          }
          else if (ddpfFourcc == DdsLayout::FourccDXT5)
          {
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
          }
        }
        else
        {
          if (ddpfRgbBitcount == 24)
          {
            if (
              ddpfRBitMask == 0xFF && ddpfGBitMask == 0xFF00 && ddpfBBitMask == 0xFF0000)
            {
              format = GL_RGB;
            }
            else if (
              ddpfRBitMask == 0xFF0000 && ddpfGBitMask == 0xFF00 && ddpfBBitMask == 0xFF)
            {
              format = GL_BGR;
            }
          }
          else if (ddpfRgbBitcount == 32)
          {
            if (
              ddpfRBitMask == 0xFF && ddpfGBitMask == 0xFF00 && ddpfBBitMask == 0xFF0000
              && ddpfABitMask == 0xFF000000)
            {
              format = GL_RGBA;
            }
            else if (
              ddpfRBitMask == 0xFF0000 && ddpfGBitMask == 0xFF00 && ddpfBBitMask == 0xFF
              && ddpfABitMask == 0xFF000000)
            {
              format = GL_BGRA;
            }
          }
        }
      }
    }

    if (!format)
    {
      return Assets::Texture(textureName(path), 16, 16);
    }

    const size_t numMips = (mipMapsCount == 0) ? 1 : mipMapsCount;

    Assets::TextureBufferList buffers(numMips);
    Assets::setMipBufferSize(buffers, numMips, width, height, format);

    readDdsMips(width, height, reader, buffers);
    return Assets::Texture{
      textureName(path),
      width,
      height,
      Color{},
      std::move(buffers),
      format,
      Assets::TextureType::Opaque};
  }
  catch (const ReaderException&)
  {
    return Assets::Texture(textureName(path), 16, 16);
  }
}

void DdsTextureReader::readDdsMips(
  const size_t width,
  const size_t height,
  Reader& reader,
  Assets::TextureBufferList& buffers)
{
  unused(width);
  unused(height);
  for (size_t i = 0, mipLevels = buffers.size(); i < mipLevels; ++i)
  {
    reader.read(buffers[i].data(), buffers[i].size());
  }
}
} // namespace IO
} // namespace TrenchBroom
