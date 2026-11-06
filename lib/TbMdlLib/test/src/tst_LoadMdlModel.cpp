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

#include "TestEnvironment.h"
#include "base/Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/Reader.h"
#include "gl/Texture.h"
#include "mdl/CatchConfig.h"
#include "mdl/EntityModel.h"
#include "mdl/LoadMdlModel.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/bbox_io.h" // IWYU pragma: keep
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <optional>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadMdlModel")
{
  auto logger = NullLogger{};

  const auto palettePath = "test/mdl/LoadMdlModel/palette.lmp";
  auto fs = fs::DiskFileSystem{getFixtureRoot()};
  auto paletteFile = fs.openFile(palettePath) | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  SECTION("valid MDL model")
  {
    const auto mdlPath = getFixtureRoot() / "test/mdl/LoadMdlModel/armor.mdl";
    const auto mdlFile = fs::Disk::openFile(mdlPath) | kdl::value();

    auto reader = mdlFile->reader().buffer();
    loadMdlModel("armor", reader, palette, fs, logger)
      | kdl::transform([](const auto& modelData) {
          CHECK(modelData.surfaceCount() == 1u);
          CHECK(modelData.frameCount() == 1u);

          const auto& surfaces = modelData.surfaces();
          const auto& surface = surfaces.front();
          CHECK(surface.skinCount() == 3u);
          CHECK(surface.frameCount() == 1u);

          // armor.mdl has no MF_HOLEY skins, so every skin's texture should be
          // classified as Opaque; the MF_HOLEY (mask == On -> Binary) branch shares
          // its logic with readMipTexture(), which is covered directly in
          // tst_LoadMipTexture.cpp.
          for (size_t i = 0; i < surface.skinCount(); ++i)
          {
            const auto* skin = surface.skin(i);
            REQUIRE(skin != nullptr);
            REQUIRE(skin->texture() != nullptr);
            CHECK(skin->texture()->alphaDomain() == img::ImageAlphaDomain::Opaque);
          }
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("missing palette")
  {
    const auto mdlPath = getFixtureRoot() / "test/mdl/LoadMdlModel/armor.mdl";
    const auto mdlFile = fs::Disk::openFile(mdlPath) | kdl::value();

    auto reader = mdlFile->reader().buffer();
    const auto expectedBounds = loadMdlModel("armor", reader, palette, fs, logger)
                                | kdl::transform([](const auto& modelData) {
                                    return modelData.frames().front().bounds();
                                  })
                                | kdl::value();

    loadMdlModel("armor", reader, std::nullopt, fs, logger)
      | kdl::transform([&](const auto& modelData) {
          CHECK(modelData.surfaceCount() == 1u);
          CHECK(modelData.frameCount() == 1u);

          const auto& surfaces = modelData.surfaces();
          const auto& surface = surfaces.front();
          CHECK(surface.skinCount() == 3u);
          CHECK(surface.frameCount() == 1u);

          // the frame is only parsed correctly if the skin data was skipped correctly
          CHECK(modelData.frames().front().bounds() == expectedBounds);
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("skin groups")
  {
    // a 2x2 model with an empty skin group, a skin group with two pictures and a single
    // skin, followed by three vertices, one triangle and one frame
    const auto mdlPath = getFixtureRoot() / "test/mdl/LoadMdlModel/skin_groups.mdl";
    const auto mdlFile = fs::Disk::openFile(mdlPath) | kdl::value();

    auto reader = mdlFile->reader().buffer();

    SECTION("with palette")
    {
      loadMdlModel("skin_groups", reader, palette, fs, logger)
        | kdl::transform([](const auto& modelData) {
            const auto& surface = modelData.surfaces().front();
            REQUIRE(surface.skinCount() == 3u);

            // the empty skin group gets the default texture
            CHECK(surface.skin(0u)->texture()->width() == 32u);
            CHECK(surface.skin(1u)->texture()->width() == 2u);
            CHECK(surface.skin(2u)->texture()->width() == 2u);

            // the frame is only parsed correctly if the skin data was skipped correctly
            REQUIRE(modelData.frameCount() == 1u);
            CHECK(modelData.frames().front().name() == "frame");
            CHECK(
              modelData.frames().front().bounds()
              == vm::bbox3f{vm::vec3f{1, 2, 3}, vm::vec3f{7, 8, 9}});
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }

    SECTION("missing palette")
    {
      loadMdlModel("skin_groups", reader, std::nullopt, fs, logger)
        | kdl::transform([](const auto& modelData) {
            const auto& surface = modelData.surfaces().front();
            REQUIRE(surface.skinCount() == 3u);

            CHECK(surface.skin(0u)->texture()->width() == 32u);
            CHECK(surface.skin(1u)->texture()->width() == 32u);
            CHECK(surface.skin(2u)->texture()->width() == 32u);

            REQUIRE(modelData.frameCount() == 1u);
            CHECK(modelData.frames().front().name() == "frame");
            CHECK(
              modelData.frames().front().bounds()
              == vm::bbox3f{vm::vec3f{1, 2, 3}, vm::vec3f{7, 8, 9}});
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }
  }

  SECTION("invalid MDL file")
  {
    const auto mdlPath = getFixtureRoot() / "test/mdl/LoadMdlModel/invalid.mdl";
    const auto mdlFile = fs::Disk::openFile(mdlPath) | kdl::value();

    auto reader = mdlFile->reader().buffer();
    CHECK(
      loadMdlModel("armor", reader, palette, fs, logger)
      == Result<mdl::EntityModelData>{Error{"Unknown MDL model version: 538976288"}});
  }
}

} // namespace tb::mdl
