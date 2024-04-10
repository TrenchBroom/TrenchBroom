/*
 Copyright (C) 2023 Daniel Walder
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

struct aiNode;
struct aiScene;
struct aiMesh;

namespace TrenchBroom::Assets
{
class Texture;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::IO
{
class FileSystem;

struct AssimpMeshWithTransforms
{
  const aiMesh* m_mesh;
  aiMatrix4x4 m_transform;
  aiMatrix4x4 m_axisTransform;
};

class AssimpParser : public EntityModelParser
{
private:
  std::filesystem::path m_path;
  const FileSystem& m_fs;

public:
  AssimpParser(std::filesystem::path path, const FileSystem& fs);

  static bool canParse(const std::filesystem::path& path);

  void loadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;
  std::unique_ptr<Assets::EntityModel> initializeModel(Logger& logger) override;
};

} // namespace TrenchBroom::IO
