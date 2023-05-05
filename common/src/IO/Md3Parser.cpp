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

#include "Md3Parser.h"

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "Exceptions.h"
#include "IO/Reader.h"
#include "IO/ResourceUtils.h"
#include "IO/SkinLoader.h"
#include "Logger.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include <kdl/path_utils.h>
#include <kdl/string_format.h>

#include <string>

namespace TrenchBroom
{
namespace IO
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
static const size_t TexCoordLength = 2 * sizeof(float);
static const size_t VertexLength = 4 * sizeof(int16_t);
static const float VertexScale = 1.0f / 64.0f;
} // namespace Md3Layout

Md3Parser::Md3Parser(const std::string& name, const Reader& reader, const FileSystem& fs)
  : m_name(name)
  , m_reader(reader)
  , m_fs(fs)
{
}

bool Md3Parser::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::str_to_lower(path.extension().string()) != ".md3")
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == Md3Layout::Ident && version == Md3Layout::Version;
}

std::unique_ptr<Assets::EntityModel> Md3Parser::doInitializeModel(Logger& logger)
{
  auto reader = m_reader;

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  if (ident != Md3Layout::Ident)
  {
    throw AssetException("Unknown MD3 model ident: " + std::to_string(ident));
  }
  if (version != Md3Layout::Version)
  {
    throw AssetException("Unknown MD3 model version: " + std::to_string(version));
  }

  /* const auto name = */ reader.readString(Md3Layout::ModelNameLength);
  /* const auto flags = */ reader.readInt<int32_t>();

  const auto frameCount = reader.readSize<int32_t>();
  /* const auto tagCount = */ reader.readSize<int32_t>();
  const auto surfaceCount = reader.readSize<int32_t>();
  /* const auto skinCount = */ reader.readSize<int32_t>();

  /* const auto frameOffset = */ reader.readSize<int32_t>();
  /* const auto tagOffset = */ reader.readSize<int32_t>();
  const auto surfaceOffset = reader.readSize<int32_t>();

  auto model = std::make_unique<Assets::EntityModel>(
    m_name, Assets::PitchType::Normal, Assets::Orientation::Oriented);
  for (size_t i = 0; i < frameCount; ++i)
  {
    model->addFrame();
  }

  parseSurfaces(reader.subReaderFromBegin(surfaceOffset), surfaceCount, *model, logger);

  return model;
}

void Md3Parser::doLoadFrame(
  const size_t frameIndex, Assets::EntityModel& model, Logger& /* logger */)
{
  auto reader = m_reader;

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  if (ident != Md3Layout::Ident)
  {
    throw AssetException("Unknown MD3 model ident: " + std::to_string(ident));
  }
  if (version != Md3Layout::Version)
  {
    throw AssetException("Unknown MD3 model version: " + std::to_string(version));
  }

  /* const auto name = */ reader.readString(Md3Layout::ModelNameLength);
  /* const auto flags = */ reader.readInt<int32_t>();

  /* const auto frameCount = */ reader.readSize<int32_t>();
  /* const auto tagCount = */ reader.readSize<int32_t>();
  /* const auto surfaceCount = */ reader.readSize<int32_t>();
  /* const auto skinCount = */ reader.readSize<int32_t>();

  const auto frameOffset = reader.readSize<int32_t>();
  /* const auto tagOffset = */ reader.readSize<int32_t>();
  const auto surfaceOffset = reader.readSize<int32_t>();

  auto& frame = parseFrame(
    reader.subReaderFromBegin(
      frameOffset + frameIndex * Md3Layout::FrameLength, Md3Layout::FrameLength),
    frameIndex,
    model);
  parseFrameSurfaces(reader.subReaderFromBegin(surfaceOffset), frame, model);
}

