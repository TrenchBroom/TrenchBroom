/*
 Copyright (C) 2010 Kristian Duske

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

#include "Result.h"
#include "gl/TextureResource.h"
#include "mdl/Palette.h"
#include "mdl/Quake3Shader.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
class Logger;

namespace gl
{
class Material;
}

namespace fs
{
class FileSystem;
} // namespace fs

namespace mdl
{
class MaterialCollection;
struct MaterialConfig;

Result<gl::Material> loadMaterial(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& materialPath,
  const gl::CreateTextureResource& createResource,
  const std::vector<Quake3Shader>& shaders,
  const std::optional<Palette>& palette);

Result<std::vector<MaterialCollection>> loadMaterialCollections(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const gl::CreateTextureResource& createResource,
  kdl::task_manager& taskManager,
  Logger& logger);

} // namespace mdl
} // namespace tb
