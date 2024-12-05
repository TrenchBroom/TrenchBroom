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
#include "mdl/GameConfig.h"
#include "mdl/Palette.h"

#include "kdl/task_manager.h"

#include <filesystem>
#include <memory>

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("Md3LoaderTest.loadFailure_2659")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2659

  auto logger = NullLogger{};

  const auto materialConfig = mdl::MaterialConfig{
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
      std::filesystem::current_path() / "fixture/test/io/Md3/armor"));

  auto taskManager = kdl::task_manager{};

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

} // namespace tb::io
