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

#include "Logger.h"
#include "io/DiskFileSystem.h"
#include "io/LoadMaterialCollections.h"
#include "io/LoadShaders.h"
#include "io/MaterialUtils.h"
#include "io/Md3Loader.h"
#include "io/Reader.h"
#include "io/VirtualFileSystem.h"
#include "mdl/EntityModel.h"
#include "mdl/GameConfig.h"
#include "mdl/Palette.h"

#include "kdl/task_manager.h"

#include "vm/bbox.h"

#include <filesystem>
#include <memory>

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("Md3Loader")
{
  auto logger = NullLogger{};
  auto taskManager = kdl::task_manager{};
  auto fs = VirtualFileSystem{};

  const auto materialConfig = mdl::MaterialConfig{
    {},
    {".tga", ".png", ".jpg", ".jpeg"},
    {},
    {},
    "scripts",
    {},
  };

  SECTION("Load valid MD3 model")
  {
    fs.mount(
      "",
      std::make_unique<DiskFileSystem>(
        std::filesystem::current_path() / "fixture/test/io/Md3/bfg"));

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

    const auto md3Path = "models/weapons2/bfg/bfg.md3";
    const auto md3File = fs.openFile(md3Path) | kdl::value();

    auto reader = md3File->reader().buffer();
    auto loader = Md3Loader{"bfg", reader, loadMaterial};
    auto modelData = loader.load(logger);

    CHECK(modelData.is_success());

    CHECK(modelData.value().frameCount() == 1u);
    CHECK(modelData.value().surfaceCount() == 2u);

    const auto* frame = modelData.value().frame("MilkShape 3D");
    CHECK(frame != nullptr);
    CHECK(vm::is_equal(
      vm::bbox3f(
        vm::vec3f(-10.234375, -10.765625, -9.4375),
        vm::vec3f(30.34375, 10.765625, 11.609375)),
      frame->bounds(),
      0.01f));

    const auto* surface1 = modelData.value().surface("x_bfg");
    CHECK(surface1 != nullptr);
    CHECK(surface1->frameCount() == 1u);
    CHECK(surface1->skinCount() == 1u);

    const auto* skin1 = surface1->skin("models/weapons2/bfg/LDAbfg");
    CHECK(skin1 != nullptr);

    const auto* surface2 = modelData.value().surface("x_fx");
    CHECK(surface2 != nullptr);
    CHECK(surface2->frameCount() == 1u);
    CHECK(surface2->skinCount() == 1u);

    const auto* skin2 = surface2->skin("models/weapons2/bfg/LDAbfg_z");
    CHECK(skin2 != nullptr);
  }
}

TEST_CASE("Md3Loader (Regression)", "[regression]")
{
  auto logger = NullLogger{};
  auto taskManager = kdl::task_manager{};
  auto fs = VirtualFileSystem{};

  const auto materialConfig = mdl::MaterialConfig{
    {},
    {".tga", ".png", ".jpg", ".jpeg"},
    {},
    {},
    "scripts",
    {},
  };

  SECTION("2659")
  {
    fs.mount(
      "",
      std::make_unique<DiskFileSystem>(
        std::filesystem::current_path() / "fixture/test/io/Md3/armor"));

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

    const auto md3Path = "models/armor_red.md3";
    const auto md3File = fs.openFile(md3Path) | kdl::value();

    auto reader = md3File->reader().buffer();
    auto loader = Md3Loader{"armor_red", reader, loadMaterial};
    auto modelData = loader.load(logger);

    CHECK(modelData.is_success());

    CHECK(modelData.value().frameCount() == 30u);
    CHECK(modelData.value().surfaceCount() == 2u);
  }
}

} // namespace tb::io
