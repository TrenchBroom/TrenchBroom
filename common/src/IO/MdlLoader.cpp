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

#include "MdlLoader.h"

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Assets/TextureResource.h"
#include "Error.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/result.h"
#include "kdl/string_format.h"

#include <fmt/format.h>

#include <string>
#include <vector>

namespace TrenchBroom::IO
{

namespace MdlLayout
{
static const int Ident = (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I');
static const int Version6 = 6;

static const unsigned int HeaderNumSkins = 0x30;
static const unsigned int Skins = 0x54;
static const unsigned int SimpleFrameName = 0x8;
static const unsigned int SimpleFrameLength = 0x10;
static const unsigned int MultiFrameTimes = 0xC;
// static const unsigned int FrameVertexSize   = 0x4;
} // namespace MdlLayout

namespace
{
const auto Normals = std::vector{
  vm::vec3f{-0.525731f, 0.000000f, 0.850651f},
  vm::vec3f{-0.442863f, 0.238856f, 0.864188f},
  vm::vec3f{-0.295242f, 0.000000f, 0.955423f},
  vm::vec3f{-0.309017f, 0.500000f, 0.809017f},
  vm::vec3f{-0.162460f, 0.262866f, 0.951056f},
  vm::vec3f{-0.000000f, 0.000000f, 1.000000f},
  vm::vec3f{-0.000000f, 0.850651f, 0.525731f},
  vm::vec3f{-0.147621f, 0.716567f, 0.681718f},
  vm::vec3f{-0.147621f, 0.716567f, 0.681718f},
  vm::vec3f{-0.000000f, 0.525731f, 0.850651f},
  vm::vec3f{-0.309017f, 0.500000f, 0.809017f},
  vm::vec3f{-0.525731f, 0.000000f, 0.850651f},
  vm::vec3f{-0.295242f, 0.000000f, 0.955423f},
  vm::vec3f{-0.442863f, 0.238856f, 0.864188f},
  vm::vec3f{-0.162460f, 0.262866f, 0.951056f},
  vm::vec3f{-0.681718f, 0.147621f, 0.716567f},
  vm::vec3f{-0.809017f, 0.309017f, 0.500000f},
  vm::vec3f{-0.587785f, 0.425325f, 0.688191f},
  vm::vec3f{-0.850651f, 0.525731f, 0.000000f},
  vm::vec3f{-0.864188f, 0.442863f, 0.238856f},
  vm::vec3f{-0.716567f, 0.681718f, 0.147621f},
  vm::vec3f{-0.688191f, 0.587785f, 0.425325f},
  vm::vec3f{-0.500000f, 0.809017f, 0.309017f},
  vm::vec3f{-0.238856f, 0.864188f, 0.442863f},
  vm::vec3f{-0.425325f, 0.688191f, 0.587785f},
  vm::vec3f{-0.716567f, 0.681718f, -0.147621f},
  vm::vec3f{-0.500000f, 0.809017f, -0.309017f},
  vm::vec3f{-0.525731f, 0.850651f, 0.000000f},
  vm::vec3f{-0.000000f, 0.850651f, -0.525731f},
  vm::vec3f{-0.238856f, 0.864188f, -0.442863f},
  vm::vec3f{-0.000000f, 0.955423f, -0.295242f},
  vm::vec3f{-0.262866f, 0.951056f, -0.162460f},
  vm::vec3f{-0.000000f, 1.000000f, 0.000000f},
  vm::vec3f{-0.000000f, 0.955423f, 0.295242f},
  vm::vec3f{-0.262866f, 0.951056f, 0.162460f},
  vm::vec3f{-0.238856f, 0.864188f, 0.442863f},
  vm::vec3f{-0.262866f, 0.951056f, 0.162460f},
  vm::vec3f{-0.500000f, 0.809017f, 0.309017f},
  vm::vec3f{-0.238856f, 0.864188f, -0.442863f},
  vm::vec3f{-0.262866f, 0.951056f, -0.162460f},
  vm::vec3f{-0.500000f, 0.809017f, -0.309017f},
  vm::vec3f{-0.850651f, 0.525731f, 0.000000f},
  vm::vec3f{-0.716567f, 0.681718f, 0.147621f},
  vm::vec3f{-0.716567f, 0.681718f, -0.147621f},
  vm::vec3f{-0.525731f, 0.850651f, 0.000000f},
  vm::vec3f{-0.425325f, 0.688191f, 0.587785f},
  vm::vec3f{-0.864188f, 0.442863f, 0.238856f},
  vm::vec3f{-0.688191f, 0.587785f, 0.425325f},
  vm::vec3f{-0.809017f, 0.309017f, 0.500000f},
  vm::vec3f{-0.681718f, 0.147621f, 0.716567f},
  vm::vec3f{-0.587785f, 0.425325f, 0.688191f},
  vm::vec3f{-0.955423f, 0.295242f, 0.000000f},
  vm::vec3f{1.000000f, 0.000000f, 0.000000f},
  vm::vec3f{-0.951056f, 0.162460f, 0.262866f},
  vm::vec3f{-0.850651f, -0.525731f, 0.000000f},
  vm::vec3f{-0.955423f, -0.295242f, 0.000000f},
  vm::vec3f{-0.864188f, -0.442863f, 0.238856f},
  vm::vec3f{-0.951056f, -0.162460f, 0.262866f},
  vm::vec3f{-0.809017f, -0.309017f, 0.500000f},
  vm::vec3f{-0.681718f, -0.147621f, 0.716567f},
  vm::vec3f{-0.850651f, 0.000000f, 0.525731f},
  vm::vec3f{-0.864188f, 0.442863f, -0.238856f},
  vm::vec3f{-0.809017f, 0.309017f, -0.500000f},
  vm::vec3f{-0.951056f, 0.162460f, -0.262866f},
  vm::vec3f{-0.525731f, 0.000000f, -0.850651f},
  vm::vec3f{-0.681718f, 0.147621f, -0.716567f},
  vm::vec3f{-0.681718f, -0.147621f, -0.716567f},
  vm::vec3f{-0.850651f, 0.000000f, -0.525731f},
  vm::vec3f{-0.809017f, -0.309017f, -0.500000f},
  vm::vec3f{-0.864188f, -0.442863f, -0.238856f},
  vm::vec3f{-0.951056f, -0.162460f, -0.262866f},
  vm::vec3f{-0.147621f, 0.716567f, -0.681718f},
  vm::vec3f{-0.309017f, 0.500000f, -0.809017f},
  vm::vec3f{-0.425325f, 0.688191f, -0.587785f},
  vm::vec3f{-0.442863f, 0.238856f, -0.864188f},
  vm::vec3f{-0.587785f, 0.425325f, -0.688191f},
  vm::vec3f{-0.688191f, 0.587785f, -0.425325f},
  vm::vec3f{-0.147621f, 0.716567f, -0.681718f},
  vm::vec3f{-0.309017f, 0.500000f, -0.809017f},
  vm::vec3f{-0.000000f, 0.525731f, -0.850651f},
  vm::vec3f{-0.525731f, 0.000000f, -0.850651f},
  vm::vec3f{-0.442863f, 0.238856f, -0.864188f},
  vm::vec3f{-0.295242f, 0.000000f, -0.955423f},
  vm::vec3f{-0.162460f, 0.262866f, -0.951056f},
  vm::vec3f{-0.000000f, 0.000000f, -1.000000f},
  vm::vec3f{-0.295242f, 0.000000f, -0.955423f},
  vm::vec3f{-0.162460f, 0.262866f, -0.951056f},
  vm::vec3f{-0.442863f, -0.238856f, -0.864188f},
  vm::vec3f{-0.309017f, -0.500000f, -0.809017f},
  vm::vec3f{-0.162460f, -0.262866f, -0.951056f},
  vm::vec3f{-0.000000f, -0.850651f, -0.525731f},
  vm::vec3f{-0.147621f, -0.716567f, -0.681718f},
  vm::vec3f{-0.147621f, -0.716567f, -0.681718f},
  vm::vec3f{-0.000000f, -0.525731f, -0.850651f},
  vm::vec3f{-0.309017f, -0.500000f, -0.809017f},
  vm::vec3f{-0.442863f, -0.238856f, -0.864188f},
  vm::vec3f{-0.162460f, -0.262866f, -0.951056f},
  vm::vec3f{-0.238856f, -0.864188f, -0.442863f},
  vm::vec3f{-0.500000f, -0.809017f, -0.309017f},
  vm::vec3f{-0.425325f, -0.688191f, -0.587785f},
  vm::vec3f{-0.716567f, -0.681718f, -0.147621f},
  vm::vec3f{-0.688191f, -0.587785f, -0.425325f},
  vm::vec3f{-0.587785f, -0.425325f, -0.688191f},
  vm::vec3f{-0.000000f, -0.955423f, -0.295242f},
  vm::vec3f{-0.000000f, -1.000000f, 0.000000f},
  vm::vec3f{-0.262866f, -0.951056f, -0.162460f},
  vm::vec3f{-0.000000f, -0.850651f, 0.525731f},
  vm::vec3f{-0.000000f, -0.955423f, 0.295242f},
  vm::vec3f{-0.238856f, -0.864188f, 0.442863f},
  vm::vec3f{-0.262866f, -0.951056f, 0.162460f},
  vm::vec3f{-0.500000f, -0.809017f, 0.309017f},
  vm::vec3f{-0.716567f, -0.681718f, 0.147621f},
  vm::vec3f{-0.525731f, -0.850651f, 0.000000f},
  vm::vec3f{-0.238856f, -0.864188f, -0.442863f},
  vm::vec3f{-0.500000f, -0.809017f, -0.309017f},
  vm::vec3f{-0.262866f, -0.951056f, -0.162460f},
  vm::vec3f{-0.850651f, -0.525731f, 0.000000f},
  vm::vec3f{-0.716567f, -0.681718f, -0.147621f},
  vm::vec3f{-0.716567f, -0.681718f, 0.147621f},
  vm::vec3f{-0.525731f, -0.850651f, 0.000000f},
  vm::vec3f{-0.500000f, -0.809017f, 0.309017f},
  vm::vec3f{-0.238856f, -0.864188f, 0.442863f},
  vm::vec3f{-0.262866f, -0.951056f, 0.162460f},
  vm::vec3f{-0.864188f, -0.442863f, 0.238856f},
  vm::vec3f{-0.809017f, -0.309017f, 0.500000f},
  vm::vec3f{-0.688191f, -0.587785f, 0.425325f},
  vm::vec3f{-0.681718f, -0.147621f, 0.716567f},
  vm::vec3f{-0.442863f, -0.238856f, 0.864188f},
  vm::vec3f{-0.587785f, -0.425325f, 0.688191f},
  vm::vec3f{-0.309017f, -0.500000f, 0.809017f},
  vm::vec3f{-0.147621f, -0.716567f, 0.681718f},
  vm::vec3f{-0.425325f, -0.688191f, 0.587785f},
  vm::vec3f{-0.162460f, -0.262866f, 0.951056f},
  vm::vec3f{-0.442863f, -0.238856f, 0.864188f},
  vm::vec3f{-0.162460f, -0.262866f, 0.951056f},
  vm::vec3f{-0.309017f, -0.500000f, 0.809017f},
  vm::vec3f{-0.147621f, -0.716567f, 0.681718f},
  vm::vec3f{-0.000000f, -0.525731f, 0.850651f},
  vm::vec3f{-0.425325f, -0.688191f, 0.587785f},
  vm::vec3f{-0.587785f, -0.425325f, 0.688191f},
  vm::vec3f{-0.688191f, -0.587785f, 0.425325f},
  vm::vec3f{-0.955423f, 0.295242f, 0.000000f},
  vm::vec3f{-0.951056f, 0.162460f, 0.262866f},
  vm::vec3f{-1.000000f, 0.000000f, 0.000000f},
  vm::vec3f{-0.850651f, 0.000000f, 0.525731f},
  vm::vec3f{-0.955423f, -0.295242f, 0.000000f},
  vm::vec3f{-0.951056f, -0.162460f, 0.262866f},
  vm::vec3f{-0.864188f, 0.442863f, -0.238856f},
  vm::vec3f{-0.951056f, 0.162460f, -0.262866f},
  vm::vec3f{-0.809017f, 0.309017f, -0.500000f},
  vm::vec3f{-0.864188f, -0.442863f, -0.238856f},
  vm::vec3f{-0.951056f, -0.162460f, -0.262866f},
  vm::vec3f{-0.809017f, -0.309017f, -0.500000f},
  vm::vec3f{-0.681718f, 0.147621f, -0.716567f},
  vm::vec3f{-0.681718f, -0.147621f, -0.716567f},
  vm::vec3f{-0.850651f, 0.000000f, -0.525731f},
  vm::vec3f{-0.688191f, 0.587785f, -0.425325f},
  vm::vec3f{-0.587785f, 0.425325f, -0.688191f},
  vm::vec3f{-0.425325f, 0.688191f, -0.587785f},
  vm::vec3f{-0.425325f, -0.688191f, -0.587785f},
  vm::vec3f{-0.587785f, -0.425325f, -0.688191f},
  vm::vec3f{-0.688191f, -0.587785f, -0.425325f},
};

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
  Reader reader,
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

