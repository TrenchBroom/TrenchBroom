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

#include "mdl/LoadEntityModel.h"

#include "Result.h"
#include "fs/FileSystem.h"
#include "gl/Resource.h"
#include "mdl/EntityModel.h"
#include "mdl/GameConfig.h"
#include "mdl/LoadAseModel.h"
#include "mdl/LoadAssimpModel.h"
#include "mdl/LoadBspModel.h"
#include "mdl/LoadDkmModel.h"
#include "mdl/LoadImageSpriteModel.h"
#include "mdl/LoadMd2Model.h"
#include "mdl/LoadSiNModel.h"
#include "mdl/LoadMd3Model.h"
#include "mdl/LoadMdlModel.h"
#include "mdl/LoadMdxModel.h"
#include "mdl/LoadSpriteModel.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <fmt/format.h>
#include <fmt/std.h>

namespace tb::mdl
{

namespace
{

auto loadPalette(const fs::FileSystem& fs, const MaterialConfig& materialConfig)
{
  const auto& path = materialConfig.palette;
  return fs.openFile(path)
         | kdl::and_then([&](auto file) { return mdl::loadPalette(*file, path); });
}

Result<EntityModelData> loadEntityModelData(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  const auto modelName = path.filename().string();

  // SiN specific
  const auto modelPath = path;

  auto file = fs.openFile(path);

  if (!file)
      file = fs.openFile("models/" + path.string());

  return file | kdl::and_then([&](auto file) -> Result<EntityModelData> {
           auto reader = file->reader().buffer();

           if (canLoadMdlModel(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      return loadMdlModel(modelName, reader, palette, logger);
                    });
           }
           if (canLoadMd2Model(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      return loadMd2Model(modelName, reader, palette, fs, logger);
                    });
           }
           if (canLoadSiNModel(path, reader))
           {
             return loadSiNModel(modelName, reader, fs, logger);
           }
           if (canLoadBspModel(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      return loadBspModel(modelName, reader, palette, fs, logger);
                    });
           }
           if (canLoadSpriteModel(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      return loadSpriteModel(modelName, reader, palette, logger);
                    });
           }
           if (canLoadMd3Model(path, reader))
           {
             return loadMd3Model(reader, loadMaterial, logger);
           }
           if (canLoadMdxModel(path, reader))
           {
             return loadMdxModel(modelName, reader, fs, logger);
           }
           if (canLoadDkmModel(path, reader))
           {
             return loadDkmModel(modelName, reader, fs, logger);
           }
           if (canLoadAseModel(path))
           {
             return loadAseModel(modelName, reader.stringView(), loadMaterial, logger);
           }
           if (canLoadImageSpriteModel(path))
           {
             return loadImageSpriteModel(modelName, reader, fs, logger);
           }
           if (canLoadAssimpModel(path))
           {
             return loadAssimpModel(path, fs, logger);
           }
           return Error{fmt::format("Unknown model format: {}", path)};
         })
         | kdl::or_else([&](const auto& e) {
             return Result<EntityModelData>{Error{
               fmt::format("Failed to load entity model '{}': {}", modelName, e.msg)}};
           });
}

gl::ResourceLoader<EntityModelData> makeEntityModelDataResourceLoader(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return [&fs, materialConfig, path, loadMaterial, &logger]() {
    return loadEntityModelData(fs, materialConfig, path, loadMaterial, logger);
  };
}

} // namespace

Result<EntityModel> loadEntityModelSync(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return loadEntityModelData(fs, materialConfig, path, loadMaterial, logger)
         | kdl::transform([&](auto modelData) {
             auto modelName = path.filename().string();
             auto modelResource = createEntityModelDataResource(std::move(modelData));
             return EntityModel{std::move(modelName), std::move(modelResource)};
           });
}

EntityModel loadEntityModelAsync(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  const CreateEntityModelDataResource& createResource,
  Logger& logger)
{
  auto name = path.filename().string();
  auto loader =
    makeEntityModelDataResourceLoader(fs, materialConfig, path, loadMaterial, logger);
  auto resource = createResource(std::move(loader));
  return EntityModel{std::move(name), std::move(resource)};
}

} // namespace tb::mdl
