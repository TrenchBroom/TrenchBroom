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

#include "LoadMdlModel.h"

#include "fs/ReaderException.h"
#include "mdl/Material.h"
#include "mdl/Palette.h"
#include "render/IndexRangeMap.h"
#include "render/IndexRangeMapBuilder.h"

#include "kd/path_utils.h"

namespace tb::mdl
{
namespace
{
namespace MdlLayout
{
constexpr int Ident = (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I');
constexpr int Version6 = 6;

constexpr unsigned int HeaderNumSkins = 0x30;
constexpr unsigned int Skins = 0x54;
constexpr unsigned int SimpleFrameName = 0x8;
constexpr unsigned int SimpleFrameLength = 0x10;
constexpr unsigned int MultiFrameTimes = 0xC;
} // namespace MdlLayout

const int MF_HOLEY = (1 << 14);

struct MdlSkinVertex
{
  bool onseam;
  int u;
  int v;
};

struct MdlSkinTriangle
{
  bool front;
  size_t vertices[3];
};

auto unpackFrameVertex(
  const vm::vec<unsigned char, 4>& vertex,
  const vm::vec3f& origin,
  const vm::vec3f& scale)
{
  auto result = vm::vec3f{};
  for (size_t i = 0; i < 3; ++i)
  {
    result[i] = origin[i] + scale[i] * float(vertex[i]);
  }
  return result;
}

auto parseFrameVertices(
  fs::Reader reader,
  const std::vector<MdlSkinVertex>& vertices,
  const vm::vec3f& origin,
  const vm::vec3f& scale)
{
  auto packedVertices = std::vector<vm::vec<unsigned char, 4>>{vertices.size()};
  for (size_t i = 0; i < vertices.size(); ++i)
  {
    for (size_t j = 0; j < 4; ++j)
    {
      packedVertices[i][j] = reader.readUnsignedChar<char>();
    }
  }

  return packedVertices | std::views::transform([&](const auto& vertex) {
           return unpackFrameVertex(vertex, origin, scale);
         })
         | kdl::ranges::to<std::vector>();
}

auto makeFrameTriangles(
  const std::vector<MdlSkinTriangle>& triangles,
  const std::vector<MdlSkinVertex>& vertices,
  const std::vector<vm::vec3f>& positions,
  const size_t skinWidth,
  const size_t skinHeight)
{
  auto frameTriangles = std::vector<EntityModelVertex>{};
  frameTriangles.reserve(triangles.size());

  for (size_t i = 0; i < triangles.size(); ++i)
  {
    const auto& triangle = triangles[i];
    for (size_t j = 0; j < 3; ++j)
    {
      const auto vertexIndex = triangle.vertices[j];
      const auto& skinVertex = vertices[vertexIndex];

      auto uv = vm::vec2f{
        float(skinVertex.u) / float(skinWidth), float(skinVertex.v) / float(skinHeight)};
      if (skinVertex.onseam && !triangle.front)
      {
        uv[0] += 0.5f;
      }

      frameTriangles.emplace_back(positions[vertexIndex], uv);
    }
  }

  return frameTriangles;
}

void doParseFrame(
  fs::Reader reader,
  EntityModelData& model,
  EntityModelSurface& surface,
  const std::vector<MdlSkinTriangle>& triangles,
  const std::vector<MdlSkinVertex>& vertices,
  const size_t skinWidth,
  const size_t skinHeight,
  const vm::vec3f& origin,
  const vm::vec3f& scale)
{
  reader.seekForward(MdlLayout::SimpleFrameName);
  auto name = reader.readString(MdlLayout::SimpleFrameLength);

  const auto positions = parseFrameVertices(reader, vertices, origin, scale);

  auto bounds = vm::bbox3f::builder{};
  bounds.add(positions.begin(), positions.end());

  const auto frameTriangles =
    makeFrameTriangles(triangles, vertices, positions, skinWidth, skinHeight);

  auto size = render::IndexRangeMap::Size{};
  size.inc(render::PrimType::Triangles, frameTriangles.size());

  auto builder = render::IndexRangeMapBuilder<EntityModelVertex::Type>{
    frameTriangles.size() * 3, size};
  builder.addTriangles(frameTriangles);

  auto& frame = model.addFrame(std::move(name), bounds.bounds());
  surface.addMesh(frame, std::move(builder.vertices()), std::move(builder.indices()));
}

void parseFrame(
  fs::Reader& reader,
  EntityModelData& model,
  EntityModelSurface& surface,
  const std::vector<MdlSkinTriangle>& triangles,
  const std::vector<MdlSkinVertex>& vertices,
  size_t skinWidth,
  size_t skinHeight,
  const vm::vec3f& origin,
  const vm::vec3f& scale)
{
  const auto frameLength =
    MdlLayout::SimpleFrameName + MdlLayout::SimpleFrameLength + vertices.size() * 4;

  const auto type = reader.readInt<int32_t>();
  if (type == 0)
  { // single frame
    doParseFrame(
      reader.subReaderFromCurrent(frameLength),
      model,
      surface,
      triangles,
      vertices,
      skinWidth,
      skinHeight,
      origin,
      scale);
    reader.seekForward(frameLength);
  }
  else
  { // frame group, but we only read the first frame
    const auto groupFrameCount = reader.readSize<int32_t>();
    reader.seekBackward(sizeof(int32_t));

    const auto frameTimeLength =
      MdlLayout::MultiFrameTimes + groupFrameCount * sizeof(float);
    doParseFrame(
      reader.subReaderFromCurrent(frameTimeLength, frameLength),
      model,
      surface,
      triangles,
      vertices,
      skinWidth,
      skinHeight,
      origin,
      scale);

    reader.seekForward(frameTimeLength + groupFrameCount * frameLength);
  }
}

std::vector<MdlSkinTriangle> parseTriangles(fs::Reader& reader, size_t count)
{
  auto triangles = std::vector<MdlSkinTriangle>{};
  triangles.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto front = reader.readBool<int32_t>();
    const auto v1 = reader.readSize<int32_t>();
    const auto v2 = reader.readSize<int32_t>();
    const auto v3 = reader.readSize<int32_t>();
    triangles.push_back(MdlSkinTriangle{front, {v1, v2, v3}});
  }
  return triangles;
}

std::vector<MdlSkinVertex> parseVertices(fs::Reader& reader, size_t count)
{
  auto vertices = std::vector<MdlSkinVertex>{};
  vertices.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto onSeam = reader.readBool<int32_t>();
    const auto u = reader.readInt<int32_t>();
    const auto v = reader.readInt<int32_t>();
    vertices.push_back(MdlSkinVertex{onSeam, u, v});
  }

