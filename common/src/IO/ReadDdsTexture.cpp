/*
 Copyright (C) 2023 iOrange
 Copyright (C) 2023 Kristian Duske

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

#include "ReadDdsTexture.h"

#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"

#include <kdl/result.h>

#include <fmt/format.h>

namespace TrenchBroom::IO
{
namespace
{
namespace DdsLayout
{
const std::size_t Ident = ((' ' << 24) + ('S' << 16) + ('D' << 8) + 'D');
const std::size_t IdentDx10 = (('0' << 24) + ('1' << 16) + ('X' << 8) + 'D');
const std::size_t BasicHeaderLengthWithIdent = 128;
const std::size_t PixelFormatOffset = 76;
const std::size_t Dx10HeaderLength = 20;

[[maybe_unused]] const std::size_t DdpfAlphaPixels = 1 << 0;
const std::size_t DdpfFourcc = 1 << 2;
[[maybe_unused]] const std::size_t DdpfRgb = 1 << 6;

const std::size_t Ddcaps2Cubemap = 1 << 9;
const std::size_t Ddcaps2CubemapPX = 1 << 10;
const std::size_t Ddcaps2CubemapNX = 1 << 11;
const std::size_t Ddcaps2CubemapPY = 1 << 12;
const std::size_t Ddcaps2CubemapNY = 1 << 13;
const std::size_t Ddcaps2CubemapPZ = 1 << 14;
const std::size_t Ddcaps2CubemapNZ = 1 << 15;
[[maybe_unused]] const std::size_t Ddcaps2CubemapAllFacesMask =
  (Ddcaps2CubemapPX | Ddcaps2CubemapNX | Ddcaps2CubemapPY | Ddcaps2CubemapNY
   | Ddcaps2CubemapPZ | Ddcaps2CubemapNZ);
const std::size_t Ddcaps2Volume = 1 << 21;

const std::size_t FourccDXT1 = (('1' << 24) + ('T' << 16) + ('X' << 8) + 'D');
const std::size_t FourccDXT3 = (('3' << 24) + ('T' << 16) + ('X' << 8) + 'D');
const std::size_t FourccDXT5 = (('5' << 24) + ('T' << 16) + ('X' << 8) + 'D');

const std::size_t D3d10ResourceMiscCubemap = 1 << 2;
const std::size_t D3d10ResourceDimensionTexture2D = 3;

const std::size_t DxgiFormatR8G8B8A8Typeless = 27;
const std::size_t DxgiFormatR8G8B8A8Unorm = 28;
const std::size_t DxgiFormatR8G8B8A8UnormSrgb = 29;
const std::size_t DxgiFormatR8G8B8A8Uint = 30;
const std::size_t DxgiFormatR8G8B8A8Snorm = 31;
const std::size_t DxgiFormatR8G8B8A8Sint = 32;
const std::size_t DxgiFormatBC1Typeless = 70;
const std::size_t DxgiFormatBC1Unorm = 71;
const std::size_t DxgiFormatBC1UnormSrgb = 72;
const std::size_t DxgiFormatBC2Typeless = 73;
const std::size_t DxgiFormatBC2Unorm = 74;
const std::size_t DxgiFormatBC2UnormSrgb = 75;
const std::size_t DxgiFormatBC3Typeless = 76;
const std::size_t DxgiFormatBC3Unorm = 77;
const std::size_t DxgiFormatBC3UnormSrgb = 78;
const std::size_t DxgiFormatB8G8R8A8Typeless = 90;
const std::size_t DxgiFormatB8G8R8A8UnormSrgb = 91;
const std::size_t DxgiFormatB8G8R8X8Typeless = 92;
const std::size_t DxgiFormatB8G8R8X8UnormSrgb = 93;
} // namespace DdsLayout

GLenum convertDx10FormatToGLFormat(const size_t dx10Format)
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

void readDdsMips(Reader& reader, Assets::TextureBufferList& buffers)
{
  for (size_t i = 0, mipLevels = buffers.size(); i < mipLevels; ++i)
  {
    reader.read(buffers[i].data(), buffers[i].size());
  }
}

} // namespace

Result<Assets::Texture, ReadTextureError> readDdsTexture(std::string name, Reader& reader)
{
  try
  {
    const auto ident = reader.readSize<uint32_t>();
    if (ident != DdsLayout::Ident)
    {
      return ReadTextureError{
        std::move(name), "Unknown Dds ident: " + std::to_string(ident)};
    }

    /*const auto size =*/reader.readSize<uint32_t>();
    /*const auto flags =*/reader.readSize<uint32_t>();
    const auto height = reader.readSize<uint32_t>();
    const auto width = reader.readSize<uint32_t>();
    /*const auto pitch =*/reader.readSize<uint32_t>();
    /*const auto depth =*/reader.readSize<uint32_t>();
    const auto mipMapsCount = reader.readSize<uint32_t>();

    if (!checkTextureDimensions(width, height))
    {
      return ReadTextureError{
        std::move(name), fmt::format("Invalid texture dimensions: {}*{}", width, height)};
    }

    reader.seekFromBegin(DdsLayout::PixelFormatOffset);
    /*const auto ddpfSize =*/reader.readSize<uint32_t>();
    const auto ddpfFlags = reader.readSize<uint32_t>();
    const auto ddpfFourcc = reader.readSize<uint32_t>();
    const auto ddpfRgbBitcount = reader.readSize<uint32_t>();
    const auto ddpfRBitMask = reader.readSize<uint32_t>();
    const auto ddpfGBitMask = reader.readSize<uint32_t>();
    const auto ddpfBBitMask = reader.readSize<uint32_t>();
    const auto ddpfABitMask = reader.readSize<uint32_t>();

    /*const auto caps =*/reader.readSize<uint32_t>();
    const auto caps2 = reader.readSize<uint32_t>();

    reader.seekFromBegin(DdsLayout::BasicHeaderLengthWithIdent);

    const auto hasFourcc = (ddpfFlags & DdsLayout::DdpfFourcc) == DdsLayout::DdpfFourcc;
    const auto isDx10File = hasFourcc && ddpfFourcc == DdsLayout::IdentDx10;

    const auto dx10Format = isDx10File ? reader.readSize<uint32_t>() : 0;
    const auto dx10resDimension = isDx10File ? reader.readSize<uint32_t>() : 0;
    const auto dx10MiscFlags = isDx10File ? reader.readSize<uint32_t>() : 0;

    auto format = GLenum{0};

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
      return ReadTextureError{
        std::move(name), "Invalid Dds texture format: " + std::to_string(format)};
    }

    const auto numMips = mipMapsCount ? mipMapsCount : 1;
    auto buffers = Assets::TextureBufferList{numMips};

    Assets::setMipBufferSize(buffers, numMips, width, height, format);
    readDdsMips(reader, buffers);

    return Assets::Texture{
      std::move(name),
      width,
      height,
      Color{},
      std::move(buffers),
      format,
      Assets::TextureType::Opaque};
  }
  catch (const ReaderException& e)
  {
    return ReadTextureError{std::move(name), e.what()};
  }
}

} // namespace TrenchBroom::IO
