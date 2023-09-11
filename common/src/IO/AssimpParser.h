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
class EntityModelSurface;
class Texture;
} // namespace Assets

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
  void loadSceneFrame(
    const aiScene* scene,
    Assets::EntityModelSurface& surface,
    Assets::EntityModel& model);
  void processNode(
    const aiNode& node,
    const aiScene& scene,
    const aiMatrix4x4& transform,
    const aiMatrix4x4& axisTransform);
  void processMesh(
    const aiMesh& mesh, const aiMatrix4x4& transform, const aiMatrix4x4& axisTransform);
  Assets::Texture processMaterial(
    const aiScene& scene, size_t materialIndex, Logger& logger) const;
  static aiMatrix4x4 get_axis_transform(const aiScene& scene);
};
} // namespace IO
} // namespace TrenchBroom
