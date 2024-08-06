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

#pragma once

#include "Assets/TextureResource.h"
#include "Result.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;
} // namespace TrenchBroom

namespace TrenchBroom::Assets
{
class Palette;
class Quake3Shader;
class Material;
class MaterialCollection;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
struct MaterialConfig;
}

namespace TrenchBroom::IO
{
class FileSystem;

Result<Assets::Material> loadMaterial(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const std::filesystem::path& materialPath,
  const Assets::CreateTextureResource& createResource,
  const std::vector<Assets::Quake3Shader>& shaders,
  const std::optional<Result<Assets::Palette>>& paletteResult);

Result<std::vector<Assets::MaterialCollection>> loadMaterialCollections(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const Assets::CreateTextureResource& createResource,
  Logger& logger);

} // namespace TrenchBroom::IO
