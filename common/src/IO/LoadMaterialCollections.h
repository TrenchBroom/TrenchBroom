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
#include "asset/Palette.h"
#include "asset/Quake3Shader.h"
#include "asset/TextureResource.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace tb
{
class Logger;
} // namespace tb

namespace tb::asset
{
class Material;
class MaterialCollection;
} // namespace tb::asset

namespace tb::Model
{
struct MaterialConfig;
}

namespace tb::IO
{
class FileSystem;

Result<asset::Material> loadMaterial(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const std::filesystem::path& materialPath,
  const asset::CreateTextureResource& createResource,
  const std::vector<asset::Quake3Shader>& shaders,
  const std::optional<Result<asset::Palette>>& paletteResult);

Result<std::vector<asset::MaterialCollection>> loadMaterialCollections(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const asset::CreateTextureResource& createResource,
  Logger& logger);

} // namespace tb::IO
