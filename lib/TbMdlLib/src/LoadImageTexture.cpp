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

#include "mdl/LoadImageTexture.h"

#include "fs/Reader.h"
#include "gl/Texture.h"
#include "gl/TextureBuffer.h"
#include "img/DecodeImage.h"
#include "img/ImageSupport.h"
#include "mdl/MaterialUtils.h"

#include "kd/contracts.h"
#include "kd/result.h"

#include <fmt/format.h>

#include <cstring>

namespace tb::mdl
{

Color getAverageColor(const gl::TextureBuffer& buffer, const GLenum format)
{
  contract_pre(format == GL_RGBA || format == GL_BGRA);

  const auto r = size_t(format == GL_RGBA ? 0 : 2);
  const auto g = size_t(1);
  const auto b = size_t(format == GL_RGBA ? 2 : 0);
  const auto a = size_t(3);

  const auto* const data = buffer.data();
  const auto bufferSize = buffer.size();
  const auto numPixels = bufferSize / 4;

  const auto stride = numPixels <= 4192 ? 1 : numPixels / 64;
  const auto numSamples = numPixels / stride;

  auto samples =
    std::views::iota(0u, numSamples) | std::views::transform([&](const auto& i) {
      const auto pixel = i * 4 * stride;
      return vm::vec4f{
        float(data[pixel + r]) / 255.0f,
        float(data[pixel + g]) / 255.0f,
        float(data[pixel + b]) / 255.0f,
        float(data[pixel + a]) / 255.0f,
      };
    });

  // can't use std::accumulate or std::reduce because samples isn't a common range
  auto average = vm::vec4f{};
  for (const auto& sample : samples)
  {
    average = average + sample;
  }

  average =
    vm::clamp(average / float(numSamples), vm::vec4f{0, 0, 0, 0}, vm::vec4f{1, 1, 1, 1});

  return RgbaF{average[0], average[1], average[2], average[3]};
}

Result<gl::Texture> loadImageTextureFromMemory(const uint8_t* begin, const size_t size)
{
  return img::decodeImage(begin, size)
         | kdl::and_then([](auto&& decodedImage) -> Result<gl::Texture> {
             if (!checkTextureDimensions(decodedImage.width, decodedImage.height))
             {
               return Error{fmt::format(
                 "Invalid texture dimensions: {}*{}",
                 decodedImage.width,
                 decodedImage.height)};
             }

             constexpr auto mipCount = 1u;
             constexpr auto format = GL_RGBA;

             auto buffers = gl::TextureBufferList{mipCount};
             setMipBufferSize(
               buffers, mipCount, decodedImage.width, decodedImage.height, format);

             contract_assert(buffers.at(0).size() == decodedImage.pixels.size());
             std::memcpy(
               buffers.at(0).data(),
               decodedImage.pixels.data(),
               decodedImage.pixels.size());

             const auto textureMask =
               decodedImage.hasTransparency ? gl::TextureMask::On : gl::TextureMask::Off;
             const auto averageColor = getAverageColor(buffers.at(0), format);

             return gl::Texture{
               decodedImage.width,
               decodedImage.height,
               averageColor,
               format,
               textureMask,
               gl::NoEmbeddedDefaults{},
               std::move(buffers)};
           });
}

Result<gl::Texture> loadImageTexture(fs::Reader& reader)
{
  auto bufferedReader = reader.buffer();
  const auto* begin = bufferedReader.begin();
  const auto* end = bufferedReader.end();
  const auto imageSize = size_t(end - begin);
  const auto* imageBegin = reinterpret_cast<const uint8_t*>(begin);

  return loadImageTextureFromMemory(imageBegin, imageSize);
}

bool isSupportedImageExtension(const std::filesystem::path& extension)
{
  return img::isSupportedExtension(extension);
}

} // namespace tb::mdl
