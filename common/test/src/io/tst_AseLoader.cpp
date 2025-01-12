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

#include "Logger.h"
#include "io/AseLoader.h"
#include "io/DiskFileSystem.h"
#include "io/LoadMaterialCollections.h"
#include "io/LoadShaders.h"
#include "io/MaterialUtils.h"
#include "io/Reader.h"
#include "io/VirtualFileSystem.h"
#include "mdl/EntityModel.h"
#include "mdl/GameConfig.h"
#include "mdl/Material.h"
#include "mdl/Palette.h"

#include "kdl/k.h"
#include "kdl/path_utils.h"
#include "kdl/task_manager.h"

#include <filesystem>

#include "Catch2.h"

namespace tb::io
{
TEST_CASE("AseLoaderTest")
{
  auto logger = NullLogger{};

  const auto materialConfig = mdl::MaterialConfig{
    {},
    {".tga", ".png", ".jpg", ".jpeg"},
    {},
    {},
    "scripts",
    {},
  };

  const auto defaultAssetsPath =
    std::filesystem::current_path() / "fixture/test/io/ResourceUtils/assets";
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(defaultAssetsPath));

  auto taskManager = kdl::task_manager{};

  SECTION("Models load without exception")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/io/Ase/wedge_with_shader";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders =
      loadShaders(fs, materialConfig, taskManager, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return io::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(io::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile =
      fs.openFile("models/mapobjects/wedges/wedge_45.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto loader = AseLoader{"wedge", reader.stringView(), loadMaterial};

    auto modelResult = loader.load(logger);
    CHECK(modelResult.is_success());

    SECTION("Windows paths are converted to generic paths")
    {
      for (const auto& surface : modelResult.value().surfaces())
      {
        for (size_t i = 0; i < surface.skinCount(); ++i)
        {
          const auto* skin = surface.skin(i);
          CHECK(
            skin->relativePath()
            == kdl::parse_path(skin->relativePath().string(), K(replace_backslashes)));
        }
      }
    }
  }

  SECTION("Fall back to material name if bitmap directive is missing")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/io/Ase/fallback_to_materialname";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders =
      loadShaders(fs, materialConfig, taskManager, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return io::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(io::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile = fs.openFile("models/wedge_45.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto loader = AseLoader{"wedge", reader.stringView(), loadMaterial};

    auto modelData = loader.load(logger);
    CHECK(modelData.is_success());

    // account for the default material
    CHECK(modelData.value().surface(0).skinCount() == 2u);
    CHECK(modelData.value().surface(0).skin(0)->name() == "textures/bigtile");
  }

  SECTION("Fall back to default material if texture cannot be loaded")
  {
    const auto basePath =
      std::filesystem::current_path() / "fixture/test/io/Ase/load_default_material";
    fs.mount("", std::make_unique<DiskFileSystem>(basePath));

    const auto shaders =
      loadShaders(fs, materialConfig, taskManager, logger) | kdl::value();

    const auto createResource = [](auto resourceLoader) {
      return createResourceSync(std::move(resourceLoader));
    };

    const auto loadMaterial = [&](const auto& materialPath) {
      return io::loadMaterial(
               fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
             | kdl::or_else(io::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
    };

    const auto aseFile = fs.openFile("models/wedge_45.ase") | kdl::value();
    auto reader = aseFile->reader().buffer();
    auto loader = AseLoader{"wedge", reader.stringView(), loadMaterial};

    auto modelData = loader.load(logger);
    CHECK(modelData.is_success());

    // account for the default texture
    CHECK(modelData.value().surface(0).skinCount() == 2u);
    // shader name is correct, but we loaded the default material

    const auto* material = modelData.value().surface(0).skin(0);
    CHECK(material->name() == "textures/bigtile");
    CHECK(material->texture()->width() == 32u);
    CHECK(material->texture()->height() == 32u);
  }
}

} // namespace tb::io
