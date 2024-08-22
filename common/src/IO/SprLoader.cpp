/*
 Copyright (C) 2021 Kristian Duske

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

#include "IO/SprLoader.h"

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Assets/TextureResource.h"
#include "Color.h"
#include "Error.h"
#include "Exceptions.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"

#include "vm/bbox.h"
#include "vm/vec.h"

namespace TrenchBroom::IO
{

namespace
{

struct SprPicture
{
  Assets::Material material;
  int x;
  int y;
  size_t width;
  size_t height;
};

SprPicture parsePicture(Reader& reader, const Assets::Palette& palette)
{
  const auto xOffset = reader.readInt<int32_t>();
  const auto yOffset = reader.readInt<int32_t>();
  const auto width = reader.readSize<int32_t>();
  const auto height = reader.readSize<int32_t>();

  auto rgbaImage = Assets::TextureBuffer{4 * width * height};
  auto averageColor = Color{};
  palette.indexedToRgba(
    reader,
    width * height,
    rgbaImage,
    Assets::PaletteTransparency::Index255Transparent,
    averageColor);

  auto texture = Assets::Texture{
    width,
    height,
    averageColor,
    GL_RGBA,
    Assets::TextureMask::On,
    Assets::NoEmbeddedDefaults{},
    std::move(rgbaImage)};
  auto textureResource = createTextureResource(std::move(texture));

  auto material = Assets::Material{"", std::move(textureResource)};

  return SprPicture{std::move(material), xOffset, yOffset, width, height};
}

void skipPicture(Reader& reader)
{
  /* const auto xOffset = */ reader.readInt<int32_t>();
  /* const auto yOffset = */ reader.readInt<int32_t>();
  const auto width = reader.readSize<int32_t>();
  const auto height = reader.readSize<int32_t>();

  reader.seekForward(width * height);
}

SprPicture parsePictureFrame(Reader& reader, const Assets::Palette& palette)
{
  const auto group = reader.readInt<int32_t>();
  if (group == 0)
  { // single picture frame
    return parsePicture(reader, palette);
  }

  // multiple picture frame
  const auto pictureCount = reader.readSize<int32_t>();
  reader.seekForward(pictureCount * sizeof(float));

  auto picture = parsePicture(reader, palette);
  for (size_t i = 0; i < pictureCount - 1; ++i)
  {
    skipPicture(reader);
  }

  return picture;
}

Result<Assets::Orientation> parseSpriteOrientationType(Reader& reader)
{
  const auto type = reader.readInt<int32_t>();
  if (type < 0 || type > 4)
  {
    return Error{"Unknown SPR type: " + std::to_string(type)};
  }

  return static_cast<Assets::Orientation>(type);
}

/**
 * Specifies the render mode for a Goldsource sprite.
 * Affects the palette data.
 */
enum class RenderMode : int32_t
{
  /** No alpha channel, just plain RGB */
  Normal = 0,
  /** Normal but also R+G+B/3 is the alpha channel */
  Additive = 1,
  /** Index 255 is the colour, R+G+B/3 is the alpha channel */
  IndexAlpha = 2,
  /** Standard Quake behaviour, Palette index 255 = transparent */
  AlphaTest = 3
};

Result<RenderMode> parseSpriteRenderMode(const int version, Reader& reader)
{
  if (version != 2)
  {
    return RenderMode::IndexAlpha;
  }

  const auto mode = reader.readInt<int32_t>();
  if (mode < 0 || mode > 3)
  {
    return Error{"Unknown SPR render mode: " + std::to_string(mode)};
  }

  return static_cast<RenderMode>(mode);
}

std::vector<unsigned char> processGoldsourcePalette(
  const RenderMode mode, const std::vector<unsigned char>& data)
{
  // Convert the data into a Goldsource palette
  auto processed = std::vector<unsigned char>{};
  processed.reserve(1024);

  for (size_t i = 0; i < 256; ++i)
  {
    const auto r = data[3 * i + 0];
    const auto g = data[3 * i + 1];
    const auto b = data[3 * i + 2];

    // add the RGB channels - for IndexAlpha, the RGB is always index 255
    if (mode == RenderMode::IndexAlpha)
    {
      processed.push_back(data[0xFF * 3 + 0]);
      processed.push_back(data[0xFF * 3 + 1]);
      processed.push_back(data[0xFF * 3 + 2]);
    }
    else
    {
      processed.push_back(r);
      processed.push_back(g);
      processed.push_back(b);
    }

    // Add the alpha channel
    switch (mode)
    {
    case RenderMode::Normal:
      processed.push_back(0xFF);
      break;
    case RenderMode::Additive:
    case RenderMode::IndexAlpha: {
      const auto average = std::round(static_cast<float>(r + g + b) / 3.0f);
      processed.push_back(static_cast<unsigned char>(average));
      break;
    }
    case RenderMode::AlphaTest:
      processed.push_back(static_cast<unsigned char>(i == 0xFF ? 0 : 0xFF));
      break;
    }
  }

  return processed;
}

