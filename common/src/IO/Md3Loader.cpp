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

#include "Md3Loader.h"

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Error.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"
#include "Logger.h"
#include "Renderer/IndexRangeMapBuilder.h" // IWYU pragma: keep
#include "Renderer/PrimType.h"

#include "kdl/range_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h" // IWYU pragma: keep
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include <fmt/core.h>

#include <ranges>
#include <string>

namespace TrenchBroom::IO
{

namespace Md3Layout
{
static const int Ident = (('3' << 24) + ('P' << 16) + ('D' << 8) + 'I');
static const int Version = 15;
static const size_t ModelNameLength = 64;
static const size_t FrameNameLength = 16;
static const size_t FrameLength = 3 * 3 * sizeof(float) + sizeof(float) + FrameNameLength;
// static const size_t TagNameLength = 64;
// static const size_t TagLength = TagNameLength + 4 * 3 * sizeof(float);
static const size_t SurfaceNameLength = 64;
static const size_t TriangleLength = 3 * sizeof(int32_t);
static const size_t ShaderNameLength = 64;
static const size_t ShaderLength = ShaderNameLength + sizeof(int32_t);
static const size_t UVLength = 2 * sizeof(float);
static const size_t VertexLength = 4 * sizeof(int16_t);
static const float VertexScale = 1.0f / 64.0f;
} // namespace Md3Layout

namespace
{

struct Md3Triangle
{
  size_t i1, i2, i3;
};


auto parseShaders(Reader reader, const size_t shaderCount)
{
  auto shaders = std::vector<std::filesystem::path>{};
  shaders.reserve(shaderCount);

  for (size_t i = 0; i < shaderCount; ++i)
  {
    const auto shaderName = reader.readString(Md3Layout::ShaderNameLength);
    /* const auto shaderIndex = */ reader.readSize<int32_t>();
    shaders.emplace_back(shaderName);
  }

  return shaders;
}

void loadSurfaceMaterials(
  Assets::EntityModelSurface& surface,
  const std::vector<std::filesystem::path>& shaderPaths,
  const LoadMaterialFunc& loadMaterial)
{
  using std::views::transform;
  surface.setSkins(shaderPaths | transform(loadMaterial) | kdl::to_vector());
}

Result<void> parseSurfaces(
  Reader reader,
  const size_t surfaceCount,
  const size_t frameCount,
  Assets::EntityModelData& model,
  const LoadMaterialFunc& loadMaterial)
{
  for (size_t i = 0; i < surfaceCount; ++i)
  {
    const auto ident = reader.readInt<int32_t>();

    if (ident != Md3Layout::Ident)
    {
      return Error{fmt::format("Unknown MD3 model surface ident: {}", ident)};
    }

    const auto surfaceName = reader.readString(Md3Layout::SurfaceNameLength);
    /* const auto flags = */ reader.readInt<int32_t>();
    /* const auto frameCount = */ reader.readSize<int32_t>();
    const auto shaderCount = reader.readSize<int32_t>();
    /* const auto vertexCount = */ reader.readSize<int32_t>();
    /* const auto triangleCount = */ reader.readSize<int32_t>();

    /* const auto triangleOffset = */ reader.readSize<int32_t>();
    const auto shaderOffset = reader.readSize<int32_t>();
    /* const auto uvOffset = */ reader.readSize<int32_t>();
    /* const auto vertexOffset = */ reader.readSize<int32_t>();
    const auto endOffset = reader.readSize<int32_t>();

    const auto shaders = parseShaders(
      reader.subReaderFromBegin(shaderOffset, shaderCount * Md3Layout::ShaderLength),
      shaderCount);

    auto& surface = model.addSurface(surfaceName, frameCount);
    loadSurfaceMaterials(surface, shaders, loadMaterial);

    reader = reader.subReaderFromBegin(endOffset);
  }

  return Result<void>{};
}

auto& parseFrame(Reader reader, Assets::EntityModelData& model)
{
  const auto minBounds = reader.readVec<float, 3>();
  const auto maxBounds = reader.readVec<float, 3>();
  /* const auto localOrigin = */ reader.readVec<float, 3>();
  /* const auto radius = */ reader.readFloat<float>();
  const auto frameName = reader.readString(Md3Layout::FrameNameLength);

  return model.addFrame(frameName, vm::bbox3f{minBounds, maxBounds});
}

auto parseVertexPositions(Reader reader, const size_t vertexCount)
{
  auto positions = std::vector<vm::vec3f>{};
  positions.reserve(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    const auto x = static_cast<float>(reader.readInt<int16_t>()) * Md3Layout::VertexScale;
    const auto y = static_cast<float>(reader.readInt<int16_t>()) * Md3Layout::VertexScale;
    const auto z = static_cast<float>(reader.readInt<int16_t>()) * Md3Layout::VertexScale;
    /* const auto n = */ reader.readInt<int16_t>();
    positions.emplace_back(x, y, z);
  }

  return positions;
}

auto parseUV(Reader reader, const size_t vertexCount)
{
  auto uv = std::vector<vm::vec2f>{};
  uv.reserve(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    const auto u = reader.readFloat<float>();
    const auto v = reader.readFloat<float>();
    uv.emplace_back(u, v);
  }

  return uv;
}

auto buildVertices(
  const std::vector<vm::vec3f>& positions, const std::vector<vm::vec2f>& uvCoords)
{
  assert(positions.size() == uvCoords.size());
  const auto vertexCount = positions.size();

  using Vertex = Assets::EntityModelVertex;
  auto vertices = std::vector<Vertex>{};
  vertices.reserve(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    vertices.emplace_back(positions[i], uvCoords[i]);
  }

  return vertices;
}

auto parseTriangles(Reader reader, const size_t triangleCount)
{
  auto triangles = std::vector<Md3Triangle>{};
  triangles.reserve(triangleCount);

  for (size_t i = 0; i < triangleCount; ++i)
  {
    const auto i1 = reader.readSize<int32_t>();
    const auto i2 = reader.readSize<int32_t>();
    const auto i3 = reader.readSize<int32_t>();
    triangles.push_back(Md3Triangle{i1, i2, i3});
  }

  return triangles;
}

void buildFrameSurface(
  Assets::EntityModelFrame& frame,
  Assets::EntityModelSurface& surface,
  const std::vector<Md3Triangle>& triangles,
  const std::vector<Assets::EntityModelVertex>& vertices)
{
  using Vertex = Assets::EntityModelVertex;

  auto rangeMap =
    Renderer::IndexRangeMap{Renderer::PrimType::Triangles, 0, 3 * triangles.size()};
  auto frameVertices = std::vector<Vertex>{};
  frameVertices.reserve(3 * triangles.size());

  for (const auto& triangle : triangles)
  {
    if (
      triangle.i1 >= vertices.size() || triangle.i2 >= vertices.size()
      || triangle.i3 >= vertices.size())
    {
      continue;
    }

    const auto& v1 = vertices[triangle.i1];
    const auto& v2 = vertices[triangle.i2];
    const auto& v3 = vertices[triangle.i3];

    frameVertices.push_back(v1);
    frameVertices.push_back(v2);
    frameVertices.push_back(v3);
  }

  surface.addMesh(frame, std::move(frameVertices), std::move(rangeMap));
}

Result<void> parseFrameSurfaces(
  Reader reader, Assets::EntityModelFrame& frame, Assets::EntityModelData& model)
{
  for (size_t i = 0; i < model.surfaceCount(); ++i)
  {
    const auto ident = reader.readInt<int32_t>();

    if (ident != Md3Layout::Ident)
    {
      return Error{fmt::format("Unknown MD3 model surface ident: {}", ident)};
    }

    /* const auto surfaceName = */ reader.readString(Md3Layout::SurfaceNameLength);
    /* const auto flags = */ reader.readInt<int32_t>();
    const auto frameCount = reader.readSize<int32_t>();
    /* const auto shaderCount = */ reader.readSize<int32_t>();
    const auto vertexCount = reader.readSize<int32_t>();
    const auto triangleCount = reader.readSize<int32_t>();

    const auto triangleOffset = reader.readSize<int32_t>();
    /* const auto shaderOffset = */ reader.readSize<int32_t>();
    const auto uvCoordOffset = reader.readSize<int32_t>();
    const auto vertexOffset = reader.readSize<int32_t>();
    const auto endOffset = reader.readSize<int32_t>();

    if (frameCount > 0)
    {
      const auto frameVertexLength = vertexCount * Md3Layout::VertexLength;
      const auto frameVertexOffset = vertexOffset + frame.index() * frameVertexLength;

      const auto vertexPositions = parseVertexPositions(
        reader.subReaderFromBegin(frameVertexOffset, frameVertexLength), vertexCount);
      const auto uvCoords = parseUV(
        reader.subReaderFromBegin(uvCoordOffset, vertexCount * Md3Layout::UVLength),
        vertexCount);
      const auto vertices = buildVertices(vertexPositions, uvCoords);

      const auto triangles = parseTriangles(
        reader.subReaderFromBegin(
          triangleOffset, triangleCount * Md3Layout::TriangleLength),
        triangleCount);

      auto& surface = model.surface(i);
      buildFrameSurface(frame, surface, triangles, vertices);
    }

    reader = reader.subReaderFromBegin(endOffset);
  }

  return Result<void>{};
}

} // namespace

Md3Loader::Md3Loader(
  std::string name, const Reader& reader, LoadMaterialFunc loadMaterial)
  : m_name{std::move(name)}
  , m_reader{reader}
  , m_loadMaterial{std::move(loadMaterial)}
{
}

bool Md3Loader::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::str_to_lower(path.extension().string()) != ".md3")
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == Md3Layout::Ident && version == Md3Layout::Version;
}

