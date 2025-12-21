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

#include "LoadEntityModel.h"

#include "Result.h"
#include "fs/FileSystem.h"
#include "io/DkmLoader.h"
#include "io/ImageSpriteLoader.h"
#include "io/LoadAseModel.h"
#include "io/LoadAssimpModel.h"
#include "io/LoadBspModel.h"
#include "io/LoadMd2Model.h"
#include "io/LoadMd3Model.h"
#include "io/LoadMdlModel.h"
#include "io/LoadSpriteModel.h"
#include "io/MdxLoader.h"
#include "mdl/EntityModel.h"
#include "mdl/GameConfig.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <fmt/format.h>
#include <fmt/std.h>

namespace tb::io
{

namespace
{

auto loadPalette(const fs::FileSystem& fs, const mdl::MaterialConfig& materialConfig)
{
  const auto& path = materialConfig.palette;
  return fs.openFile(path)
         | kdl::and_then([&](auto file) { return mdl::loadPalette(*file, path); });
}

Result<mdl::EntityModelData> loadEntityModelData(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  const auto modelName = path.filename().string();
  return fs.openFile(path)
         | kdl::and_then([&](auto file) -> Result<mdl::EntityModelData> {
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
             if (MdxLoader::canParse(path, reader))
             {
               auto loader = MdxLoader{modelName, reader, fs};
               return loader.load(logger);
             }
             if (DkmLoader::canParse(path, reader))
             {
               auto loader = DkmLoader{modelName, reader, fs};
               return loader.load(logger);
             }
             if (canLoadAseModel(path))
             {
               return loadAseModel(modelName, reader.stringView(), loadMaterial, logger);
             }
             if (ImageSpriteLoader::canParse(path))
             {
               auto loader = ImageSpriteLoader{modelName, file, fs};
               return loader.load(logger);
             }
             if (canLoadAssimpModel(path))
             {
               return loadAssimpModel(path, fs, logger);
             }
             return Error{fmt::format("Unknown model format: {}", path)};
           })
         | kdl::or_else([&](const auto& e) {
             return Result<mdl::EntityModelData>{Error{
               fmt::format("Failed to load entity model '{}': {}", modelName, e.msg)}};
           });
}

mdl::ResourceLoader<mdl::EntityModelData> makeEntityModelDataResourceLoader(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return [&fs, materialConfig, path, loadMaterial, &logger]() {
    return loadEntityModelData(fs, materialConfig, path, loadMaterial, logger);
  };
}

} // namespace

Result<mdl::EntityModel> loadEntityModelSync(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return loadEntityModelData(fs, materialConfig, path, loadMaterial, logger)
         | kdl::transform([&](auto modelData) {
             auto modelName = path.filename().string();
             auto modelResource =
               mdl::createEntityModelDataResource(std::move(modelData));
             return mdl::EntityModel{std::move(modelName), std::move(modelResource)};
           });
}

mdl::EntityModel loadEntityModelAsync(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  const mdl::CreateEntityModelDataResource& createResource,
  Logger& logger)
{
  auto name = path.filename().string();
  auto loader =
    makeEntityModelDataResourceLoader(fs, materialConfig, path, loadMaterial, logger);
  auto resource = createResource(std::move(loader));
  return mdl::EntityModel{std::move(name), std::move(resource)};
}

} // namespace tb::io
