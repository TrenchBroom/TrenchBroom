/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/LoadUnrealTexture.h"

#include "Color.h"
#include "fs/Reader.h"
#include "fs/ReaderException.h"
#include "gl/Texture.h"
#include "gl/TextureBuffer.h"
#include "mdl/LoadUnrealPackage.h"
#include "mdl/MaterialUtils.h"

#include "kd/result.h"

#include <fmt/format.h>

#include <algorithm>
#include <array>

namespace tb::mdl
{
namespace
{

struct UnrealMip
{
  std::vector<unsigned char> data;
  int32_t uSize;
  int32_t vSize;
  uint8_t uBits;
  uint8_t vBits;
};

/**
 * TLazyArray<byte>. Port of uetex.py's `read_lazy_array_bytes`: the
 * SkipOffset is present only when FileVersion > 61 (strictly greater) -- the
 * exact boundary between Unreal (61) and UT99 (68). We always read
 * sequentially, so the skip offset itself is unused.
 */
std::vector<unsigned char> readUnrealLazyArrayBytes(
  const UnrealPackage& package, fs::Reader& reader)
{
  if (package.fileVersion > 61)
  {
    reader.readInt<int32_t>(); // SkipOffset, unused
  }
  const auto count = readUnrealCompactIndex(reader);
  auto data = std::vector<unsigned char>(size_t(count));
  reader.read(data.data(), data.size());
  return data;
}

/**
 * TArray<FMipmap>. Port of uetex.py's `read_mips`.
 */
std::vector<UnrealMip> readUnrealMips(const UnrealPackage& package, fs::Reader& reader)
{
  const auto count = readUnrealCompactIndex(reader);
  auto mips = std::vector<UnrealMip>{};
  mips.reserve(size_t(count));
  for (int32_t i = 0; i < count; ++i)
  {
    auto data = readUnrealLazyArrayBytes(package, reader);
    const auto uSize = reader.readInt<int32_t>();
    const auto vSize = reader.readInt<int32_t>();
    const auto uBits = reader.readUnsignedChar<uint8_t>();
    const auto vBits = reader.readUnsignedChar<uint8_t>();
    mips.push_back(UnrealMip{std::move(data), uSize, vSize, uBits, vBits});
  }
  return mips;
}

/**
 * UPalette body: TArray<FColor>, always 256 entries for a standard palette.
 * On-disk order is R, G, B, A (confirmed from UModel's FColor serializer);
 * a previous version of this port had B and R swapped -- don't repeat that.
 * Port of uetex.py's `read_fcolor_array`.
 */
std::vector<unsigned char> readUnrealPaletteColors(fs::Reader& reader)
{
  const auto count = readUnrealCompactIndex(reader);
  auto colors = std::vector<unsigned char>(size_t(count) * 4);
  reader.read(colors.data(), colors.size());
  return colors;
}

const UnrealProperty* findUnrealProperty(
  const std::vector<UnrealProperty>& props, const std::string& name)
{
  const auto it = std::find_if(
    props.begin(), props.end(), [&](const auto& p) { return p.name == name; });
  return it != props.end() ? &*it : nullptr;
}

/**
 * P8 decode. Unlike Quake's palette convention (index 255 transparent), UE1
 * masked textures treat palette index 0 as the transparent index, so this
 * writes RGBA directly from the package's own palette bytes instead of
 * going through mdl::Palette (whose Index255Transparent handling doesn't
 * match this convention). Port of uetex.py's per-pixel decode loop in
 * `main()`.
 */
Result<gl::Texture> decodeUnrealTexture(
  fs::Reader& reader,
  const UnrealPackage& package,
  const std::string& packageName,
  const std::string& textureName)
{
  try
  {
    const auto* texExport = package.findExport(textureName);
    if (!texExport || package.classNameOfExport(*texExport) != "Texture")
    {
      return Error{fmt::format("{}: texture '{}' not found", packageName, textureName)};
    }

    auto texReader = reader.subReaderFromBegin(
      size_t(texExport->serialOffset), size_t(texExport->serialSize));
    const auto props = readUnrealPropertyList(package, texReader);

    const auto* paletteProp = findUnrealProperty(props, "Palette");
    if (!paletteProp || !paletteProp->objectRef)
    {
      return Error{fmt::format(
        "{}: texture '{}' has no Palette property", packageName, textureName)};
    }

    const auto* palExport = package.findExport(*paletteProp->objectRef);
    if (!palExport || package.classNameOfExport(*palExport) != "Palette")
    {
      return Error{fmt::format(
        "{}: palette '{}' referenced by texture '{}' not found",
        packageName,
        *paletteProp->objectRef,
        textureName)};
    }

    auto palReader = reader.subReaderFromBegin(
      size_t(palExport->serialOffset), size_t(palExport->serialSize));
    readUnrealPropertyList(package, palReader); // usually just "None"
    const auto paletteColors = readUnrealPaletteColors(palReader);
    if (paletteColors.size() != 256 * 4)
    {
      return Error{fmt::format(
        "{}: palette '{}' does not have 256 entries",
        packageName,
        *paletteProp->objectRef)};
    }

    const auto mips = readUnrealMips(package, texReader);
    if (mips.empty())
    {
      return Error{
        fmt::format("{}: texture '{}' has no mips", packageName, textureName)};
    }

    const auto width = size_t(mips.front().uSize);
    const auto height = size_t(mips.front().vSize);
    if (!checkTextureDimensions(width, height))
    {
      return Error{fmt::format("Invalid texture dimensions: {}*{}", width, height)};
    }

    const auto* maskedProp = findUnrealProperty(props, "bMasked");
    const auto bMasked = maskedProp != nullptr && maskedProp->boolValue;
    const auto mask = bMasked ? gl::TextureMask::On : gl::TextureMask::Off;

    auto buffers = gl::TextureBufferList{};
    buffers.reserve(mips.size());
    auto averageColor = Color{RgbaF{}};

    for (size_t level = 0; level < mips.size(); ++level)
    {
      const auto& mip = mips[level];
      const auto pixelCount = size_t(mip.uSize) * size_t(mip.vSize);
      if (mip.data.size() != pixelCount)
      {
        return Error{fmt::format(
          "{}: texture '{}' mip {} has {} index bytes, expected {}",
          packageName,
          textureName,
          level,
          mip.data.size(),
          pixelCount)};
      }

      auto buffer = gl::TextureBuffer{pixelCount * 4};
      auto* dst = buffer.data();

      auto colorSum = std::array<uint32_t, 3>{0, 0, 0};
      for (size_t p = 0; p < pixelCount; ++p)
      {
        const auto index = size_t(mip.data[p]);
        const auto* color = &paletteColors[index * 4];
        dst[p * 4 + 0] = color[0];
        dst[p * 4 + 1] = color[1];
        dst[p * 4 + 2] = color[2];
        dst[p * 4 + 3] = (bMasked && index == 0) ? 0 : 255;

        colorSum[0] += uint32_t(color[0]);
        colorSum[1] += uint32_t(color[1]);
        colorSum[2] += uint32_t(color[2]);
      }

      if (level == 0 && pixelCount > 0)
      {
        averageColor = RgbaF{
          float(colorSum[0]) / (255.0f * float(pixelCount)),
          float(colorSum[1]) / (255.0f * float(pixelCount)),
          float(colorSum[2]) / (255.0f * float(pixelCount)),
          1.0f};
      }

      buffers.push_back(std::move(buffer));
    }

    return gl::Texture{
      width,
      height,
      averageColor,
      GL_RGBA,
      mask,
      gl::NoEmbeddedDefaults{},
      std::move(buffers)};
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace

Result<gl::Texture> loadUnrealTexture(
  fs::Reader& reader, const std::string& packageName, const std::string& textureName)
{
  return loadUnrealPackage(reader) | kdl::and_then([&](const auto& package) {
           return decodeUnrealTexture(reader, package, packageName, textureName);
         });
}

} // namespace tb::mdl