  return vertices;
}

Material parseSkin(
  fs::Reader& reader,
  const size_t width,
  const size_t height,
  const int flags,
  std::string skinName,
  const Palette& palette)
{
  const auto size = width * height;
  const auto transparency = (flags & MF_HOLEY)
                              ? PaletteTransparency::Index255Transparent
                              : PaletteTransparency::Opaque;
  const auto mask = (transparency == PaletteTransparency::Index255Transparent)
                      ? TextureMask::On
                      : TextureMask::Off;
  auto avgColor = Color{RgbaF{}};
  auto rgbaImage = TextureBuffer{size * 4};

  const auto skinGroup = reader.readSize<int32_t>();
  if (skinGroup == 0)
  {
    palette.indexedToRgba(reader, size, rgbaImage, transparency, avgColor);

    auto texture = Texture{
      width,
      height,
      avgColor,
      GL_RGBA,
      mask,
      NoEmbeddedDefaults{},
      std::move(rgbaImage)};

    auto textureResource = createTextureResource(std::move(texture));
    return Material{std::move(skinName), std::move(textureResource)};
  }

  const auto pictureCount = reader.readSize<int32_t>();
  reader.seekForward(pictureCount * 4); // skip the picture times

  palette.indexedToRgba(reader, size, rgbaImage, transparency, avgColor);
  reader.seekForward((pictureCount - 1) * size); // skip all remaining pictures

  auto texture = Texture{
    width,
    height,
    avgColor,
    GL_RGBA,
    mask,
    NoEmbeddedDefaults{},
    std::move(rgbaImage)};

  auto textureResource = createTextureResource(std::move(texture));
  return Material{std::move(skinName), std::move(textureResource)};
}

void parseSkins(
  fs::Reader& reader,
  EntityModelSurface& surface,
  const size_t count,
  const size_t width,
  const size_t height,
  const int flags,
  const std::string& modelName,
  const Palette& palette)
{
  auto skins = std::vector<Material>{};
  skins.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    auto skinName = fmt::format("{}_{}", modelName, i);
    skins.push_back(
      parseSkin(reader, width, height, flags, std::move(skinName), palette));
  }

  surface.setSkins(std::move(skins));
}

} // namespace

bool canLoadMdlModel(const std::filesystem::path& path, fs::Reader reader)
{
  if (!kdl::path_has_extension(kdl::path_to_lower(path), ".mdl"))
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == MdlLayout::Ident && version == MdlLayout::Version6;
}

Result<EntityModelData> loadMdlModel(
  const std::string& name, fs::Reader reader, const Palette& palette, Logger&)
{
  try
  {
    const auto ident = reader.readInt<int32_t>();
    const auto version = reader.readInt<int32_t>();

    if (ident != MdlLayout::Ident)
    {
      return Error{fmt::format("Unknown MDL model ident: {}", ident)};
    }
    if (version != MdlLayout::Version6)
    {
      return Error{fmt::format("Unknown MDL model version: {}", version)};
    }

    const auto scale = reader.readVec<float, 3>();
    const auto origin = reader.readVec<float, 3>();

    reader.seekFromBegin(MdlLayout::HeaderNumSkins);
    const auto skinCount = reader.readSize<int32_t>();
    const auto skinWidth = reader.readSize<int32_t>();
    const auto skinHeight = reader.readSize<int32_t>();
    const auto vertexCount = reader.readSize<int32_t>();
    const auto triangleCount = reader.readSize<int32_t>();
    const auto frameCount = reader.readSize<int32_t>();
    /* const auto syncType = */ reader.readSize<int32_t>();
    const auto flags = reader.readInt<int32_t>();

    auto data =
      EntityModelData{PitchType::MdlInverted, Orientation::Oriented};
    auto& surface = data.addSurface(name, frameCount);

    reader.seekFromBegin(MdlLayout::Skins);
    parseSkins(reader, surface, skinCount, skinWidth, skinHeight, flags, name, palette);

    const auto vertices = parseVertices(reader, vertexCount);
    const auto triangles = parseTriangles(reader, triangleCount);

    for (size_t i = 0; i < frameCount; ++i)
    {
      parseFrame(
        reader, data, surface, triangles, vertices, skinWidth, skinHeight, origin, scale);
    }

    return data;
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
