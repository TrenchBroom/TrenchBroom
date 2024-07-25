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

#include "DkmParser.h"

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Error.h"
#include "IO/FileSystem.h"
#include "IO/PathInfo.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"
#include "IO/SkinLoader.h"
#include "IO/TraversalMode.h"
#include "Renderer/GLVertex.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/string_format.h"

#include "vm/vec.h"

#include <fmt/format.h>

#include <string>

namespace TrenchBroom::IO
{

namespace DkmLayout
{
static const int Ident = (('D' << 24) + ('M' << 16) + ('K' << 8) + 'D');
static const int Version1 = 1;
static const int Version2 = 2;
static const size_t SkinNameLength = 64;
static const size_t FrameNameLength = 16;
} // namespace DkmLayout

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

struct DkmVertex
{
  unsigned int x, y, z;
  unsigned char normalIndex;
};

struct DkmFrame
{
  vm::vec3f scale;
  vm::vec3f offset;
  std::string name;
  std::vector<DkmVertex> vertices;

  vm::vec3f vertex(size_t index) const
  {
    assert(index < vertices.size());

    const auto& vertex = vertices[index];
    const auto position = vm::vec3f{float(vertex.x), float(vertex.y), float(vertex.z)};
    return position * scale + offset;
  }

  const vm::vec3f& normal(size_t index) const
  {
    assert(index < vertices.size());

    const DkmVertex& vertex = vertices[index];
    return Normals[vertex.normalIndex];
  }
};

struct DkmMeshVertex
{
  size_t vertexIndex;
  vm::vec2f uv;
};

struct DkmMesh
{
  Renderer::PrimType type;
  std::vector<DkmMeshVertex> vertices;
};

auto parseSkins(Reader reader, const size_t count)
{
  auto skins = std::vector<std::string>{};
  skins.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    skins.push_back(reader.readString(DkmLayout::SkinNameLength));
  }

  return skins;
}

auto parseUnpackedVertex(Reader& reader)
{
  const auto x = reader.readUnsignedChar<char>();
  const auto y = reader.readUnsignedChar<char>();
  const auto z = reader.readUnsignedChar<char>();
  const auto normalIndex = reader.readUnsignedChar<char>();
  return DkmVertex{x, y, z, normalIndex};
}

auto parsePackedVertex(Reader& reader)
{
  const auto packedPosition = reader.read<uint32_t, uint32_t>();
  const auto normalIndex = reader.readUnsignedChar<char>();
  return DkmVertex{
    (packedPosition & 0xFFE00000) >> 21,
    (packedPosition & 0x1FF800) >> 11,
    (packedPosition & 0x7FF),
    normalIndex,
  };
}

auto parseVertices(Reader& reader, const size_t vertexCount, const int version)
{
  assert(version == 1 || version == 2);

  auto vertices = std::vector<DkmVertex>{};
  vertices.reserve(vertexCount);

  if (version == 1)
  {
    for (size_t i = 0; i < vertexCount; ++i)
    {
      vertices.push_back(parseUnpackedVertex(reader));
    }
  }
  else
  {
    /* Version 2 vertices are packed into a 32bit integer
     * X occupies the first 11 bits
     * Y occupies the following 10 bits
     * Z occupies the following 11 bits
     */
    for (size_t i = 0; i < vertexCount; ++i)
    {
      vertices.push_back(parsePackedVertex(reader));
    }
  }

  return vertices;
}

auto parseFrame(
  Reader reader,
  const size_t /* frameIndex */,
  const size_t vertexCount,
  const int version)
{
  const auto scale = reader.readVec<float, 3>();
  const auto offset = reader.readVec<float, 3>();
  auto name = reader.readString(DkmLayout::FrameNameLength);
  auto vertices = parseVertices(reader, vertexCount, version);

  return DkmFrame{
    scale,
    offset,
    std::move(name),
    std::move(vertices),
  };
}

auto parseMeshVertices(Reader& reader, const size_t count)
{
  auto vertices = std::vector<DkmMeshVertex>{};
  vertices.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto vertexIndex = reader.readSize<int32_t>();
    const auto u = reader.readFloat<float>();
    const auto v = reader.readFloat<float>();
    vertices.push_back({vertexIndex, {u, v}});
  }

  return vertices;
}

auto parseMeshes(Reader reader, const size_t /* commandCount */)
{
  auto meshes = std::vector<DkmMesh>{};

  // vertex count is signed, where < 0 indicates a triangle fan and > 0 indicates a
  // triangle strip
  auto vertexCount = reader.readInt<int32_t>();
  while (vertexCount != 0)
  {
    /* const auto skinIndex    = */ reader.readSize<int32_t>();
    /* const auto surfaceIndex = */ reader.readSize<int32_t>();

    const auto type = vertexCount < 0 ? Renderer::PrimType::TriangleFan
                                      : Renderer::PrimType::TriangleStrip;
    auto vertices = parseMeshVertices(reader, size_t(std::abs(vertexCount)));
    meshes.push_back({type, std::move(vertices)});

    vertexCount = reader.readInt<int32_t>();
  }

  return meshes;
}

/**
 * Daikatana's models contain wrong skin paths. They often refer to a skin like "x/y.bmp"
 * which does not exist, and the correct skin file name will be "x/y.wal" instead. That's
 * why we try to find a matching file name by disregarding the extension.
 */
