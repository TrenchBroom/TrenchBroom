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
#include "TestLogger.h"
#include "base/Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/WadFileSystem.h"
#include "mdl/CatchConfig.h"
#include "mdl/LoadMipTexture.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("LoadMipTexture")
{
  SECTION("isIdMipTexture")
  {
    CHECK(isIdMipTexture("texture.d"));
    CHECK(isIdMipTexture("texture.D"));
    CHECK(isIdMipTexture("path/to/texture.d"));
    CHECK(!isIdMipTexture("texture.c"));
    CHECK(!isIdMipTexture("texture.wal"));
    CHECK(!isIdMipTexture("texture"));
  }

  SECTION("isHlMipTexture")
  {
    CHECK(isHlMipTexture("texture.c"));
    CHECK(isHlMipTexture("texture.C"));
    CHECK(isHlMipTexture("path/to/texture.c"));
    CHECK(!isHlMipTexture("texture.d"));
    CHECK(!isHlMipTexture("texture.wal"));
    CHECK(!isHlMipTexture("texture"));
  }

  SECTION("isMipTexture")
  {
    CHECK(isMipTexture("texture.d"));
    CHECK(isMipTexture("texture.D"));
    CHECK(isMipTexture("texture.c"));
    CHECK(isMipTexture("texture.C"));
    CHECK(isMipTexture("path/to/texture.d"));
    CHECK(!isMipTexture("texture.wal"));
    CHECK(!isMipTexture("texture"));
  }

  SECTION("loadIdMipTexture")
  {
    using TexInfo = std::tuple<std::string, size_t, size_t>;

    // clang-format off
    const auto [textureName, width, height] = GENERATE(values<TexInfo>({
    { "cr8_czg_1",          64,  64 },
    { "cr8_czg_2",          64,  64 },
    { "cr8_czg_3",          64, 128 },
    { "cr8_czg_4",          64, 128 },
    { "cr8_czg_5",          64, 128 },
    { "speedM_1",          128, 128 },
    { "cap4can-o-jam",      64,  64 },
    { "can-o-jam",          64,  64 },
    { "eat_me",             64,  64 },
    { "coffin1",           128, 128 },
    { "coffin2",           128, 128 },
    { "czg_fronthole",     128, 128 },
    { "czg_backhole",      128, 128 },
    { "u_get_this",         64,  64 },
    { "for_sux-m-ass",      64,  64 },
    { "dex_5",             128, 128 },
    { "polished_turd",      64,  64 },
    { "crackpipes",        128, 128 },
    { "bongs2",            128, 128 },
    { "blowjob_machine",   128, 128 },
    { "lasthopeofhuman",   128, 128 },
    }));
    // clang-format on

    const auto palettePath = getFixtureRoot() / "test/mdl/LoadMipTexture/palette.lmp";
    const auto wadPath = getFixtureRoot() / "test/mdl/LoadMipTexture/cr8_czg.wad";

    fs::Disk::openFile(wadPath) | kdl::transform([&](auto wadFile) {
      return fs::WadFileSystem{wadFile};
    }) | kdl::and_then([&](auto wadFS) {
      REQUIRE(wadFS.reload());
      return wadFS.openFile(textureName + ".D")
             | kdl::join(
               fs::Disk::openFile(palettePath) | kdl::and_then([&](auto paletteFile) {
                 return mdl::loadPalette(*paletteFile, palettePath);
               }))
             | kdl::and_then([](auto textureFile, auto palette) {
                 auto reader = textureFile->reader().buffer();
                 return loadIdMipTexture(reader, palette, false);
               })
             | kdl::transform([&](auto texture) {
                 CHECK(texture.width() == width);
                 CHECK(texture.height() == height);
               });
    }) | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("loadIdMipTexture sets alphaDomain from the mask")
  {
    const auto isMasked = GENERATE(true, false);
    CAPTURE(isMasked);

    const auto palettePath = getFixtureRoot() / "test/mdl/LoadMipTexture/palette.lmp";
    const auto wadPath = getFixtureRoot() / "test/mdl/LoadMipTexture/cr8_czg.wad";

    fs::Disk::openFile(wadPath) | kdl::transform([&](auto wadFile) {
      return fs::WadFileSystem{wadFile};
    }) | kdl::and_then([&](auto wadFS) {
      REQUIRE(wadFS.reload());
      return wadFS.openFile("cr8_czg_1.D")
             | kdl::join(
               fs::Disk::openFile(palettePath) | kdl::and_then([&](auto paletteFile) {
                 return mdl::loadPalette(*paletteFile, palettePath);
               }))
             | kdl::and_then([&](auto textureFile, auto palette) {
                 auto reader = textureFile->reader().buffer();
                 return loadIdMipTexture(reader, palette, isMasked);
               })
             | kdl::transform([&](auto texture) {
                 CHECK(
                   texture.alphaDomain()
                   == (isMasked ? img::ImageAlphaDomain::Binary : img::ImageAlphaDomain::Opaque));
               });
    }) | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("loadHlMipTexture")
  {
    using TexInfo = std::tuple<std::string, size_t, size_t>;

    // clang-format off
    const auto [textureName, width, height] = GENERATE(values<TexInfo>({
    { "bongs2",            128, 128 },
    { "blowjob_machine",   128, 128 },
    }));
    // clang-format on

    auto fs = fs::DiskFileSystem{getFixtureRoot()};

    auto logger = TestLogger{};

    const auto wadPath = getFixtureRoot() / "test/mdl/LoadMipTexture/hl.wad";
    auto wadFS = fs::WadFileSystem{fs::Disk::openFile(wadPath) | kdl::value()};
    REQUIRE(wadFS.reload());

    const auto file = wadFS.openFile(textureName + ".C") | kdl::value();
    auto reader = file->reader().buffer();
    const auto texture = loadHlMipTexture(reader, false) | kdl::value();

    CHECK(logger.countMessages(LogLevel::Error) == 0);
    CHECK(logger.countMessages(LogLevel::Warn) == 0);
    CHECK(texture.width() == width);
    CHECK(texture.height() == height);
  }

  SECTION("loadHlMipTexture sets alphaDomain from the mask")
  {
    const auto isMasked = GENERATE(true, false);
    CAPTURE(isMasked);

    const auto wadPath = getFixtureRoot() / "test/mdl/LoadMipTexture/hl.wad";
    auto wadFS = fs::WadFileSystem{fs::Disk::openFile(wadPath) | kdl::value()};
    REQUIRE(wadFS.reload());

    const auto file = wadFS.openFile("bongs2.C") | kdl::value();
    auto reader = file->reader().buffer();
    const auto texture = loadHlMipTexture(reader, isMasked) | kdl::value();

    CHECK(
      texture.alphaDomain()
      == (isMasked ? img::ImageAlphaDomain::Binary : img::ImageAlphaDomain::Opaque));
  }
}

} // namespace tb::mdl