Result<Assets::Palette> parseEmbeddedPalette(
  Reader& reader,
  const RenderMode renderMode,
  const int version,
  const Assets::Palette& defaultPalette)
{
  if (version != 2)
  {
    return defaultPalette;
  }

  const auto paletteSize = reader.readSize<int16_t>();
  if (paletteSize != 256)
  {
    return Error{
      "Incorrect SPR palette size: expected 256, got " + std::to_string(paletteSize)};
  }
  auto data = std::vector<unsigned char>(paletteSize * 3);
  reader.read(data.data(), data.size());
  data = processGoldsourcePalette(renderMode, data);
  return Assets::makePalette(data, Assets::PaletteColorFormat::Rgba);
}

} // namespace

SprLoader::SprLoader(
  std::string name, const Reader& reader, const Assets::Palette& palette)
  : m_name{std::move(name)}
  , m_reader{reader}
  , m_palette{palette}
{
}

bool SprLoader::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::path_to_lower(path.extension()) != ".spr")
  {
    return false;
  }

  const auto ident = reader.readString(4);
  const auto version = reader.readInt<int32_t>();

  return ident == "IDSP" && (version == 1 || version == 2);
}

Result<Assets::EntityModelData> SprLoader::load(Logger& /* logger */)
{
  // see https://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_6.htm#CSPRF

  // Half-Life sprites (SPR version 2) are the same as Quake sprites, except
  // there is an additional integer in the header (render mode), and the palette
  // data is embedded after the header instead of using the external palette file.

  try
  {
    auto reader = m_reader;

    const auto ident = reader.readString(4);
    if (ident != "IDSP")
    {
      return Error{"Unknown SPR ident: " + ident};
    }

    // Version 1: Quake SPR format
    // Version 2: Half-Life SPR format
    const auto version = reader.readInt<int32_t>();
    if (version != 1 && version != 2)
    {
      return Error{"Unknown SPR version: " + std::to_string(version)};
    }

    return parseSpriteOrientationType(reader).and_then([&](const auto orientationType) {
      return parseSpriteRenderMode(version, reader).and_then([&](const auto renderMode) {
        /* const auto radius = */ reader.readFloat<float>();
        /* const auto maxWidth = */ reader.readSize<int32_t>();
        /* const auto maxHeight = */ reader.readSize<int32_t>();
        const auto frameCount = reader.readSize<int32_t>();
        /* const auto beamLength = */ reader.readFloat<float>();
        /* const auto synchtype = */ reader.readInt<int32_t>();

        return parseEmbeddedPalette(reader, renderMode, version, m_palette)
          .transform([&](auto palette) {
            auto data =
              Assets::EntityModelData{Assets::PitchType::Normal, orientationType};
            auto& surface = data.addSurface(m_name, frameCount);

            auto materials = std::vector<Assets::Material>{};
            materials.reserve(frameCount);

            for (size_t i = 0; i < frameCount; ++i)
            {
              auto pictureFrame = parsePictureFrame(reader, palette);
              materials.push_back(std::move(pictureFrame.material));

              const auto w = static_cast<float>(pictureFrame.width);
              const auto h = static_cast<float>(pictureFrame.height);
              const auto x1 = static_cast<float>(pictureFrame.x);
              const auto y1 = static_cast<float>(pictureFrame.y) - h;
              const auto x2 = x1 + w;
              const auto y2 = y1 + h;

              const auto bboxMin =
                vm::vec3f{vm::min(x1, x2), vm::min(x1, x2), vm::min(y1, y2)};
              const auto bboxMax =
                vm::vec3f{vm::max(x1, x2), vm::max(x1, x2), vm::max(y1, y2)};
              auto& modelFrame = data.addFrame(std::to_string(i), {bboxMin, bboxMax});
              modelFrame.setSkinOffset(i);

              const auto triangles = std::vector<Assets::EntityModelVertex>{
                Assets::EntityModelVertex{{x1, y1, 0}, {0, 1}},
                Assets::EntityModelVertex{{x1, y2, 0}, {0, 0}},
                Assets::EntityModelVertex{{x2, y2, 0}, {1, 0}},

                Assets::EntityModelVertex{{x2, y2, 0}, {1, 0}},
                Assets::EntityModelVertex{{x2, y1, 0}, {1, 1}},
                Assets::EntityModelVertex{{x1, y1, 0}, {0, 1}},
              };

              auto size = Renderer::IndexRangeMap::Size{};
              size.inc(Renderer::PrimType::Triangles, 2);

              auto builder =
                Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{6, size};
              builder.addTriangles(triangles);

              surface.addMesh(modelFrame, builder.vertices(), builder.indices());
            }

            surface.setSkins(std::move(materials));

            return data;
          });
      });
    });
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace TrenchBroom::IO
