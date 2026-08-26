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
#include "fs/DiskFileSystem.h"
#include "gl/Texture.h"
#include "mdl/CatchConfig.h"
#include "mdl/LoadWalTexture.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{
const auto basePath = std::filesystem::path{"test/mdl/LoadWalTexture"};
}

TEST_CASE("loadWalTexture")
{
  auto fs = fs::DiskFileSystem{getFixtureRoot()};

  SECTION("isWalTexture")
  {
    CHECK(isWalTexture("texture.wal"));
    CHECK(isWalTexture("texture.WAL"));
    CHECK(isWalTexture("path/to/texture.wal"));
    CHECK(!isWalTexture("texture.d"));
    CHECK(!isWalTexture("texture"));
  }

  SECTION("Quake2")
  {
    const auto palettePath = basePath / "colormap.pcx";
    auto paletteFile = fs.openFile(palettePath) | kdl::value();
    const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

    using TexInfo =
      std::tuple<std::filesystem::path, size_t, size_t, gl::EmbeddedDefaults>;

    // clang-format off
    const auto
    [path,                  width, height, embeddedDefaults] = GENERATE(values<TexInfo>({
    { "rtz/b_pv_v1a1.wal",  128,   256,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "rtz/b_pv_v1a2.wal",  128,   256,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "rtz/b_pv_v1a3.wal",  128,   128,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "rtz/b_rc_v16.wal",   128,   128,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "rtz/b_rc_v16w.wal",  128,   128,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "rtz/b_rc_v28.wal",   128,    64,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "rtz/b_rc_v4.wal",    128,   128,    gl::Q2EmbeddedDefaults{0, 0, 0} },
    { "lavatest.wal",       64,     64,    gl::Q2EmbeddedDefaults{9, 8, 700} },
    { "watertest.wal",      64,     64,    gl::Q2EmbeddedDefaults{9, 32, 120} },
    }));
    // clang-format on

    INFO(path);
    INFO(width);
    INFO(height);

    const auto file = fs.openFile(basePath / path) | kdl::value();
    auto reader = file->reader().buffer();

    const auto name = path.stem().generic_string();
    const auto texture = loadWalTexture(reader, palette) | kdl::value();
    CHECK(texture.width() == width);
    CHECK(texture.height() == height);
    CHECK(texture.embeddedDefaults() == embeddedDefaults);

    // Quake2 WAL textures never carry transparency in TrenchBroom's loader
    CHECK(texture.alphaDomain() == img::ImageAlphaDomain::Opaque);
  }

  // Daikatana WAL textures (version 3) carry their own embedded palette and use the
  // classic index-255-is-transparent convention, computed while decoding pixel data
  // (i.e. lazily, unlike the `{`-prefixed name convention used by Quake1/Half-Life).
  // Since that convention only ever produces fully transparent or fully opaque pixels,
  // it can never classify as Graduated.
  SECTION("Daikatana")
  {
    SECTION("without a transparent index")
    {
      const auto file = fs.openFile(basePath / "dk_opaque.wal") | kdl::value();
      auto reader = file->reader().buffer();

      const auto texture = loadWalTexture(reader, std::nullopt) | kdl::value();
      CHECK(texture.width() == 2);
      CHECK(texture.height() == 2);
      CHECK(texture.alphaDomain() == img::ImageAlphaDomain::Opaque);
    }

    SECTION("with a transparent index")
    {
      const auto file = fs.openFile(basePath / "dk_binaryAlpha.wal") | kdl::value();
      auto reader = file->reader().buffer();

      const auto texture = loadWalTexture(reader, std::nullopt) | kdl::value();
      CHECK(texture.width() == 2);
      CHECK(texture.height() == 2);
      CHECK(texture.alphaDomain() == img::ImageAlphaDomain::Binary);
    }
  }
}

} // namespace tb::mdl