void Md3Parser::parseSurfaces(
  Reader reader, const size_t surfaceCount, Assets::EntityModel& model, Logger& logger)
{
  for (size_t i = 0; i < surfaceCount; ++i)
  {
    const auto ident = reader.readInt<int32_t>();

    if (ident != Md3Layout::Ident)
    {
      throw AssetException("Unknown MD3 model surface ident: " + std::to_string(ident));
    }

    const auto surfaceName = reader.readString(Md3Layout::SurfaceNameLength);
    /* const auto flags = */ reader.readInt<int32_t>();
    /* const auto frameCount = */ reader.readSize<int32_t>();
    const auto shaderCount = reader.readSize<int32_t>();
    /* const auto vertexCount = */ reader
      .readSize<int32_t>(); // the number of vertices per frame!
    /* const auto triangleCount = */ reader.readSize<int32_t>();

    /* const auto triangleOffset = */ reader.readSize<int32_t>();
    const auto shaderOffset = reader.readSize<int32_t>();
    /* const auto texCoordOffset = */ reader.readSize<int32_t>();
    /* const auto vertexOffset = */ reader
      .readSize<int32_t>(); // all vertices for all frames are stored there!
    const auto endOffset = reader.readSize<int32_t>();

    const auto shaders = parseShaders(
      reader.subReaderFromBegin(shaderOffset, shaderCount * Md3Layout::ShaderLength),
      shaderCount);

    auto& surface = model.addSurface(surfaceName);
    loadSurfaceSkins(surface, shaders, logger);

    reader = reader.subReaderFromBegin(endOffset);
  }
}

Assets::EntityModelLoadedFrame& Md3Parser::parseFrame(
  Reader reader, const size_t frameIndex, Assets::EntityModel& model)
{
  const auto minBounds = reader.readVec<float, 3>();
  const auto maxBounds = reader.readVec<float, 3>();
  /* const auto localOrigin = */ reader.readVec<float, 3>();
  /* const auto radius = */ reader.readFloat<float>();
  const auto frameName = reader.readString(Md3Layout::FrameNameLength);

  return model.loadFrame(frameIndex, frameName, vm::bbox3f(minBounds, maxBounds));
}

void Md3Parser::parseFrameSurfaces(
  Reader reader, Assets::EntityModelLoadedFrame& frame, Assets::EntityModel& model)
{
  for (size_t i = 0; i < model.surfaceCount(); ++i)
  {
    const auto ident = reader.readInt<int32_t>();

    if (ident != Md3Layout::Ident)
    {
      throw AssetException("Unknown MD3 model surface ident: " + std::to_string(ident));
    }

    /* const auto surfaceName = */ reader.readString(Md3Layout::SurfaceNameLength);
    /* const auto flags = */ reader.readInt<int32_t>();
    const auto frameCount = reader.readSize<int32_t>();
    /* const auto shaderCount = */ reader.readSize<int32_t>();
    const auto vertexCount =
      reader.readSize<int32_t>(); // the number of vertices per frame!
    const auto triangleCount = reader.readSize<int32_t>();

    const auto triangleOffset = reader.readSize<int32_t>();
    /* const auto shaderOffset = */ reader.readSize<int32_t>();
    const auto texCoordOffset = reader.readSize<int32_t>();
    const auto vertexOffset =
      reader.readSize<int32_t>(); // all vertices for all frames are stored there!
    const auto endOffset = reader.readSize<int32_t>();

    if (frameCount > 0)
    {
      const auto frameVertexLength = vertexCount * Md3Layout::VertexLength;
      const auto frameVertexOffset = vertexOffset + frame.index() * frameVertexLength;

      const auto vertexPositions = parseVertexPositions(
        reader.subReaderFromBegin(frameVertexOffset, frameVertexLength), vertexCount);
      const auto texCoords = parseTexCoords(
        reader.subReaderFromBegin(
          texCoordOffset, vertexCount * Md3Layout::TexCoordLength),
        vertexCount);
      const auto vertices = buildVertices(vertexPositions, texCoords);

      const auto triangles = parseTriangles(
        reader.subReaderFromBegin(
          triangleOffset, triangleCount * Md3Layout::TriangleLength),
        triangleCount);

      auto& surface = model.surface(i);
      buildFrameSurface(frame, surface, triangles, vertices);
    }

    reader = reader.subReaderFromBegin(endOffset);
  }
}

