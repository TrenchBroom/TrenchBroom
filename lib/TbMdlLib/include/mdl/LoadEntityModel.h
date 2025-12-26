/*
 Copyright (C) 2024 Kristian Duske

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
#include "mdl/EntityModelDataResource.h"

#include <filesystem>
#include <functional>

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
class EntityModel;
struct MaterialConfig;

using LoadMaterialFunc = std::function<gl::Material(const std::filesystem::path&)>;

Result<EntityModel> loadEntityModelSync(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger);

EntityModel loadEntityModelAsync(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  const CreateEntityModelDataResource& createResource,
  Logger& logger);

} // namespace mdl
} // namespace tb
