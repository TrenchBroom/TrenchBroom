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

#include "Md2Parser.h"

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Assets/Palette.h"
#include "Exceptions.h"
#include "IO/Reader.h"
#include "IO/SkinLoader.h"
#include "Renderer/GLVertex.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/path_utils.h"
#include "kdl/string_format.h"

#include <fmt/format.h>

#include <string>

namespace TrenchBroom::IO
{
namespace Md2Layout
{
static const int Ident = (('2' << 24) + ('P' << 16) + ('D' << 8) + 'I');
static const int Version = 8;
static const size_t SkinNameLength = 64;
static const size_t FrameNameLength = 16;
} // namespace Md2Layout

namespace
{
const vm::vec3f Normals[162] = {
  vm::vec3f{-0.525731f, 0.000000f, 0.850651f},
  vm::vec3f{-0.442863f, 0.238856f, 0.864188f},
  vm::vec3f{-0.295242f, 0.000000f, 0.955423f},
  vm::vec3f{-0.309017f, 0.500000f, 0.809017f},
  vm::vec3f{-0.162460f, 0.262866f, 0.951056f},
  vm::vec3f{0.000000f, 0.000000f, 1.000000f},
  vm::vec3f{0.000000f, 0.850651f, 0.525731f},
  vm::vec3f{-0.147621f, 0.716567f, 0.681718f},
  vm::vec3f{0.147621f, 0.716567f, 0.681718f},
  vm::vec3f{0.000000f, 0.525731f, 0.850651f},
  vm::vec3f{0.309017f, 0.500000f, 0.809017f},
  vm::vec3f{0.525731f, 0.000000f, 0.850651f},
  vm::vec3f{0.295242f, 0.000000f, 0.955423f},
  vm::vec3f{0.442863f, 0.238856f, 0.864188f},
  vm::vec3f{0.162460f, 0.262866f, 0.951056f},
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
  vm::vec3f{0.000000f, 0.850651f, -0.525731f},
  vm::vec3f{-0.238856f, 0.864188f, -0.442863f},
  vm::vec3f{0.000000f, 0.955423f, -0.295242f},
  vm::vec3f{-0.262866f, 0.951056f, -0.162460f},
  vm::vec3f{0.000000f, 1.000000f, 0.000000f},
  vm::vec3f{0.000000f, 0.955423f, 0.295242f},
  vm::vec3f{-0.262866f, 0.951056f, 0.162460f},
  vm::vec3f{0.238856f, 0.864188f, 0.442863f},
  vm::vec3f{0.262866f, 0.951056f, 0.162460f},
  vm::vec3f{0.500000f, 0.809017f, 0.309017f},
  vm::vec3f{0.238856f, 0.864188f, -0.442863f},
  vm::vec3f{0.262866f, 0.951056f, -0.162460f},
  vm::vec3f{0.500000f, 0.809017f, -0.309017f},
  vm::vec3f{0.850651f, 0.525731f, 0.000000f},
  vm::vec3f{0.716567f, 0.681718f, 0.147621f},
  vm::vec3f{0.716567f, 0.681718f, -0.147621f},
  vm::vec3f{0.525731f, 0.850651f, 0.000000f},
  vm::vec3f{0.425325f, 0.688191f, 0.587785f},
  vm::vec3f{0.864188f, 0.442863f, 0.238856f},
  vm::vec3f{0.688191f, 0.587785f, 0.425325f},
  vm::vec3f{0.809017f, 0.309017f, 0.500000f},
  vm::vec3f{0.681718f, 0.147621f, 0.716567f},
  vm::vec3f{0.587785f, 0.425325f, 0.688191f},
  vm::vec3f{0.955423f, 0.295242f, 0.000000f},
  vm::vec3f{1.000000f, 0.000000f, 0.000000f},
  vm::vec3f{0.951056f, 0.162460f, 0.262866f},
  vm::vec3f{0.850651f, -0.525731f, 0.000000f},
  vm::vec3f{0.955423f, -0.295242f, 0.000000f},
  vm::vec3f{0.864188f, -0.442863f, 0.238856f},
  vm::vec3f{0.951056f, -0.162460f, 0.262866f},
  vm::vec3f{0.809017f, -0.309017f, 0.500000f},
  vm::vec3f{0.681718f, -0.147621f, 0.716567f},
  vm::vec3f{0.850651f, 0.000000f, 0.525731f},
  vm::vec3f{0.864188f, 0.442863f, -0.238856f},
  vm::vec3f{0.809017f, 0.309017f, -0.500000f},
  vm::vec3f{0.951056f, 0.162460f, -0.262866f},
  vm::vec3f{0.525731f, 0.000000f, -0.850651f},
  vm::vec3f{0.681718f, 0.147621f, -0.716567f},
  vm::vec3f{0.681718f, -0.147621f, -0.716567f},
  vm::vec3f{0.850651f, 0.000000f, -0.525731f},
  vm::vec3f{0.809017f, -0.309017f, -0.500000f},
  vm::vec3f{0.864188f, -0.442863f, -0.238856f},
  vm::vec3f{0.951056f, -0.162460f, -0.262866f},
  vm::vec3f{0.147621f, 0.716567f, -0.681718f},
  vm::vec3f{0.309017f, 0.500000f, -0.809017f},
  vm::vec3f{0.425325f, 0.688191f, -0.587785f},
  vm::vec3f{0.442863f, 0.238856f, -0.864188f},
  vm::vec3f{0.587785f, 0.425325f, -0.688191f},
  vm::vec3f{0.688191f, 0.587785f, -0.425325f},
  vm::vec3f{-0.147621f, 0.716567f, -0.681718f},
  vm::vec3f{-0.309017f, 0.500000f, -0.809017f},
  vm::vec3f{0.000000f, 0.525731f, -0.850651f},
  vm::vec3f{-0.525731f, 0.000000f, -0.850651f},
  vm::vec3f{-0.442863f, 0.238856f, -0.864188f},
  vm::vec3f{-0.295242f, 0.000000f, -0.955423f},
  vm::vec3f{-0.162460f, 0.262866f, -0.951056f},
  vm::vec3f{0.000000f, 0.000000f, -1.000000f},
  vm::vec3f{0.295242f, 0.000000f, -0.955423f},
  vm::vec3f{0.162460f, 0.262866f, -0.951056f},
  vm::vec3f{-0.442863f, -0.238856f, -0.864188f},
  vm::vec3f{-0.309017f, -0.500000f, -0.809017f},
  vm::vec3f{-0.162460f, -0.262866f, -0.951056f},
  vm::vec3f{0.000000f, -0.850651f, -0.525731f},
  vm::vec3f{-0.147621f, -0.716567f, -0.681718f},
  vm::vec3f{0.147621f, -0.716567f, -0.681718f},
  vm::vec3f{0.000000f, -0.525731f, -0.850651f},
  vm::vec3f{0.309017f, -0.500000f, -0.809017f},
  vm::vec3f{0.442863f, -0.238856f, -0.864188f},
  vm::vec3f{0.162460f, -0.262866f, -0.951056f},
  vm::vec3f{0.238856f, -0.864188f, -0.442863f},
  vm::vec3f{0.500000f, -0.809017f, -0.309017f},
  vm::vec3f{0.425325f, -0.688191f, -0.587785f},
  vm::vec3f{0.716567f, -0.681718f, -0.147621f},
  vm::vec3f{0.688191f, -0.587785f, -0.425325f},
  vm::vec3f{0.587785f, -0.425325f, -0.688191f},
  vm::vec3f{0.000000f, -0.955423f, -0.295242f},
  vm::vec3f{0.000000f, -1.000000f, 0.000000f},
  vm::vec3f{0.262866f, -0.951056f, -0.162460f},
  vm::vec3f{0.000000f, -0.850651f, 0.525731f},
  vm::vec3f{0.000000f, -0.955423f, 0.295242f},
  vm::vec3f{0.238856f, -0.864188f, 0.442863f},
  vm::vec3f{0.262866f, -0.951056f, 0.162460f},
  vm::vec3f{0.500000f, -0.809017f, 0.309017f},
  vm::vec3f{0.716567f, -0.681718f, 0.147621f},
  vm::vec3f{0.525731f, -0.850651f, 0.000000f},
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
  vm::vec3f{0.442863f, -0.238856f, 0.864188f},
  vm::vec3f{0.162460f, -0.262866f, 0.951056f},
  vm::vec3f{0.309017f, -0.500000f, 0.809017f},
  vm::vec3f{0.147621f, -0.716567f, 0.681718f},
  vm::vec3f{0.000000f, -0.525731f, 0.850651f},
  vm::vec3f{0.425325f, -0.688191f, 0.587785f},
  vm::vec3f{0.587785f, -0.425325f, 0.688191f},
  vm::vec3f{0.688191f, -0.587785f, 0.425325f},
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

struct Md2Vertex
{
  unsigned char x, y, z;
  unsigned char normalIndex;
};

struct Md2Frame
{
  vm::vec3f scale;
  vm::vec3f offset;
  std::string name;
  std::vector<Md2Vertex> vertices;

  vm::vec3f vertex(size_t index) const
  {
    const auto& vertex = vertices[index];
    const auto position = vm::vec3f{float(vertex.x), float(vertex.y), float(vertex.z)};
    return position * scale + offset;
  }

  const vm::vec3f& normal(size_t index) const
  {
    const auto& vertex = vertices[index];
    return Normals[vertex.normalIndex];
  }
};

struct Md2MeshVertex
{
  size_t vertexIndex;
  vm::vec2f uv;
};

struct Md2Mesh
{
  Renderer::PrimType type;
  std::vector<Md2MeshVertex> vertices;
};


std::vector<std::string> parseSkins(Reader reader, const size_t count)
{
  auto skins = std::vector<std::string>{};
  skins.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    skins.push_back(reader.readString(Md2Layout::SkinNameLength));
  }

  return skins;
}

auto parseVertices(Reader& reader, const size_t vertexCount)
{
  auto vertices = std::vector<Md2Vertex>{};
  vertices.reserve(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    const auto x = reader.readUnsignedChar<char>();
    const auto y = reader.readUnsignedChar<char>();
    const auto z = reader.readUnsignedChar<char>();
    const auto normalIndex = reader.readUnsignedChar<char>();
    vertices.push_back({x, y, z, normalIndex});
  }

  return vertices;
}

auto parseFrame(Reader reader, const size_t /* frameIndex */, const size_t vertexCount)
{
  const auto scale = reader.readVec<float, 3>();
  const auto offset = reader.readVec<float, 3>();
  auto name = reader.readString(Md2Layout::FrameNameLength);
  auto vertices = parseVertices(reader, vertexCount);

  return Md2Frame{scale, offset, std::move(name), std::move(vertices)};
}

auto parseMeshVertices(Reader& reader, const size_t count)
{
  auto vertices = std::vector<Md2MeshVertex>{};
  vertices.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto u = reader.readFloat<float>();
    const auto v = reader.readFloat<float>();
    const auto vertexIndex = reader.readSize<int32_t>();
    vertices.push_back({vertexIndex, {u, v}});
  }

