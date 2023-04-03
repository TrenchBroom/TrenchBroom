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

#include "MdxParser.h"

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "Exceptions.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/SkinLoader.h"
#include "Renderer/GLVertex.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/string_format.h"

#include <string>

namespace TrenchBroom
{
namespace IO
{
const vm::vec3f MdxParser::Normals[162] = {
  vm::vec3f(-0.525731f, 0.000000f, 0.850651f),
  vm::vec3f(-0.442863f, 0.238856f, 0.864188f),
  vm::vec3f(-0.295242f, 0.000000f, 0.955423f),
  vm::vec3f(-0.309017f, 0.500000f, 0.809017f),
  vm::vec3f(-0.162460f, 0.262866f, 0.951056f),
  vm::vec3f(0.000000f, 0.000000f, 1.000000f),
  vm::vec3f(0.000000f, 0.850651f, 0.525731f),
  vm::vec3f(-0.147621f, 0.716567f, 0.681718f),
  vm::vec3f(0.147621f, 0.716567f, 0.681718f),
  vm::vec3f(0.000000f, 0.525731f, 0.850651f),
  vm::vec3f(0.309017f, 0.500000f, 0.809017f),
  vm::vec3f(0.525731f, 0.000000f, 0.850651f),
  vm::vec3f(0.295242f, 0.000000f, 0.955423f),
  vm::vec3f(0.442863f, 0.238856f, 0.864188f),
  vm::vec3f(0.162460f, 0.262866f, 0.951056f),
  vm::vec3f(-0.681718f, 0.147621f, 0.716567f),
  vm::vec3f(-0.809017f, 0.309017f, 0.500000f),
  vm::vec3f(-0.587785f, 0.425325f, 0.688191f),
  vm::vec3f(-0.850651f, 0.525731f, 0.000000f),
  vm::vec3f(-0.864188f, 0.442863f, 0.238856f),
  vm::vec3f(-0.716567f, 0.681718f, 0.147621f),
  vm::vec3f(-0.688191f, 0.587785f, 0.425325f),
  vm::vec3f(-0.500000f, 0.809017f, 0.309017f),
  vm::vec3f(-0.238856f, 0.864188f, 0.442863f),
  vm::vec3f(-0.425325f, 0.688191f, 0.587785f),
  vm::vec3f(-0.716567f, 0.681718f, -0.147621f),
  vm::vec3f(-0.500000f, 0.809017f, -0.309017f),
  vm::vec3f(-0.525731f, 0.850651f, 0.000000f),
  vm::vec3f(0.000000f, 0.850651f, -0.525731f),
  vm::vec3f(-0.238856f, 0.864188f, -0.442863f),
  vm::vec3f(0.000000f, 0.955423f, -0.295242f),
  vm::vec3f(-0.262866f, 0.951056f, -0.162460f),
  vm::vec3f(0.000000f, 1.000000f, 0.000000f),
  vm::vec3f(0.000000f, 0.955423f, 0.295242f),
  vm::vec3f(-0.262866f, 0.951056f, 0.162460f),
  vm::vec3f(0.238856f, 0.864188f, 0.442863f),
  vm::vec3f(0.262866f, 0.951056f, 0.162460f),
  vm::vec3f(0.500000f, 0.809017f, 0.309017f),
  vm::vec3f(0.238856f, 0.864188f, -0.442863f),
  vm::vec3f(0.262866f, 0.951056f, -0.162460f),
  vm::vec3f(0.500000f, 0.809017f, -0.309017f),
  vm::vec3f(0.850651f, 0.525731f, 0.000000f),
  vm::vec3f(0.716567f, 0.681718f, 0.147621f),
  vm::vec3f(0.716567f, 0.681718f, -0.147621f),
  vm::vec3f(0.525731f, 0.850651f, 0.000000f),
  vm::vec3f(0.425325f, 0.688191f, 0.587785f),
  vm::vec3f(0.864188f, 0.442863f, 0.238856f),
  vm::vec3f(0.688191f, 0.587785f, 0.425325f),
  vm::vec3f(0.809017f, 0.309017f, 0.500000f),
  vm::vec3f(0.681718f, 0.147621f, 0.716567f),
  vm::vec3f(0.587785f, 0.425325f, 0.688191f),
  vm::vec3f(0.955423f, 0.295242f, 0.000000f),
  vm::vec3f(1.000000f, 0.000000f, 0.000000f),
  vm::vec3f(0.951056f, 0.162460f, 0.262866f),
  vm::vec3f(0.850651f, -0.525731f, 0.000000f),
  vm::vec3f(0.955423f, -0.295242f, 0.000000f),
  vm::vec3f(0.864188f, -0.442863f, 0.238856f),
  vm::vec3f(0.951056f, -0.162460f, 0.262866f),
  vm::vec3f(0.809017f, -0.309017f, 0.500000f),
  vm::vec3f(0.681718f, -0.147621f, 0.716567f),
  vm::vec3f(0.850651f, 0.000000f, 0.525731f),
  vm::vec3f(0.864188f, 0.442863f, -0.238856f),
  vm::vec3f(0.809017f, 0.309017f, -0.500000f),
  vm::vec3f(0.951056f, 0.162460f, -0.262866f),
  vm::vec3f(0.525731f, 0.000000f, -0.850651f),
  vm::vec3f(0.681718f, 0.147621f, -0.716567f),
  vm::vec3f(0.681718f, -0.147621f, -0.716567f),
  vm::vec3f(0.850651f, 0.000000f, -0.525731f),
  vm::vec3f(0.809017f, -0.309017f, -0.500000f),
  vm::vec3f(0.864188f, -0.442863f, -0.238856f),
  vm::vec3f(0.951056f, -0.162460f, -0.262866f),
  vm::vec3f(0.147621f, 0.716567f, -0.681718f),
  vm::vec3f(0.309017f, 0.500000f, -0.809017f),
  vm::vec3f(0.425325f, 0.688191f, -0.587785f),
  vm::vec3f(0.442863f, 0.238856f, -0.864188f),
  vm::vec3f(0.587785f, 0.425325f, -0.688191f),
  vm::vec3f(0.688191f, 0.587785f, -0.425325f),
  vm::vec3f(-0.147621f, 0.716567f, -0.681718f),
  vm::vec3f(-0.309017f, 0.500000f, -0.809017f),
  vm::vec3f(0.000000f, 0.525731f, -0.850651f),
  vm::vec3f(-0.525731f, 0.000000f, -0.850651f),
  vm::vec3f(-0.442863f, 0.238856f, -0.864188f),
  vm::vec3f(-0.295242f, 0.000000f, -0.955423f),
  vm::vec3f(-0.162460f, 0.262866f, -0.951056f),
  vm::vec3f(0.000000f, 0.000000f, -1.000000f),
  vm::vec3f(0.295242f, 0.000000f, -0.955423f),
  vm::vec3f(0.162460f, 0.262866f, -0.951056f),
  vm::vec3f(-0.442863f, -0.238856f, -0.864188f),
  vm::vec3f(-0.309017f, -0.500000f, -0.809017f),
  vm::vec3f(-0.162460f, -0.262866f, -0.951056f),
  vm::vec3f(0.000000f, -0.850651f, -0.525731f),
  vm::vec3f(-0.147621f, -0.716567f, -0.681718f),
  vm::vec3f(0.147621f, -0.716567f, -0.681718f),
  vm::vec3f(0.000000f, -0.525731f, -0.850651f),
  vm::vec3f(0.309017f, -0.500000f, -0.809017f),
  vm::vec3f(0.442863f, -0.238856f, -0.864188f),
  vm::vec3f(0.162460f, -0.262866f, -0.951056f),
  vm::vec3f(0.238856f, -0.864188f, -0.442863f),
  vm::vec3f(0.500000f, -0.809017f, -0.309017f),
  vm::vec3f(0.425325f, -0.688191f, -0.587785f),
  vm::vec3f(0.716567f, -0.681718f, -0.147621f),
  vm::vec3f(0.688191f, -0.587785f, -0.425325f),
  vm::vec3f(0.587785f, -0.425325f, -0.688191f),
  vm::vec3f(0.000000f, -0.955423f, -0.295242f),
  vm::vec3f(0.000000f, -1.000000f, 0.000000f),
  vm::vec3f(0.262866f, -0.951056f, -0.162460f),
  vm::vec3f(0.000000f, -0.850651f, 0.525731f),
  vm::vec3f(0.000000f, -0.955423f, 0.295242f),
  vm::vec3f(0.238856f, -0.864188f, 0.442863f),
  vm::vec3f(0.262866f, -0.951056f, 0.162460f),
  vm::vec3f(0.500000f, -0.809017f, 0.309017f),
  vm::vec3f(0.716567f, -0.681718f, 0.147621f),
  vm::vec3f(0.525731f, -0.850651f, 0.000000f),
  vm::vec3f(-0.238856f, -0.864188f, -0.442863f),
  vm::vec3f(-0.500000f, -0.809017f, -0.309017f),
  vm::vec3f(-0.262866f, -0.951056f, -0.162460f),
  vm::vec3f(-0.850651f, -0.525731f, 0.000000f),
  vm::vec3f(-0.716567f, -0.681718f, -0.147621f),
  vm::vec3f(-0.716567f, -0.681718f, 0.147621f),
  vm::vec3f(-0.525731f, -0.850651f, 0.000000f),
  vm::vec3f(-0.500000f, -0.809017f, 0.309017f),
  vm::vec3f(-0.238856f, -0.864188f, 0.442863f),
  vm::vec3f(-0.262866f, -0.951056f, 0.162460f),
  vm::vec3f(-0.864188f, -0.442863f, 0.238856f),
  vm::vec3f(-0.809017f, -0.309017f, 0.500000f),
  vm::vec3f(-0.688191f, -0.587785f, 0.425325f),
  vm::vec3f(-0.681718f, -0.147621f, 0.716567f),
  vm::vec3f(-0.442863f, -0.238856f, 0.864188f),
  vm::vec3f(-0.587785f, -0.425325f, 0.688191f),
  vm::vec3f(-0.309017f, -0.500000f, 0.809017f),
  vm::vec3f(-0.147621f, -0.716567f, 0.681718f),
  vm::vec3f(-0.425325f, -0.688191f, 0.587785f),
  vm::vec3f(-0.162460f, -0.262866f, 0.951056f),
  vm::vec3f(0.442863f, -0.238856f, 0.864188f),
  vm::vec3f(0.162460f, -0.262866f, 0.951056f),
  vm::vec3f(0.309017f, -0.500000f, 0.809017f),
  vm::vec3f(0.147621f, -0.716567f, 0.681718f),
  vm::vec3f(0.000000f, -0.525731f, 0.850651f),
  vm::vec3f(0.425325f, -0.688191f, 0.587785f),
  vm::vec3f(0.587785f, -0.425325f, 0.688191f),
  vm::vec3f(0.688191f, -0.587785f, 0.425325f),
  vm::vec3f(-0.955423f, 0.295242f, 0.000000f),
  vm::vec3f(-0.951056f, 0.162460f, 0.262866f),
  vm::vec3f(-1.000000f, 0.000000f, 0.000000f),
  vm::vec3f(-0.850651f, 0.000000f, 0.525731f),
  vm::vec3f(-0.955423f, -0.295242f, 0.000000f),
  vm::vec3f(-0.951056f, -0.162460f, 0.262866f),
  vm::vec3f(-0.864188f, 0.442863f, -0.238856f),
  vm::vec3f(-0.951056f, 0.162460f, -0.262866f),
  vm::vec3f(-0.809017f, 0.309017f, -0.500000f),
  vm::vec3f(-0.864188f, -0.442863f, -0.238856f),
  vm::vec3f(-0.951056f, -0.162460f, -0.262866f),
  vm::vec3f(-0.809017f, -0.309017f, -0.500000f),
  vm::vec3f(-0.681718f, 0.147621f, -0.716567f),
  vm::vec3f(-0.681718f, -0.147621f, -0.716567f),
  vm::vec3f(-0.850651f, 0.000000f, -0.525731f),
  vm::vec3f(-0.688191f, 0.587785f, -0.425325f),
  vm::vec3f(-0.587785f, 0.425325f, -0.688191f),
  vm::vec3f(-0.425325f, 0.688191f, -0.587785f),
  vm::vec3f(-0.425325f, -0.688191f, -0.587785f),
  vm::vec3f(-0.587785f, -0.425325f, -0.688191f),
  vm::vec3f(-0.688191f, -0.587785f, -0.425325f)};

MdxParser::MdxFrame::MdxFrame(const size_t vertexCount)
  : name("")
  , vertices(vertexCount)
{
}

vm::vec3f MdxParser::MdxFrame::vertex(const size_t index) const
{
  const MdxVertex& vertex = vertices[index];
  const vm::vec3f position(
    static_cast<float>(vertex.x),
    static_cast<float>(vertex.y),
    static_cast<float>(vertex.z));
  return position * scale + offset;
}

const vm::vec3f& MdxParser::MdxFrame::normal(const size_t index) const
{
  const MdxVertex& vertex = vertices[index];
  return Normals[vertex.normalIndex];
}

MdxParser::MdxMesh::MdxMesh(const int i_vertexCount)
  : type(i_vertexCount < 0 ? Fan : Strip)
  , vertexCount(static_cast<size_t>(i_vertexCount < 0 ? -i_vertexCount : i_vertexCount))
  , vertices(vertexCount)
{
}

MdxParser::MdxParser(const std::string& name, const Reader& reader, const FileSystem& fs)
  : m_name(name)
  , m_reader(reader)
  , m_fs(fs)
{
}

bool MdxParser::canParse(const Path& path, Reader reader)
{
  if (kdl::str_to_lower(path.extension()) != "mdx")
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  return ident == MdxLayout::Ident && version == MdxLayout::Version;
}

// http://tfc.duke.free.fr/old/models/md2.htm
std::unique_ptr<Assets::EntityModel> MdxParser::doInitializeModel(Logger& logger)
{
  auto reader = m_reader;
  const int ident = reader.readInt<int32_t>();
  const int version = reader.readInt<int32_t>();

  if (ident != MdxLayout::Ident)
  {
    throw AssetException("Unknown MDX model ident: " + std::to_string(ident));
  }
  if (version != MdxLayout::Version)
  {
    throw AssetException("Unknown MDX model version: " + std::to_string(version));
  }

  /*const size_t skinWidth =*/reader.readSize<int32_t>();
  /*const size_t skinHeight =*/reader.readSize<int32_t>();
  /*const size_t frameSize =*/reader.readSize<int32_t>();

  const size_t skinCount = reader.readSize<int32_t>();
  /* const size_t vertexCount = */ reader.readSize<int32_t>();
  /* const size_t triangleCount =*/reader.readSize<int32_t>();
  /* const size_t commandCount = */ reader.readSize<int32_t>();
  const size_t frameCount = reader.readSize<int32_t>();

  /* const size_t sfxDefineCount = */ reader.readSize<int32_t>();
  /* const size_t sfxEntryCount = */ reader.readSize<int32_t>();
  /* const size_t subObjectCount = */ reader.readSize<int32_t>();

  const size_t skinOffset = reader.readSize<int32_t>();

  const MdxSkinList skins = parseSkins(reader.subReaderFromBegin(skinOffset), skinCount);

  auto model = std::make_unique<Assets::EntityModel>(
    m_name, Assets::PitchType::Normal, Assets::Orientation::Oriented);
  for (size_t i = 0; i < frameCount; ++i)
  {
    model->addFrame();
  }

  auto& surface = model->addSurface(m_name);
  loadSkins(surface, skins, logger);

  return model;
}

void MdxParser::doLoadFrame(
  size_t frameIndex, Assets::EntityModel& model, Logger& /* logger */)
{
  auto reader = m_reader;
  const auto ident = reader.readInt<int32_t>();
  const auto version = reader.readInt<int32_t>();

  if (ident != MdxLayout::Ident)
  {
    throw AssetException("Unknown MD2 model ident: " + std::to_string(ident));
  }
  if (version != MdxLayout::Version)
  {
    throw AssetException("Unknown MD2 model version: " + std::to_string(version));
  }

  /*const size_t skinWidth =*/reader.readSize<int32_t>();
  /*const size_t skinHeight =*/reader.readSize<int32_t>();
  /*const size_t frameSize =*/reader.readSize<int32_t>();

  /* const size_t skinCount = */ reader.readSize<int32_t>();
  const size_t vertexCount = reader.readSize<int32_t>();
  /* const size_t triangleCount =*/reader.readSize<int32_t>();
  const size_t commandCount = reader.readSize<int32_t>();
  /* const size_t frameCount = */ reader.readSize<int32_t>();

  /* const size_t sfxDefineCount = */ reader.readSize<int32_t>();
  /* const size_t sfxEntryCount = */ reader.readSize<int32_t>();
  /* const size_t subObjectCount = */ reader.readSize<int32_t>();

  /* const size_t skinOffset = */ reader.readSize<int32_t>();
  /* const auto triangleOffset =*/reader.readSize<int32_t>();
  const auto frameOffset = reader.readSize<int32_t>();
  const auto commandOffset = reader.readSize<int32_t>();

  const auto frameSize = 6 * sizeof(float) + MdxLayout::FrameNameLength + vertexCount * 4;
  const auto frame = parseFrame(
    reader.subReaderFromBegin(frameOffset + frameIndex * frameSize, frameSize),
    frameIndex,
    vertexCount);
  const auto meshes =
    parseMeshes(reader.subReaderFromBegin(commandOffset, commandCount * 4), commandCount);

  auto& surface = model.surface(0);
  buildFrame(model, surface, frameIndex, frame, meshes);
}

MdxParser::MdxSkinList MdxParser::parseSkins(Reader reader, const size_t skinCount)
{
  MdxSkinList skins;
  skins.reserve(skinCount);
  for (size_t i = 0; i < skinCount; ++i)
  {
    skins.emplace_back(reader.readString(MdxLayout::SkinNameLength));
  }
  return skins;
}

MdxParser::MdxFrame MdxParser::parseFrame(
  Reader reader, const size_t /* frameIndex */, const size_t vertexCount)
{
  auto frame = MdxFrame(vertexCount);
  frame.scale = reader.readVec<float, 3>();
  frame.offset = reader.readVec<float, 3>();
  frame.name = reader.readString(MdxLayout::FrameNameLength);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    frame.vertices[i].x = reader.readUnsignedChar<char>();
    frame.vertices[i].y = reader.readUnsignedChar<char>();
    frame.vertices[i].z = reader.readUnsignedChar<char>();
    frame.vertices[i].normalIndex = reader.readUnsignedChar<char>();
  }

