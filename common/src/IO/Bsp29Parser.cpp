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

#include "Bsp29Parser.h"

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Assets/TextureResource.h"
#include "Error.h"
#include "IO/File.h"
#include "IO/MaterialUtils.h"
#include "IO/ReadMipTexture.h"
#include "IO/Reader.h"
#include "IO/ReaderException.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "Renderer/MaterialIndexRangeMap.h"
#include "Renderer/MaterialIndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/result.h"
#include "kdl/string_format.h"

#include <fmt/format.h>

#include <string>
#include <vector>

namespace TrenchBroom::IO
{
namespace BspLayout
{
static const size_t DirMaterialsAddress = 0x14;
static const size_t DirVerticesAddress = 0x1C;
static const size_t DirTexInfosAddress = 0x34;
static const size_t DirFacesAddress = 0x3C;
static const size_t DirEdgesAddress = 0x64;
static const size_t DirFaceEdgesAddress = 0x6C;
static const size_t DirModelAddress = 0x74;

static const size_t FaceSize = 0x14;
static const size_t FaceEdgeIndex = 0x4;
static const size_t FaceRest = 0x8;

static const size_t MaterialInfoSize = 0x28;
static const size_t MaterialInfoRest = 0x4;

static const size_t FaceEdgeSize = 0x4;
static const size_t ModelSize = 0x40;
static const size_t ModelFaceIndex = 0x38;
} // namespace BspLayout

namespace
{

struct MaterialInfo
{
  vm::vec3f uAxis;
  vm::vec3f vAxis;
  float uOffset;
  float vOffset;
  size_t materialIndex;
};

struct EdgeInfo
{
  size_t vertexIndex1, vertexIndex2;
};

struct FaceInfo
{
  size_t edgeIndex;
  size_t edgeCount;
  size_t materialInfoIndex;
};


std::vector<Assets::Material> parseMaterials(
  Reader reader, const Assets::Palette& palette, const FileSystem& fs, Logger& logger)
{
  const auto materialCount = reader.readSize<int32_t>();
  auto result = std::vector<Assets::Material>{};
  result.reserve(materialCount);

  for (size_t i = 0; i < materialCount; ++i)
  {
    const auto offset = reader.readInt<int32_t>();
    // 2153: Some BSPs contain negative offsets.
    if (offset < 0)
    {
      result.push_back(loadDefaultMaterial(fs, "unknown", logger));
      continue;
    }

    auto materialName = readMipTextureName(reader);
    auto textureReader = reader.subReaderFromBegin(size_t(offset)).buffer();
    const auto mask = getTextureMaskFromName(materialName);

    result.push_back(
      readIdMipTexture(textureReader, palette, mask)
      | kdl::or_else(makeReadTextureErrorHandler(fs, logger))
      | kdl::transform([&](auto texture) {
          auto textureResource = createTextureResource(std::move(texture));
          return Assets::Material{std::move(materialName), std::move(textureResource)};
        })
      | kdl::value());
  }

  return result;
}

std::vector<MaterialInfo> parseMaterialInfos(Reader reader, const size_t count)
{
  auto result = std::vector<MaterialInfo>(count);
  for (size_t i = 0; i < count; ++i)
  {
    result[i].uAxis = reader.readVec<float, 3>();
    result[i].uOffset = reader.readFloat<float>();
    result[i].vAxis = reader.readVec<float, 3>();
    result[i].vOffset = reader.readFloat<float>();
    result[i].materialIndex = reader.readSize<uint32_t>();
    reader.seekForward(BspLayout::MaterialInfoRest);
  }
  return result;
}

std::vector<vm::vec3f> parseVertices(Reader reader, const size_t vertexCount)
{
  auto result = std::vector<vm::vec3f>(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i)
  {
    result[i] = reader.readVec<float, 3>();
  }
  return result;
}

std::vector<EdgeInfo> parseEdgeInfos(Reader reader, const size_t edgeInfoCount)
{
  auto result = std::vector<EdgeInfo>(edgeInfoCount);
  for (size_t i = 0; i < edgeInfoCount; ++i)
  {
    result[i].vertexIndex1 = reader.readSize<uint16_t>();
    result[i].vertexIndex2 = reader.readSize<uint16_t>();
  }
  return result;
}

std::vector<FaceInfo> parseFaceInfos(Reader reader, const size_t faceInfoCount)
{
  auto result = std::vector<FaceInfo>(faceInfoCount);
  for (size_t i = 0; i < faceInfoCount; ++i)
  {
    reader.seekForward(BspLayout::FaceEdgeIndex);
    result[i].edgeIndex = reader.readSize<int32_t>();
    result[i].edgeCount = reader.readSize<uint16_t>();
    result[i].materialInfoIndex = reader.readSize<uint16_t>();
    reader.seekForward(BspLayout::FaceRest);
  }
  return result;
}

std::vector<int> parseFaceEdges(Reader reader, const size_t faceEdgeCount)
{
  auto result = std::vector<int>(faceEdgeCount);
  for (size_t i = 0; i < faceEdgeCount; ++i)
  {
    result[i] = reader.readInt<int32_t>();
  }
  return result;
}

vm::vec2f uvCoords(
  const vm::vec3f& vertex,
  const MaterialInfo& materialInfo,
  const Assets::Material* material)
{
  if (const auto* texture = getTexture(material))
  {
    const auto textureSize = texture->sizef();
    return vm::vec2f{
      (vm::dot(vertex, materialInfo.uAxis) + materialInfo.uOffset) / textureSize.x(),
      (vm::dot(vertex, materialInfo.vAxis) + materialInfo.vOffset) / textureSize.y(),
    };
  }

  return vm::vec2f{0, 0};
}

void parseFrame(
  Reader reader,
  const size_t frameIndex,
  Assets::EntityModelData& modelData,
  const std::vector<MaterialInfo>& materialInfos,
  const std::vector<vm::vec3f>& vertices,
  const std::vector<EdgeInfo>& edgeInfos,
  const std::vector<FaceInfo>& faceInfos,
  const std::vector<int>& faceEdges)
{
  using Vertex = Assets::EntityModelVertex;

  auto& surface = modelData.surface(0);

  reader.seekForward(BspLayout::ModelFaceIndex);
  const auto modelFaceIndex = reader.readSize<int32_t>();
  const auto modelFaceCount = reader.readSize<int32_t>();
  auto totalVertexCount = size_t(0);
  auto size = Renderer::MaterialIndexRangeMap::Size{};

  for (size_t i = 0; i < modelFaceCount; ++i)
  {
    const auto& faceInfo = faceInfos[modelFaceIndex + i];
    const auto& materialInfo = materialInfos[faceInfo.materialInfoIndex];
    if (const auto* skin = surface.skin(materialInfo.materialIndex))
    {
      const auto faceVertexCount = faceInfo.edgeCount;
      size.inc(skin, Renderer::PrimType::Polygon, faceVertexCount);
      totalVertexCount += faceVertexCount;
    }
  }

  auto bounds = vm::bbox3f::builder{};

  auto builder =
    Renderer::MaterialIndexRangeMapBuilder<Vertex::Type>{totalVertexCount, size};
  for (size_t i = 0; i < modelFaceCount; ++i)
  {
    const auto& faceInfo = faceInfos[modelFaceIndex + i];
    const auto& materialInfo = materialInfos[faceInfo.materialInfoIndex];
    if (const auto* skin = surface.skin(materialInfo.materialIndex))
    {
      const auto faceVertexCount = faceInfo.edgeCount;

      auto faceVertices = std::vector<Vertex>{};
      faceVertices.reserve(faceVertexCount);

      for (size_t k = 0; k < faceVertexCount; ++k)
      {
        const auto faceEdgeIndex = faceEdges[faceInfo.edgeIndex + k];
        const auto vertexIndex = faceEdgeIndex < 0
                                   ? edgeInfos[size_t(-faceEdgeIndex)].vertexIndex2
                                   : edgeInfos[size_t(faceEdgeIndex)].vertexIndex1;

        const auto& position = vertices[vertexIndex];
        const auto uvCoordss = uvCoords(position, materialInfo, skin);

        bounds.add(position);

        faceVertices.emplace_back(position, uvCoordss);
      }

      builder.addPolygon(skin, faceVertices);
    }
  }

  auto frameName = fmt::format("frame_{}", frameIndex);
  auto& frame = modelData.addFrame(std::move(frameName), bounds.bounds());
  surface.addMesh(frame, std::move(builder.vertices()), std::move(builder.indices()));
}

} // namespace

Bsp29Parser::Bsp29Parser(
  std::string name, const Reader& reader, Assets::Palette palette, const FileSystem& fs)
  : m_name{std::move(name)}
  , m_reader{reader}
  , m_palette{std::move(palette)}
  , m_fs{fs}
{
}

bool Bsp29Parser::canParse(const std::filesystem::path& path, Reader reader)
{
  if (kdl::str_to_lower(path.extension().string()) != ".bsp")
  {
    return false;
  }

  const auto version = reader.readInt<int32_t>();
  return version == 29;
}

Result<Assets::EntityModel> Bsp29Parser::initializeModel(Logger& logger)
{
  try
  {
    auto reader = m_reader;
    const auto version = reader.readInt<int32_t>();
    if (version != 29)
    {
      return Error{"Unsupported BSP model version: " + std::to_string(version)};
    }

    reader.seekFromBegin(BspLayout::DirModelAddress);
    const auto modelsOffset = reader.readSize<int32_t>();
    const auto modelsLength = reader.readSize<int32_t>();
    const auto frameCount = modelsLength / BspLayout::ModelSize;

    reader.seekFromBegin(BspLayout::DirTexInfosAddress);
    const auto materialInfoOffset = reader.readSize<int32_t>();
    const auto materialInfoLength = reader.readSize<int32_t>();
    const auto materialInfoCount = materialInfoLength / BspLayout::MaterialInfoSize;

    reader.seekFromBegin(BspLayout::DirVerticesAddress);
    const auto vertexOffset = reader.readSize<int32_t>();
    const auto vertexLength = reader.readSize<int32_t>();
    const auto vertexCount = vertexLength / (3 * sizeof(float));

    reader.seekFromBegin(BspLayout::DirEdgesAddress);
    const auto edgeInfoOffset = reader.readSize<int32_t>();
    const auto edgeInfoLength = reader.readSize<int32_t>();
    const auto edgeInfoCount = edgeInfoLength / (2 * sizeof(uint16_t));

    reader.seekFromBegin(BspLayout::DirFacesAddress);
    const auto faceInfoOffset = reader.readSize<int32_t>();
    const auto faceInfoLength = reader.readSize<int32_t>();
    const auto faceInfoCount = faceInfoLength / BspLayout::FaceSize;

    reader.seekFromBegin(BspLayout::DirFaceEdgesAddress);
    const auto faceEdgesOffset = reader.readSize<int32_t>();
    const auto faceEdgesLength = reader.readSize<int32_t>();
    const auto faceEdgesCount = faceEdgesLength / BspLayout::FaceEdgeSize;

    reader.seekFromBegin(BspLayout::DirMaterialsAddress);
    const auto materialsOffset = reader.readSize<int32_t>();

    auto data =
      Assets::EntityModelData{Assets::PitchType::Normal, Assets::Orientation::Oriented};

    auto materials =
      parseMaterials(reader.subReaderFromBegin(materialsOffset), m_palette, m_fs, logger);

    auto& surface = data.addSurface(m_name, frameCount);
    surface.setSkins(std::move(materials));

    const auto materialInfos = parseMaterialInfos(
      reader.subReaderFromBegin(materialInfoOffset), materialInfoCount);
    const auto vertices =
      parseVertices(reader.subReaderFromBegin(vertexOffset), vertexCount);
    const auto edgeInfos =
      parseEdgeInfos(reader.subReaderFromBegin(edgeInfoOffset), edgeInfoCount);
    const auto faceInfos =
      parseFaceInfos(reader.subReaderFromBegin(faceInfoOffset), faceInfoCount);
    const auto faceEdges =
      parseFaceEdges(reader.subReaderFromBegin(faceEdgesOffset), faceEdgesCount);


    for (size_t i = 0; i < frameCount; ++i)
    {
      parseFrame(
        reader.subReaderFromBegin(
          modelsOffset + i * BspLayout::ModelSize, BspLayout::ModelSize),
        i,
        data,
        materialInfos,
        vertices,
        edgeInfos,
        faceInfos,
        faceEdges);
    }

    return Assets::EntityModel{m_name, std::move(data)};
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace TrenchBroom::IO
