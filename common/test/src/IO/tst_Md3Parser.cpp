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

#include "Assets/EntityModel.h"
#include "Assets/Palette.h"
#include "Assets/Quake3Shader.h"
#include "IO/DiskFileSystem.h"
#include "IO/File.h"
#include "IO/LoadMaterialCollections.h"
#include "IO/LoadShaders.h"
#include "IO/MaterialUtils.h"
#include "IO/Md3Parser.h"
#include "IO/Reader.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "vm/bbox.h"
#include "vm/forward.h"
#include "vm/vec.h"

#include <cstdio>
#include <filesystem>
#include <memory>

#include "Catch2.h" // IWYU pragma: keep

namespace TrenchBroom::IO
{
TEST_CASE("Md3ParserTest.loadValidMd3")
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
  fs.mount(
    "",
    std::make_unique<DiskFileSystem>(
      std::filesystem::current_path() / "fixture/test/IO/Md3/bfg"));

  const auto shaders = loadShaders(fs, materialConfig, logger) | kdl::value();

  const auto createResource = [](auto resourceLoader) {
    return createResourceSync(std::move(resourceLoader));
  };

  const auto loadMaterial = [&](const auto& materialPath) {
    return IO::loadMaterial(
             fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
           | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
  };

  const auto md3Path = "models/weapons2/bfg/bfg.md3";
  const auto md3File = fs.openFile(md3Path) | kdl::value();

  auto reader = md3File->reader().buffer();
  auto parser = Md3Parser("bfg", reader, loadMaterial);
  auto model = parser.initializeModel(logger);

  CHECK(model.is_success());

  CHECK(model.value().frameCount() == 1u);
  CHECK(model.value().surfaceCount() == 2u);

  const auto* frame = model.value().frame("MilkShape 3D");
  CHECK(frame != nullptr);
  CHECK(vm::is_equal(
    vm::bbox3f(
      vm::vec3f(-10.234375, -10.765625, -9.4375),
      vm::vec3f(30.34375, 10.765625, 11.609375)),
    frame->bounds(),
    0.01f));

  const auto* surface1 = model.value().surface("x_bfg");
  CHECK(surface1 != nullptr);
  CHECK(surface1->frameCount() == 1u);
  CHECK(surface1->skinCount() == 1u);

  const auto* skin1 = surface1->skin("models/weapons2/bfg/LDAbfg");
  CHECK(skin1 != nullptr);

  const auto* surface2 = model.value().surface("x_fx");
  CHECK(surface2 != nullptr);
  CHECK(surface2->frameCount() == 1u);
  CHECK(surface2->skinCount() == 1u);

  const auto* skin2 = surface2->skin("models/weapons2/bfg/LDAbfg_z");
  CHECK(skin2 != nullptr);
}
} // namespace TrenchBroom::IO