  return frame;
}

MdxParser::MdxMeshList MdxParser::parseMeshes(
  Reader reader, const size_t /* commandCount */)
{
  MdxMeshList meshes;

  int32_t type = reader.readInt<int32_t>();
  while (type != 0)
  {
    MdxMesh mesh(type);
    /* const size_t subObjectId = */ reader.readSize<int32_t>();
    for (size_t i = 0; i < mesh.vertexCount; ++i)
    {
      mesh.vertices[i].texCoords[0] = reader.readFloat<float>();
      mesh.vertices[i].texCoords[1] = reader.readFloat<float>();
      mesh.vertices[i].vertexIndex = reader.readSize<int32_t>();
    }
    meshes.emplace_back(mesh);

    type = reader.readInt<int32_t>();
  }

  return meshes;
}

void MdxParser::loadSkins(
  Assets::EntityModelSurface& surface, const MdxSkinList& skins, Logger& logger)
{
  std::vector<Assets::Texture> textures;
  textures.reserve(skins.size());

  for (const auto& skin : skins)
  {
    auto path = Path(skin);
    if (path.isAbsolute())
    {
      path = path.makeRelative();
    }
    textures.push_back(loadSkin(path, m_fs, logger));
  }

  surface.setSkins(std::move(textures));
}

void MdxParser::buildFrame(
  Assets::EntityModel& model,
  Assets::EntityModelSurface& surface,
  const size_t frameIndex,
  const MdxFrame& frame,
  const MdxMeshList& meshes)
{
  size_t vertexCount = 0;
  Renderer::IndexRangeMap::Size size;
  for (const auto& md2Mesh : meshes)
  {
    vertexCount += md2Mesh.vertices.size();
    if (md2Mesh.type == MdxMesh::Fan)
    {
      size.inc(Renderer::PrimType::TriangleFan);
    }
    else
    {
      size.inc(Renderer::PrimType::TriangleStrip);
    }
  }

  vm::bbox3f::builder bounds;

  Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type> builder(
    vertexCount, size);
  for (const auto& md2Mesh : meshes)
  {
    if (!md2Mesh.vertices.empty())
    {
      vertexCount += md2Mesh.vertices.size();
      const auto vertices = getVertices(frame, md2Mesh.vertices);

      bounds.add(
        std::begin(vertices), std::end(vertices), Renderer::GetVertexComponent<0>());

      if (md2Mesh.type == MdxMesh::Fan)
      {
        builder.addTriangleFan(vertices);
      }
      else
      {
        builder.addTriangleStrip(vertices);
      }
    }
  }

  auto& modelFrame = model.loadFrame(frameIndex, frame.name, bounds.bounds());
  surface.addIndexedMesh(
    modelFrame, std::move(builder.vertices()), std::move(builder.indices()));
}

std::vector<Assets::EntityModelVertex> MdxParser::getVertices(
  const MdxFrame& frame, const MdxMeshVertexList& meshVertices) const
{
  std::vector<Assets::EntityModelVertex> result;
  result.reserve(meshVertices.size());

  for (const MdxMeshVertex& md2MeshVertex : meshVertices)
  {
    const auto position = frame.vertex(md2MeshVertex.vertexIndex);
    const auto& texCoords = md2MeshVertex.texCoords;

    result.emplace_back(position, texCoords);
  }

  return result;
}
} // namespace IO
} // namespace TrenchBroom