Result<std::filesystem::path> findSkin(const std::string& skin, const FileSystem& fs)
{
  const auto skinPath = std::filesystem::path{skin};
  if (fs.pathInfo(skinPath) == PathInfo::File)
  {
    return skinPath;
  }

  // try "wal" extension instead
  if (kdl::str_to_lower(skinPath.extension().string()) == ".bmp")
  {
    const auto walPath = kdl::path_replace_extension(skinPath, ".wal");
    if (fs.pathInfo(walPath) == PathInfo::File)
    {
      return walPath;
    }
  }

  // Search for any file with the correct base name.
  const auto folder = skinPath.parent_path();
  const auto basename = skinPath.stem();
  return fs.find(
           folder, TraversalMode::Flat, makeFilenamePathMatcher(basename.string() + ".*"))
         | kdl::transform(
           [&](auto items) { return items.size() == 1 ? items.front() : skinPath; });
}

Result<void> loadSkins(
  Assets::EntityModelSurface& surface,
  const std::vector<std::string>& skins,
  const FileSystem& fs,
  Logger& logger)
{
  return kdl::vec_transform(
           skins,
           [&](const auto& skin) {
             return findSkin(skin, fs) | kdl::transform([&](const auto skinPath) {
                      return loadSkin(skinPath, fs, logger);
                    });
           })
         | kdl::fold() | kdl::transform([&](auto materials) {
             surface.setSkins(std::move(materials));
           });
}

auto getVertices(const DkmFrame& frame, const std::vector<DkmMeshVertex>& meshVertices)
{
  return kdl::vec_transform(meshVertices, [&](const auto& meshVertex) {
    const auto position = frame.vertex(meshVertex.vertexIndex);
    return Assets::EntityModelVertex{position, meshVertex.uv};
  });
}

void buildFrame(
  Assets::EntityModelData& model,
  Assets::EntityModelSurface& surface,
  const DkmFrame& frame,
  const std::vector<DkmMesh>& meshes)
{
  size_t vertexCount = 0;
  auto size = Renderer::IndexRangeMap::Size{};

  for (const auto& mesh : meshes)
  {
    vertexCount += mesh.vertices.size();
    size.inc(mesh.type);
  }

  auto bounds = vm::bbox3f::builder{};

  auto builder =
    Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{vertexCount, size};
  for (const auto& mesh : meshes)
  {
    if (!mesh.vertices.empty())
    {
      vertexCount += mesh.vertices.size();
      const auto vertices = getVertices(frame, mesh.vertices);

      bounds.add(vertices.begin(), vertices.end(), Renderer::GetVertexComponent<0>());
      if (mesh.type == Renderer::PrimType::TriangleStrip)
      {
        builder.addTriangleStrip(vertices);
      }
      else if (mesh.type == Renderer::PrimType::TriangleFan)
      {
        builder.addTriangleFan(vertices);
      }
    }
  }

  auto& modelFrame = model.addFrame(frame.name, bounds.bounds());
  surface.addMesh(
    modelFrame, std::move(builder.vertices()), std::move(builder.indices()));
}

} // namespace

DkmParser::DkmParser(std::string name, const Reader& reader, const FileSystem& fs)
  : m_name{std::move(name)}
  , m_reader{reader}
  , m_fs{fs}
{
}

bool DkmParser::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::path_to_lower(path.extension()) != ".dkm")
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == DkmLayout::Ident
         && (version == DkmLayout::Version1 || version == DkmLayout::Version2);
}

// http://tfc.duke.free.fr/old/models/md2.htm
Result<Assets::EntityModel> DkmParser::initializeModel(Logger& logger)
{
  try
  {
    auto reader = m_reader;

    const auto ident = reader.readInt<int32_t>();
    const auto version = reader.readInt<int32_t>();

    if (ident != DkmLayout::Ident)
    {
      return Error{fmt::format("Unknown DKM model ident: {}", ident)};
    }

    if (version != DkmLayout::Version1 && version != DkmLayout::Version2)
    {
      return Error{fmt::format("Unknown DKM model version: {}", version)};
    }

    /* const auto origin = */ reader.readVec<float, 3>();

    const auto frameSize = reader.readSize<int32_t>();

    const auto skinCount = reader.readSize<int32_t>();
    const auto vertexCount = reader.readSize<int32_t>();
    /* const auto uvCoordCount =*/reader.readSize<int32_t>();
    /* const auto triangleCount =*/reader.readSize<int32_t>();
    const auto commandCount = reader.readSize<int32_t>();
    const auto frameCount = reader.readSize<int32_t>();
    /* const auto surfaceCount =*/reader.readSize<int32_t>();

    const auto skinOffset = reader.readSize<int32_t>();
    /* const auto uvCoordOffset =*/reader.readSize<int32_t>();
    /* const auto triangleOffset =*/reader.readSize<int32_t>();
    const auto frameOffset = reader.readSize<int32_t>();
    const auto commandOffset = reader.readSize<int32_t>();
    /* const auto surfaceOffset =*/reader.readSize<int32_t>();

    const auto skins = parseSkins(reader.subReaderFromBegin(skinOffset), skinCount);

    auto data =
      Assets::EntityModelData{Assets::PitchType::Normal, Assets::Orientation::Oriented};

    auto& surface = data.addSurface(m_name, frameCount);
    return loadSkins(surface, skins, m_fs, logger).transform([&]() {
      const auto meshes = parseMeshes(
        reader.subReaderFromBegin(commandOffset, commandCount * 4), commandCount);

      for (size_t i = 0; i < frameCount; ++i)
      {
        const auto frame = parseFrame(
          reader.subReaderFromBegin(frameOffset + i * frameSize, frameSize),
          i,
          vertexCount,
          version);

        buildFrame(data, surface, frame, meshes);
      }

      return Assets::EntityModel{m_name, std::move(data)};
    });
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace TrenchBroom::IO
