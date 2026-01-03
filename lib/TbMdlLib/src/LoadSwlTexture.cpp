/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/LoadSwlTexture.h"

#include "fs/Reader.h"
#include "fs/ReaderException.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <string>

namespace tb::mdl
{
namespace SwlLayout
{
constexpr size_t TextureNameLength = 64;
constexpr size_t AnimNameLength = 64;
constexpr size_t MipLevels = 4;
constexpr size_t PaletteSize = 1024;
} // namespace SwlLayout


Result<gl::Texture> loadSwlTexture(fs::Reader& reader)
{
  try
  {
    reader.seekForward(SwlLayout::TextureNameLength);
    const auto width = reader.readSize<uint32_t>();
    const auto height = reader.readSize<uint32_t>();

    auto paletteReader = reader.subReaderFromCurrent(SwlLayout::PaletteSize);
    reader.seekForward(SwlLayout::PaletteSize);

    reader.seekForward(4); // palcrc

    auto offsets = std::vector<size_t>{}; // offsets from the beginning of the file

    offsets.reserve(SwlLayout::MipLevels);

    for (size_t i = 0; i < SwlLayout::MipLevels; ++i)
    {
      offsets.push_back(reader.readSize<uint32_t>());
    }

    reader.seekForward(SwlLayout::AnimNameLength);

    reader.seekForward(4); // flags
    reader.seekForward(4); // contents
    reader.seekForward(2); // value
    reader.seekForward(2); // direct
    reader.seekForward(4); // animtime
    reader.seekForward(4); // nonlit
    reader.seekForward(2); // directangle
    reader.seekForward(2); // trans_angle
    reader.seekForward(4); // directstyle
    reader.seekForward(4); // translucence
    reader.seekForward(4); // friction
    reader.seekForward(4); // restitution
    reader.seekForward(4); // trans_mag
    reader.seekForward(4); // color[0]
    reader.seekForward(4); // color[1]
    reader.seekForward(4); // color[2]

    return mdl::loadPalette(paletteReader, mdl::PaletteColorFormat::Rgbx)
           | kdl::transform([&](const auto& palette) {
               auto mip0AverageColor = Color{};
               auto buffers = gl::TextureBufferList{};

               bool has_transparency = false;

               for (size_t mipLevel = 0; mipLevel < SwlLayout::MipLevels; ++mipLevel)
               {
                 const auto w = width / (size_t(1) << mipLevel);
                 const auto h = height / (size_t(1) << mipLevel);

                 reader.seekFromBegin(offsets[mipLevel]);

                 auto rgbaImage = gl::TextureBuffer{4 * w * h};

                 auto averageColor = Color{};
                 palette.indexedToRgba(
                   reader,
                   w * h,
                   rgbaImage,
                   mdl::PaletteTransparency::Opaque,
                   averageColor);

                 for (uint32_t i = 0; i < rgbaImage.size() / 4; i++)
                 {
                     uint32_t &c = ((uint32_t *) (rgbaImage.data()))[i];

                     // transparency
                     if (c == 0xFFFF00FF) {
                         has_transparency = true;
                         c = 0;
                     }
                 }
                 buffers.emplace_back(std::move(rgbaImage));

                 if (mipLevel == 0)
                 {
                   mip0AverageColor = averageColor;
                 }
               }

               return gl::Texture{
                 width,
                 height,
                 mip0AverageColor,
                 GL_RGBA,
                 has_transparency ? gl::TextureMask::On : gl::TextureMask::Off,
                 gl::NoEmbeddedDefaults{},
                 std::move(buffers)};
             });
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::io