  return kdl::vec_transform(packedVertices, [&](const auto& vertex) {
    return unpackFrameVertex(vertex, origin, scale);
  });
}

auto makeFrameTriangles(
  const std::vector<MdlSkinTriangle>& triangles,
  const std::vector<MdlSkinVertex>& vertices,
  const std::vector<vm::vec3f>& positions,
  const size_t skinWidth,
  const size_t skinHeight)
{
  auto frameTriangles = std::vector<Assets::EntityModelVertex>{};
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
  Reader reader,
  Assets::EntityModelData& model,
  Assets::EntityModelSurface& surface,
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

  auto size = Renderer::IndexRangeMap::Size{};
  size.inc(Renderer::PrimType::Triangles, frameTriangles.size());

  auto builder = Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{
    frameTriangles.size() * 3, size};
  builder.addTriangles(frameTriangles);

  auto& frame = model.addFrame(std::move(name), bounds.bounds());
  surface.addMesh(frame, std::move(builder.vertices()), std::move(builder.indices()));
}

void parseFrame(
  Reader& reader,
  Assets::EntityModelData& model,
  Assets::EntityModelSurface& surface,
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

std::vector<MdlSkinTriangle> parseTriangles(Reader& reader, size_t count)
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

std::vector<MdlSkinVertex> parseVertices(Reader& reader, size_t count)
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

Assets::Material parseSkin(
  Reader& reader,
  const size_t width,
  const size_t height,
  const int flags,
  std::string skinName,
  const Assets::Palette& palette)
{
  const auto size = width * height;
  const auto transparency = (flags & MF_HOLEY)
                              ? Assets::PaletteTransparency::Index255Transparent
                              : Assets::PaletteTransparency::Opaque;
  const auto mask = (transparency == Assets::PaletteTransparency::Index255Transparent)
                      ? Assets::TextureMask::On
                      : Assets::TextureMask::Off;
  auto avgColor = Color{};
  auto rgbaImage = Assets::TextureBuffer{size * 4};

  const auto skinGroup = reader.readSize<int32_t>();
  if (skinGroup == 0)
  {
    palette.indexedToRgba(reader, size, rgbaImage, transparency, avgColor);

    auto texture = Assets::Texture{
      width,
      height,
      avgColor,
      GL_RGBA,
      mask,
      Assets::NoEmbeddedDefaults{},
      std::move(rgbaImage)};

    auto textureResource = createTextureResource(std::move(texture));
    return Assets::Material{std::move(skinName), std::move(textureResource)};
  }

  const auto pictureCount = reader.readSize<int32_t>();
  reader.seekForward(pictureCount * 4); // skip the picture times

  palette.indexedToRgba(reader, size, rgbaImage, transparency, avgColor);
  reader.seekForward((pictureCount - 1) * size); // skip all remaining pictures

  auto texture = Assets::Texture{
    width,
    height,
    avgColor,
    GL_RGBA,
    mask,
    Assets::NoEmbeddedDefaults{},
    std::move(rgbaImage)};

  auto textureResource = createTextureResource(std::move(texture));
  return Assets::Material{std::move(skinName), std::move(textureResource)};
}

void parseSkins(
  Reader& reader,
  Assets::EntityModelSurface& surface,
  const size_t count,
  const size_t width,
  const size_t height,
  const int flags,
  const std::string& modelName,
  const Assets::Palette& palette)
{
  auto skins = std::vector<Assets::Material>{};
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

MdlLoader::MdlLoader(
  std::string name, const Reader& reader, const Assets::Palette& palette)
  : m_name{std::move(name)}
  , m_reader{reader}
  , m_palette{palette}
{
}

bool MdlLoader::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::str_to_lower(path.extension().string()) != ".mdl")
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == MdlLayout::Ident && version == MdlLayout::Version6;
}

Result<Assets::EntityModel> MdlLoader::load(Logger& /* logger */)
{
  try
  {
    auto reader = m_reader;

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

    auto data = Assets::EntityModelData{
      Assets::PitchType::MdlInverted, Assets::Orientation::Oriented};
    auto& surface = data.addSurface(m_name, frameCount);

    reader.seekFromBegin(MdlLayout::Skins);
    parseSkins(
      reader, surface, skinCount, skinWidth, skinHeight, flags, m_name, m_palette);

    const auto vertices = parseVertices(reader, vertexCount);
    const auto triangles = parseTriangles(reader, triangleCount);

    for (size_t i = 0; i < frameCount; ++i)
    {
      parseFrame(
        reader, data, surface, triangles, vertices, skinWidth, skinHeight, origin, scale);
    }

    return Assets::EntityModel{m_name, std::move(data)};
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace TrenchBroom::IO