  return vertices;
}

auto parseMeshes(Reader reader, const size_t /* commandCount */)
{
  auto meshes = std::vector<Md2Mesh>{};

  // vertex count is signed, where < 0 indicates a triangle fan and > 0 indicates a
  // triangle strip
  while (!reader.eof())
  {
    const auto vertexCount = reader.readInt<int32_t>();

    const auto type = vertexCount < 0 ? Renderer::PrimType::TriangleFan
                                      : Renderer::PrimType::TriangleStrip;
    auto vertices = parseMeshVertices(reader, size_t(std::abs(vertexCount)));
    meshes.push_back({type, std::move(vertices)});
  }

  return meshes;
}

void loadSkins(
  Assets::EntityModelSurface& surface,
  const std::vector<std::string>& skins,
  const Assets::Palette& palette,
  const FileSystem& fs,
  Logger& logger)
{
  auto materials = std::vector<Assets::Material>{};
  materials.reserve(skins.size());

  for (const auto& skin : skins)
  {
    materials.push_back(loadSkin(skin, fs, palette, logger));
  }

  surface.setSkins(std::move(materials));
}

auto getVertices(const Md2Frame& frame, const std::vector<Md2MeshVertex>& meshVertices)
{
  auto vertices = std::vector<Assets::EntityModelVertex>{};
  vertices.reserve(meshVertices.size());

  for (const auto& md2MeshVertex : meshVertices)
  {
    const auto position = frame.vertex(md2MeshVertex.vertexIndex);
    const auto& uv = md2MeshVertex.uv;

    vertices.emplace_back(position, uv);
  }

  return vertices;
}

void buildFrame(
  Assets::EntityModel& model,
  Assets::EntityModelSurface& surface,
  const size_t frameIndex,
  const Md2Frame& frame,
  const std::vector<Md2Mesh>& meshes)
{
  size_t vertexCount = 0;
  auto size = Renderer::IndexRangeMap::Size{};
  for (const auto& md2Mesh : meshes)
  {
    vertexCount += md2Mesh.vertices.size();
    size.inc(md2Mesh.type);
  }

  auto bounds = vm::bbox3f::builder{};

  auto builder =
    Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{vertexCount, size};
  for (const auto& md2Mesh : meshes)
  {
    if (!md2Mesh.vertices.empty())
    {
      vertexCount += md2Mesh.vertices.size();
      const auto vertices = getVertices(frame, md2Mesh.vertices);

      bounds.add(
        std::begin(vertices), std::end(vertices), Renderer::GetVertexComponent<0>());

      if (md2Mesh.type == Renderer::PrimType::TriangleFan)
      {
        builder.addTriangleFan(vertices);
      }
      else if (md2Mesh.type == Renderer::PrimType::TriangleStrip)
      {
        builder.addTriangleStrip(vertices);
      }
    }
  }

  auto& modelFrame = model.loadFrame(frameIndex, frame.name, bounds.bounds());
  surface.addMesh(
    modelFrame, std::move(builder.vertices()), std::move(builder.indices()));
}

} // namespace

