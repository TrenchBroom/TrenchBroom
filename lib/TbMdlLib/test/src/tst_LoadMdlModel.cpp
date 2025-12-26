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
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/Reader.h"
#include "mdl/CatchConfig.h"
#include "mdl/EntityModel.h"
#include "mdl/LoadMdlModel.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadMdlModel")
{
  auto logger = NullLogger{};

  const auto palettePath = "fixture/test/mdl/LoadMdlModel/palette.lmp";
  auto fs = fs::DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile(palettePath) | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  SECTION("valid MDL model")
  {
    const auto mdlPath =
      std::filesystem::current_path() / "fixture/test/mdl/LoadMdlModel/armor.mdl";
    const auto mdlFile = fs::Disk::openFile(mdlPath) | kdl::value();

    auto reader = mdlFile->reader().buffer();
    loadMdlModel("armor", reader, palette, logger)
      | kdl::transform([](const auto& modelData) {
          CHECK(modelData.surfaceCount() == 1u);
          CHECK(modelData.frameCount() == 1u);

          const auto& surfaces = modelData.surfaces();
          const auto& surface = surfaces.front();
          CHECK(surface.skinCount() == 3u);
          CHECK(surface.frameCount() == 1u);
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("invalid MDL file")
  {
    const auto mdlPath =
      std::filesystem::current_path() / "fixture/test/mdl/LoadMdlModel/invalid.mdl";
    const auto mdlFile = fs::Disk::openFile(mdlPath) | kdl::value();

    auto reader = mdlFile->reader().buffer();
    CHECK(
      loadMdlModel("armor", reader, palette, logger)
      == Result<mdl::EntityModelData>{Error{"Unknown MDL model version: 538976288"}});
  }
}

} // namespace tb::mdl
