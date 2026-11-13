/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2024 Léo Peltier

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


#include "TestEnvironment.h"
#include "TestLogger.h"
#include "base/Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/Reader.h"
#include "gl/Material.h"
#include "gl/Texture.h"
#include "mdl/CatchConfig.h"
#include "mdl/EntityModel.h"
#include "mdl/LoadBspModel.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <optional>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadBspModel")
{
  auto logger = TestLogger{};

  const auto palettePath = "palette.lmp";
  auto fs = fs::DiskFileSystem{getFixtureRoot() / "test/mdl/LoadBspModel"};
  auto paletteFile = fs.openFile(palettePath) | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  SECTION("valid Half-Life BSP")
  {
    const auto bspPath = getFixtureRoot() / "test/mdl/LoadBspModel/hl.bsp";
    const auto bspFile = fs::Disk::openFile(bspPath) | kdl::value();

    auto reader = bspFile->reader().buffer();
    loadBspModel("hl", reader, palette, fs, logger)
      | kdl::transform([](const auto& bspData) {
          CHECK(bspData.surfaceCount() == 1u);
          CHECK(bspData.frameCount() == 1u);

          const auto& surfaces = bspData.surfaces();
          const auto& surface = surfaces.front();
          REQUIRE(surface.skinCount() == 3u);
          CHECK(surface.frameCount() == 1u);

          CHECK(surface.skin(0u)->name() == "skip");
          CHECK(surface.skin(1u)->name() == "embedded");
          CHECK(surface.skin(2u)->name() == "EXTERNAL");
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("valid Quake BSP")
  {
    const auto bspPath = getFixtureRoot() / "test/mdl/LoadBspModel/quake.bsp";
    const auto bspFile = fs::Disk::openFile(bspPath) | kdl::value();

    auto reader = bspFile->reader().buffer();
    loadBspModel("quake", reader, palette, fs, logger)
      | kdl::transform([](const auto& bspData) {
          CHECK(bspData.surfaceCount() == 1u);
          CHECK(bspData.frameCount() == 1u);

          const auto& surfaces = bspData.surfaces();
          const auto& surface = surfaces.front();
          REQUIRE(surface.skinCount() == 3u);
          CHECK(surface.frameCount() == 1u);

          CHECK(surface.skin(0u)->name() == "skip");
          CHECK(surface.skin(1u)->name() == "embedded");
          CHECK(surface.skin(2u)->name() == "EXTERNAL");

          // the embedded textures are decoded with the palette into all four mip levels,
          // unlike the default texture, which has only one
          CHECK(surface.skin(0u)->texture()->buffersIfLoaded().size() == 4u);
          CHECK(surface.skin(1u)->texture()->buffersIfLoaded().size() == 4u);
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });

    // only the external texture, which is not embedded in the BSP, is reported
    CHECK(logger.countMessages(LogLevel::Error) == 1);
  }

  SECTION("missing palette")
  {
    SECTION("Quake BSPs require the palette")
    {
      const auto bspPath = getFixtureRoot() / "test/mdl/LoadBspModel/quake.bsp";
      const auto bspFile = fs::Disk::openFile(bspPath) | kdl::value();

      auto reader = bspFile->reader().buffer();
      loadBspModel("quake", reader, std::nullopt, fs, logger)
        | kdl::transform([](const auto& bspData) {
            CHECK(bspData.surfaceCount() == 1u);
            CHECK(bspData.frameCount() == 1u);

            const auto& surfaces = bspData.surfaces();
            const auto& surface = surfaces.front();
            REQUIRE(surface.skinCount() == 3u);
            CHECK(surface.frameCount() == 1u);

            CHECK(surface.skin(0u)->name() == "skip");
            CHECK(surface.skin(1u)->name() == "embedded");
            CHECK(surface.skin(2u)->name() == "EXTERNAL");

            // the textures get the default texture, which has pixel data unlike the
            // empty texture that replaces it if it cannot be loaded
            for (size_t i = 0; i < surface.skinCount(); ++i)
            {
              const auto* texture = surface.skin(i)->texture();
              CHECK(texture->width() == 32u);
              CHECK(!texture->buffersIfLoaded().empty());
            }
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });

      // each texture that requires the missing palette is reported
      CHECK(logger.countMessages(LogLevel::Error) == 3);
    }

    SECTION("Half-Life BSPs have embedded palettes")
    {
      const auto bspPath = getFixtureRoot() / "test/mdl/LoadBspModel/hl.bsp";
      const auto bspFile = fs::Disk::openFile(bspPath) | kdl::value();

      auto reader = bspFile->reader().buffer();
      CHECK(loadBspModel("hl", reader, std::nullopt, fs, logger).is_success());

      // only the external texture, which is not embedded in the BSP, is reported
      CHECK(logger.countMessages(LogLevel::Error) == 1);
    }
  }

  SECTION("invalid BSP")
  {
    const auto bspPath = getFixtureRoot() / "test/mdl/LoadBspModel/invalid_version.bsp";
    const auto bspFile = fs::Disk::openFile(bspPath) | kdl::value();

    auto reader = bspFile->reader().buffer();
    CHECK(
      loadBspModel("hl", reader, palette, fs, logger)
      == Result<mdl::EntityModelData>{Error{"Unsupported BSP model version: 63"}});
  }
}

} // namespace tb::mdl
