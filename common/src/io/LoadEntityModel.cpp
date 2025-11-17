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
#include "io/AseLoader.h"
#include "io/AssimpLoader.h"
#include "io/BspLoader.h"
#include "io/DkmLoader.h"
#include "io/FileSystem.h"
#include "io/ImageSpriteLoader.h"
#include "io/Md2Loader.h"
#include "io/Md3Loader.h"
#include "io/MdlLoader.h"
#include "io/MdxLoader.h"
#include "io/SprLoader.h"
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

auto loadPalette(const FileSystem& fs, const mdl::MaterialConfig& materialConfig)
{
  const auto& path = materialConfig.palette;
  return fs.openFile(path)
         | kdl::and_then([&](auto file) { return mdl::loadPalette(*file, path); });
}

Result<mdl::EntityModelData> loadEntityModelData(
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  const auto modelName = path.filename().string();
  return fs.openFile(path)
         | kdl::and_then([&](auto file) -> Result<mdl::EntityModelData> {
             auto reader = file->reader().buffer();

             if (io::MdlLoader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = io::MdlLoader{modelName, reader, palette};
                        return loader.load(logger);
                      });
             }
             if (io::Md2Loader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = io::Md2Loader{modelName, reader, palette, fs};
                        return loader.load(logger);
                      });
             }
             if (io::BspLoader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = io::BspLoader{modelName, reader, palette, fs};
                        return loader.load(logger);
                      });
             }
             if (io::SprLoader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = io::SprLoader{modelName, reader, palette};
                        return loader.load(logger);
                      });
             }
             if (io::Md3Loader::canParse(path, reader))
             {
               auto loader = io::Md3Loader{modelName, reader, loadMaterial};
               return loader.load(logger);
             }
             if (io::MdxLoader::canParse(path, reader))
             {
               auto loader = io::MdxLoader{modelName, reader, fs};
               return loader.load(logger);
             }
             if (io::DkmLoader::canParse(path, reader))
             {
               auto loader = io::DkmLoader{modelName, reader, fs};
               return loader.load(logger);
             }
             if (io::AseLoader::canParse(path))
             {
               auto loader = io::AseLoader{modelName, reader.stringView(), loadMaterial};
               return loader.load(logger);
             }
             if (io::ImageSpriteLoader::canParse(path))
             {
               auto loader = io::ImageSpriteLoader{modelName, file, fs};
               return loader.load(logger);
             }
             if (io::AssimpLoader::canParse(path))
             {
               auto loader = io::AssimpLoader{path, fs};
               return loader.load(logger);
             }
             return Error{fmt::format("Unknown model format: {}", path)};
           })
         | kdl::or_else([&](const auto& e) {
             return Result<mdl::EntityModelData>{Error{
               fmt::format("Failed to load entity model '{}': {}", modelName, e.msg)}};
           });
}

mdl::ResourceLoader<mdl::EntityModelData> makeEntityModelDataResourceLoader(
  const FileSystem& fs,
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
  const FileSystem& fs,
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
  const FileSystem& fs,
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
