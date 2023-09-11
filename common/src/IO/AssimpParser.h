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

#include "Assets/EntityModel_Forward.h"
#include "IO/EntityModelParser.h"

#include <assimp/matrix4x4.h>

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
} // namespace Assets

namespace IO
{
class FileSystem;

struct AssimpMeshWithTransforms
{
  const aiMesh* m_mesh;
  aiMatrix4x4 m_transform;
  aiMatrix4x4 m_axisTransform;

  AssimpMeshWithTransforms(
    const aiMesh* mesh, const aiMatrix4x4& transform, const aiMatrix4x4& axisTransform)
    : m_mesh(mesh)
    , m_transform(transform)
    , m_axisTransform(axisTransform)
  {
  }
};

class AssimpParser : public EntityModelParser
{
private:
  std::filesystem::path m_path;
  const FileSystem& m_fs;

public:
  AssimpParser(std::filesystem::path path, const FileSystem& fs);

  static bool canParse(const std::filesystem::path& path);

private:
  std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
  void loadSceneFrame(const aiScene* scene, Assets::EntityModel& model) const;
  static void processNode(
    std::vector<AssimpMeshWithTransforms>& meshes,
    const aiNode& node,
    const aiScene& scene,
    const aiMatrix4x4& transform,
    const aiMatrix4x4& axisTransform);
  static std::vector<Assets::EntityModelVertex> computeMeshVertices(
    const aiMesh& mesh, const aiMatrix4x4& transform, const aiMatrix4x4& axisTransform);
  Assets::Texture processMaterial(
    const aiScene& scene, size_t materialIndex, Logger& logger) const;
  static aiMatrix4x4 getAxisTransform(const aiScene& scene);
};
} // namespace IO
} // namespace TrenchBroom