Result<Assets::EntityModelData> Md3Loader::load(Logger&)
{
  try
  {
    auto reader = m_reader;

    const auto ident = reader.readInt<int32_t>();
    const auto version = reader.readInt<int32_t>();

    if (ident != Md3Layout::Ident)
    {
      return Error{fmt::format("Unknown MD3 model ident: {}", ident)};
    }

    if (version != Md3Layout::Version)
    {
      return Error{fmt::format("Unknown MD3 model version: {}", version)};
    }

    /* const auto name = */ reader.readString(Md3Layout::ModelNameLength);
    /* const auto flags = */ reader.readInt<int32_t>();

    const auto frameCount = reader.readSize<int32_t>();
    /* const auto tagCount = */ reader.readSize<int32_t>();
    const auto surfaceCount = reader.readSize<int32_t>();
    /* const auto materialCount = */ reader.readSize<int32_t>();

    const auto frameOffset = reader.readSize<int32_t>();
    /* const auto tagOffset = */ reader.readSize<int32_t>();
    const auto surfaceOffset = reader.readSize<int32_t>();

    auto data =
      Assets::EntityModelData{Assets::PitchType::Normal, Assets::Orientation::Oriented};

    return parseSurfaces(
             reader.subReaderFromBegin(surfaceOffset),
             surfaceCount,
             frameCount,
             data,
             m_loadMaterial)
           | kdl::and_then([&]() {
               return kdl::vec_transform(
                        std::views::iota(0u, frameCount),
                        [&](const auto i) {
                          auto& frame = parseFrame(
                            reader.subReaderFromBegin(
                              frameOffset + i * Md3Layout::FrameLength,
                              Md3Layout::FrameLength),
                            data);
                          return parseFrameSurfaces(
                            reader.subReaderFromBegin(surfaceOffset), frame, data);
                        })
                      | kdl::fold() | kdl::transform([&]() { return std::move(data); });
             });
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace TrenchBroom::IO