Md2Parser::Md2Parser(
  std::string name,
  const Reader& reader,
  const Assets::Palette& palette,
  const FileSystem& fs)
  : m_name{std::move(name)}
  , m_reader{reader}
  , m_palette{palette}
  , m_fs{fs}
{
}

bool Md2Parser::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::path_to_lower(path.extension()) != ".md2")
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == Md2Layout::Ident && version == Md2Layout::Version;
}

// http://tfc.duke.free.fr/old/models/md2.htm
std::unique_ptr<Assets::EntityModel> Md2Parser::initializeModel(Logger& logger)
{
  auto reader = m_reader;
  const int ident = reader.readInt<int32_t>();
  const int version = reader.readInt<int32_t>();

  if (ident != Md2Layout::Ident)
  {
    throw AssetException{fmt::format("Unknown MD2 model ident: {}", ident)};
  }

  if (version != Md2Layout::Version)
  {
    throw AssetException{fmt::format("Unknown MD2 model version: {}", version)};
  }

  /*const auto skinWidth =*/reader.readSize<int32_t>();
  /*const auto skinHeight =*/reader.readSize<int32_t>();
  /*const auto frameSize =*/reader.readSize<int32_t>();

  const auto skinCount = reader.readSize<int32_t>();
  const auto vertexCount = reader.readSize<int32_t>();
  /* const auto uvCoordCount =*/reader.readSize<int32_t>();
  /* const auto triangleCount =*/reader.readSize<int32_t>();
  const auto commandCount = reader.readSize<int32_t>();

  const auto frameCount = reader.readSize<int32_t>();
  const auto skinOffset = reader.readSize<int32_t>();
  /* const auto uvCoordOffset =*/reader.readSize<int32_t>();
  /* const auto triangleOffset =*/reader.readSize<int32_t>();
  const auto frameOffset = reader.readSize<int32_t>();
  const auto commandOffset = reader.readSize<int32_t>();

  const auto skins = parseSkins(reader.subReaderFromBegin(skinOffset), skinCount);

  auto model = std::make_unique<Assets::EntityModel>(
    m_name, Assets::PitchType::Normal, Assets::Orientation::Oriented);
  for (size_t i = 0; i < frameCount; ++i)
  {
    model->addFrame();
  }

  auto& surface = model->addSurface(m_name);
  loadSkins(surface, skins, m_palette, m_fs, logger);

  const auto frameSize = 6 * sizeof(float) + Md2Layout::FrameNameLength + vertexCount * 4;
  const auto meshes =
    parseMeshes(reader.subReaderFromBegin(commandOffset, commandCount * 4), commandCount);

  for (size_t i = 0; i < frameCount; ++i)
  {
    const auto frame = parseFrame(
      reader.subReaderFromBegin(frameOffset + i * frameSize, frameSize), i, vertexCount);

    buildFrame(*model, surface, i, frame, meshes);
  }

  return model;
}

} // namespace TrenchBroom::IO