std::vector<Md3Parser::Md3Triangle> Md3Parser::parseTriangles(
  Reader reader, const size_t triangleCount)
{
  std::vector<Md3Triangle> result;
  result.reserve(triangleCount);
  for (size_t i = 0; i < triangleCount; ++i)
  {
    const auto i1 = reader.readSize<int32_t>();
    const auto i2 = reader.readSize<int32_t>();
    const auto i3 = reader.readSize<int32_t>();
    result.push_back(Md3Triangle{i1, i2, i3});
  }
  return result;
}

std::vector<std::filesystem::path> Md3Parser::parseShaders(
  Reader reader, const size_t shaderCount)
{
  std::vector<std::filesystem::path> result;
  result.reserve(shaderCount);
  for (size_t i = 0; i < shaderCount; ++i)
  {
    const auto shaderName = reader.readString(Md3Layout::ShaderNameLength);
    /* const auto shaderIndex = */ reader.readSize<int32_t>();
    result.emplace_back(shaderName);
  }
  return result;
}

std::vector<vm::vec3f> Md3Parser::parseVertexPositions(
  Reader reader, const size_t vertexCount)
{
  std::vector<vm::vec3f> result;
  result.reserve(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i)
  {
    const auto x = static_cast<float>(reader.readInt<int16_t>()) * Md3Layout::VertexScale;
    const auto y = static_cast<float>(reader.readInt<int16_t>()) * Md3Layout::VertexScale;
    const auto z = static_cast<float>(reader.readInt<int16_t>()) * Md3Layout::VertexScale;
    /* const auto n = */ reader.readInt<int16_t>();
    result.emplace_back(x, y, z);
  }
  return result;
}

std::vector<vm::vec2f> Md3Parser::parseTexCoords(Reader reader, const size_t vertexCount)
{
  std::vector<vm::vec2f> result;
  result.reserve(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i)
  {
    const auto s = reader.readFloat<float>();
    const auto t = reader.readFloat<float>();
    result.emplace_back(s, t);
  }
  return result;
}

std::vector<Assets::EntityModelVertex> Md3Parser::buildVertices(
  const std::vector<vm::vec3f>& positions, const std::vector<vm::vec2f>& texCoords)
{
  assert(positions.size() == texCoords.size());
  const auto vertexCount = positions.size();

  using Vertex = Assets::EntityModelVertex;
  std::vector<Vertex> result;
  result.reserve(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    result.emplace_back(positions[i], texCoords[i]);
  }

  return result;
}

void Md3Parser::loadSurfaceSkins(
  Assets::EntityModelSurface& surface,
  const std::vector<std::filesystem::path>& shaders,
  Logger& logger)
{
  std::vector<Assets::Texture> textures;
  textures.reserve(shaders.size());

  for (const auto& shader : shaders)
  {
    textures.push_back(loadShader(logger, shader));
  }

  surface.setSkins(std::move(textures));
}

Assets::Texture Md3Parser::loadShader(
  Logger& logger, const std::filesystem::path& path) const
{
  const auto shaderPath = kdl::path_remove_extension(path);
  return IO::loadShader(shaderPath, m_fs, logger);
}

void Md3Parser::buildFrameSurface(
  Assets::EntityModelLoadedFrame& frame,
  Assets::EntityModelSurface& surface,
  const std::vector<Md3Parser::Md3Triangle>& triangles,
  const std::vector<Assets::EntityModelVertex>& vertices)
{
  using Vertex = Assets::EntityModelVertex;

  const auto rangeMap =
    Renderer::IndexRangeMap(Renderer::PrimType::Triangles, 0, 3 * triangles.size());
  std::vector<Vertex> frameVertices;
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

  surface.addIndexedMesh(frame, std::move(frameVertices), std::move(rangeMap));
}
} // namespace IO
} // namespace TrenchBroom
