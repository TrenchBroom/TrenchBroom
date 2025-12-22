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

#include "LoadBspModel.h"

#include "fs/ReaderException.h"
#include "mdl/LoadMipTexture.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Material.h"
#include "mdl/Palette.h"
#include "render/MaterialIndexRangeMap.h"
#include "render/MaterialIndexRangeMapBuilder.h"

#include "kd/path_utils.h"

namespace tb::mdl
{
namespace
{
namespace BspLayout
{
constexpr size_t DirMaterialsAddress = 0x14;
constexpr size_t DirVerticesAddress = 0x1C;
constexpr size_t DirTexInfosAddress = 0x34;
constexpr size_t DirFacesAddress = 0x3C;
constexpr size_t DirEdgesAddress = 0x64;
constexpr size_t DirFaceEdgesAddress = 0x6C;
constexpr size_t DirModelAddress = 0x74;

constexpr size_t FaceSize = 0x14;
constexpr size_t FaceEdgeIndex = 0x4;
constexpr size_t FaceRest = 0x8;

constexpr size_t MaterialInfoSize = 0x28;
constexpr size_t MaterialInfoRest = 0x4;

constexpr size_t FaceEdgeSize = 0x4;
constexpr size_t ModelSize = 0x40;
constexpr size_t ModelFaceIndex = 0x38;
} // namespace BspLayout

bool isBSPVersionSupported(const int version)
{
  // 29 is Quake, 30 is GoldSrc.
  return version == 29 || version == 30;
}

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


std::vector<Material> parseMaterials(
  fs::Reader reader,
  const int version,
  const Palette& palette,
  const fs::FileSystem& fs,
  Logger& logger)
{
  const auto materialCount = reader.readSize<int32_t>();
  auto result = std::vector<Material>{};
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
      (version == 29 ? loadIdMipTexture(textureReader, palette, mask)
                     : loadHlMipTexture(textureReader, mask))
      | kdl::or_else(makeReadTextureErrorHandler(fs, logger))
      | kdl::transform([&](auto texture) {
          auto textureResource = createTextureResource(std::move(texture));
          return Material{std::move(materialName), std::move(textureResource)};
        })
      | kdl::value());
  }

  return result;
}

std::vector<MaterialInfo> parseMaterialInfos(fs::Reader reader, const size_t count)
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

std::vector<vm::vec3f> parseVertices(fs::Reader reader, const size_t vertexCount)
{
  auto result = std::vector<vm::vec3f>(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i)
  {
    result[i] = reader.readVec<float, 3>();
  }
  return result;
}

std::vector<EdgeInfo> parseEdgeInfos(fs::Reader reader, const size_t edgeInfoCount)
{
  auto result = std::vector<EdgeInfo>(edgeInfoCount);
  for (size_t i = 0; i < edgeInfoCount; ++i)
  {
    result[i].vertexIndex1 = reader.readSize<uint16_t>();
    result[i].vertexIndex2 = reader.readSize<uint16_t>();
  }
  return result;
}

std::vector<FaceInfo> parseFaceInfos(fs::Reader reader, const size_t faceInfoCount)
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

std::vector<int> parseFaceEdges(fs::Reader reader, const size_t faceEdgeCount)
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
  const Material* material)
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
  fs::Reader reader,
  const size_t frameIndex,
  EntityModelData& modelData,
  const std::vector<MaterialInfo>& materialInfos,
  const std::vector<vm::vec3f>& vertices,
  const std::vector<EdgeInfo>& edgeInfos,
  const std::vector<FaceInfo>& faceInfos,
  const std::vector<int>& faceEdges)
{
  using Vertex = EntityModelVertex;

  auto& surface = modelData.surface(0);

  reader.seekForward(BspLayout::ModelFaceIndex);
  const auto modelFaceIndex = reader.readSize<int32_t>();
  const auto modelFaceCount = reader.readSize<int32_t>();
  auto totalVertexCount = size_t(0);
  auto size = render::MaterialIndexRangeMap::Size{};

  for (size_t i = 0; i < modelFaceCount; ++i)
  {
    const auto& faceInfo = faceInfos[modelFaceIndex + i];
    const auto& materialInfo = materialInfos[faceInfo.materialInfoIndex];
    if (const auto* skin = surface.skin(materialInfo.materialIndex))
    {
      const auto faceVertexCount = faceInfo.edgeCount;
      size.inc(skin, render::PrimType::Polygon, faceVertexCount);
      totalVertexCount += faceVertexCount;
    }
  }

  auto bounds = vm::bbox3f::builder{};

  auto builder =
    render::MaterialIndexRangeMapBuilder<Vertex::Type>{totalVertexCount, size};
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

bool canLoadBspModel(const std::filesystem::path& path, fs::Reader reader)
{
  if (!kdl::path_has_extension(kdl::path_to_lower(path), ".bsp"))
  {
    return false;
  }

  const auto version = reader.readInt<int32_t>();
  return isBSPVersionSupported(version);
}

Result<EntityModelData> loadBspModel(
  const std::string& name,
  fs::Reader reader,
  const Palette& palette,
  const fs::FileSystem& fs,
  Logger& logger)
{
  try
  {
    const auto version = reader.readInt<int32_t>();
    if (!isBSPVersionSupported(version))
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

    auto data = EntityModelData{PitchType::Normal, Orientation::Oriented};

    auto materials = parseMaterials(
      reader.subReaderFromBegin(materialsOffset), version, palette, fs, logger);

    auto& surface = data.addSurface(name, frameCount);
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

    return data;
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
