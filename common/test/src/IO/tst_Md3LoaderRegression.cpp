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
#include "IO/Md3Loader.h"
#include "IO/Reader.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include <cstdio>
#include <filesystem>
#include <memory>

#include "Catch2.h" // IWYU pragma: keep

namespace TrenchBroom::IO
{
TEST_CASE("Md3LoaderTest.loadFailure_2659")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2659

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
      std::filesystem::current_path() / "fixture/test/IO/Md3/armor"));

  const auto shaders = loadShaders(fs, materialConfig, logger) | kdl::value();

  const auto createResource = [](auto resourceLoader) {
    return createResourceSync(std::move(resourceLoader));
  };

  const auto loadMaterial = [&](const auto& materialPath) {
    return IO::loadMaterial(
             fs, materialConfig, materialPath, createResource, shaders, std::nullopt)
           | kdl::or_else(IO::makeReadMaterialErrorHandler(fs, logger)) | kdl::value();
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
} // namespace TrenchBroom::IO
