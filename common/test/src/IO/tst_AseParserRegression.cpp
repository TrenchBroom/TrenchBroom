/*
 Copyright (C) 2018 Eric Wasylishen

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

#include "Assets/EntityModel.h"
#include "Assets/Material.h"
#include "Assets/Palette.h"
#include "Assets/Quake3Shader.h"
#include "IO/AseParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/File.h"
#include "IO/LoadMaterialCollections.h"
#include "IO/LoadShaders.h"
#include "IO/MaterialUtils.h"
#include "IO/Reader.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "Catch2.h" // IWYU pragma: keep

namespace TrenchBroom::IO
{
TEST_CASE("AseParserTest")
{
  auto logger = NullLogger{};

  const auto materialConfig = Model::MaterialConfig{
    {},
    {".tga", ".png", ".jpg", ".jpeg"},
    {},
    {},
    "scripts",
    {},
  };

  auto fs = VirtualFileSystem{};

  const auto defaultAssetsPath =
    std::filesystem::current_path() / "fixture/test/IO/ResourceUtils/assets";
  fs.mount("", std::make_unique<DiskFileSystem>(defaultAssetsPath));

  SECTION("parseFailure_2657")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/IO/Ase/steelstorm_player";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders = loadShaders(fs, materialConfig, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return IO::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile = fs.openFile("player.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto parser = AseParser{"player", reader.stringView(), loadMaterial};

    auto model = parser.initializeModel(logger);
    CHECK(model.is_success());
  }

  SECTION("parseFailure_2679")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/IO/Ase/no_scene_directive";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders = loadShaders(fs, materialConfig, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return IO::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile = fs.openFile("wedge_45.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto parser = AseParser{"wedge", reader.stringView(), loadMaterial};

    auto model = parser.initializeModel(logger);
    CHECK(model.is_success());
  }

  SECTION("parseFailure_2898_vertex_index")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/IO/Ase/index_out_of_bounds";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders = loadShaders(fs, materialConfig, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return IO::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile = fs.openFile("wedge_45.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto parser = AseParser{"wedge", reader.stringView(), loadMaterial};

    auto model = parser.initializeModel(logger);
    CHECK(model.is_success());
  }

  SECTION("parseFailure_2898_no_uv")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/IO/Ase/index_out_of_bounds";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders = loadShaders(fs, materialConfig, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return IO::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile = fs.openFile("wedge_45_no_uv.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto parser = AseParser{"wedge", reader.stringView(), loadMaterial};

    auto model = parser.initializeModel(logger);
    CHECK(model.is_success());
  }
}

} // namespace TrenchBroom::IO
