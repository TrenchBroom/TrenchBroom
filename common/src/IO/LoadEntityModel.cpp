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

#include "Assets/EntityModel.h"
#include "Assets/Palette.h"
#include "Error.h"
#include "IO/AseLoader.h"
#include "IO/AssimpLoader.h"
#include "IO/BspLoader.h"
#include "IO/DkmLoader.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/ImageSpriteLoader.h"
#include "IO/Md2Loader.h"
#include "IO/Md3Loader.h"
#include "IO/MdlLoader.h"
#include "IO/MdxLoader.h"
#include "IO/SprLoader.h"
#include "Model/GameConfig.h"
#include "Result.h"

#include <kdl/result.h>

namespace TrenchBroom::IO
{

namespace
{

auto loadPalette(const FileSystem& fs, const Model::MaterialConfig& materialConfig)
{
  const auto& path = materialConfig.palette;
  return fs.openFile(path)
         | kdl::and_then([&](auto file) { return Assets::loadPalette(*file, path); });
}

Result<Assets::EntityModelData> loadEntityModelData(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return fs.openFile(path)
         | kdl::and_then([&](auto file) -> Result<Assets::EntityModelData> {
             const auto modelName = path.filename().string();
             auto reader = file->reader().buffer();

             if (IO::MdlLoader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = IO::MdlLoader{modelName, reader, palette};
                        return loader.load(logger);
                      });
             }
             if (IO::Md2Loader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = IO::Md2Loader{modelName, reader, palette, fs};
                        return loader.load(logger);
                      });
             }
             if (IO::BspLoader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = IO::BspLoader{modelName, reader, palette, fs};
                        return loader.load(logger);
                      });
             }
             if (IO::SprLoader::canParse(path, reader))
             {
               return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                        auto loader = IO::SprLoader{modelName, reader, palette};
                        return loader.load(logger);
                      });
             }
             if (IO::Md3Loader::canParse(path, reader))
             {
               auto loader = IO::Md3Loader{modelName, reader, loadMaterial};
               return loader.load(logger);
             }
             if (IO::MdxLoader::canParse(path, reader))
             {
               auto loader = IO::MdxLoader{modelName, reader, fs};
               return loader.load(logger);
             }
             if (IO::DkmLoader::canParse(path, reader))
             {
               auto loader = IO::DkmLoader{modelName, reader, fs};
               return loader.load(logger);
             }
             if (IO::AseLoader::canParse(path))
             {
               auto loader = IO::AseLoader{modelName, reader.stringView(), loadMaterial};
               return loader.load(logger);
             }
             if (IO::ImageSpriteLoader::canParse(path))
             {
               auto loader = IO::ImageSpriteLoader{modelName, file, fs};
               return loader.load(logger);
             }
             if (IO::AssimpLoader::canParse(path))
             {
               auto loader = IO::AssimpLoader{path, fs};
               return loader.load(logger);
             }
             return Error{"Unknown model format: '" + path.string() + "'"};
           });
}
} // namespace

Result<Assets::EntityModel> loadEntityModel(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return loadEntityModelData(fs, materialConfig, path, loadMaterial, logger)
         | kdl::transform([&](auto modelData) {
             auto modelName = path.filename().string();
             return Assets::EntityModel{std::move(modelName), std::move(modelData)};
           });
}

} // namespace TrenchBroom::IO
