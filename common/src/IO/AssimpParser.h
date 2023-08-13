/*
 Copyright (C) 2022 Amara M. Kilic
 Copyright (C) 2022 Kristian Duske

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

#pragma once

#include "IO/EntityModelParser.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <assimp/types.h>

#include <filesystem>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace TrenchBroom
{
namespace Assets
{
class Texture;
}

namespace IO
{
class FileSystem;

struct AssimpFace
{
  size_t m_material;
  std::vector<size_t> m_vertices;
  AssimpFace(size_t material, std::vector<size_t> vertices)
    : m_material{material}
    , m_vertices{std::move(vertices)}
  {
  }
};

struct AssimpVertex
{
  size_t m_position;
  vm::vec2f m_texcoords;
  AssimpVertex(size_t position, const vm::vec2f& texcoords)
    : m_position{position}
    , m_texcoords{texcoords}
  {
  }
};

struct AssimpBoneInformation
{
  size_t m_boneIndex{0};
  int32_t m_parentIndex{-1};
  aiString m_name;
  aiMatrix4x4 m_localTransform;
  aiMatrix4x4 m_globalTransform;

  AssimpBoneInformation() = default;

  AssimpBoneInformation(
    size_t boneIndex,
    int32_t parentIndex,
    const aiString& name,
    const aiMatrix4x4& localTransform,
    const aiMatrix4x4& globalTransform)
    : m_boneIndex(boneIndex)
    , m_parentIndex(parentIndex)
    , m_name(name)
    , m_localTransform(localTransform)
    , m_globalTransform(globalTransform)
  {
  }
};

class AssimpParser : public EntityModelParser
{
private:
  std::filesystem::path m_path;
  const FileSystem& m_fs;

  std::vector<vm::vec3f> m_positions;
  std::vector<AssimpVertex> m_vertices;
  std::vector<AssimpFace> m_faces;
  std::vector<Assets::Texture> m_textures;

public:
  AssimpParser(std::filesystem::path path, const FileSystem& fs);

  static bool canParse(const std::filesystem::path& path);

private:
  std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
  void doLoadFrame(
    size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;
  void loadSceneFrame(
    const aiScene* scene, const size_t frameIndex, Assets::EntityModel& model);
  void processRootNode(
    const aiNode& node,
    const aiScene& scene,
    const aiMatrix4x4& transform,
    const aiMatrix4x4& axisTransform,
    const std::vector<AssimpBoneInformation>& boneTransforms);
  void processNode(
    const aiNode& node,
    const aiScene& scene,
    const aiMatrix4x4& transform,
    const aiMatrix4x4& axisTransform,
    const std::vector<AssimpBoneInformation>& boneTransforms);
  void processMesh(
    const aiMesh& mesh,
    const aiMatrix4x4& transform,
    const aiMatrix4x4& axisTransform,
    const std::vector<AssimpBoneInformation>& boneTransforms);
  void processMaterials(const aiScene& scene, Logger& logger);
  static aiMatrix4x4 get_axis_transform(const aiScene& scene);
};
} // namespace IO
} // namespace TrenchBroom
