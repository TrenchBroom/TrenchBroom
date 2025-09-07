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

#include "io/DiskFileSystem.h"
#include "io/ReadWalTexture.h"
#include "mdl/Palette.h"
#include "mdl/Texture.h"

#include "kdl/result.h"

#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::io
{
namespace
{
constexpr auto fixturePath = "fixture/test/io/Wal";
}

TEST_CASE("readWalTexture")
{
  const auto palettePath = "fixture/test/colormap.pcx";
  auto fs = DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile("fixture/test/colormap.pcx") | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  using TexInfo =
    std::tuple<std::filesystem::path, size_t, size_t, mdl::EmbeddedDefaults>;

  // clang-format off
  const auto 
  [path,                  width, height, embeddedDefaults] = GENERATE(values<TexInfo>({
  { "rtz/b_pv_v1a1.wal",  128,   256,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "rtz/b_pv_v1a2.wal",  128,   256,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "rtz/b_pv_v1a3.wal",  128,   128,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "rtz/b_rc_v16.wal",   128,   128,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "rtz/b_rc_v16w.wal",  128,   128,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "rtz/b_rc_v28.wal",   128,    64,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "rtz/b_rc_v4.wal",    128,   128,    mdl::Q2EmbeddedDefaults{0, 0, 0} },
  { "lavatest.wal",       64,     64,    mdl::Q2EmbeddedDefaults{9, 8, 700} },
  { "watertest.wal",      64,     64,    mdl::Q2EmbeddedDefaults{9, 32, 120} },
  }));
  // clang-format on

  INFO(path);
  INFO(width);
  INFO(height);

  const auto file = fs.openFile(fixturePath / path) | kdl::value();
  auto reader = file->reader().buffer();

  const auto name = path.stem().generic_string();
  const auto texture = readWalTexture(reader, palette) | kdl::value();
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
  CHECK(texture.embeddedDefaults() == embeddedDefaults);
}

} // namespace tb::io
