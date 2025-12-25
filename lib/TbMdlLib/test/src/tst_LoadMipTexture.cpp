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
#include "TestLogger.h"
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

TEST_CASE("loadIdMipTexture")
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

  const auto palettePath =
    std::filesystem::current_path() / "fixture/test/mdl/LoadMipTexture/palette.lmp";
  const auto wadPath =
    std::filesystem::current_path() / "fixture/test/mdl/LoadMipTexture/cr8_czg.wad";

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
               return loadIdMipTexture(reader, palette, gl::TextureMask::Off);
             })
           | kdl::transform([&](auto texture) {
               CHECK(texture.width() == width);
               CHECK(texture.height() == height);
             });
  }) | kdl::transform_error([](const auto& e) { FAIL(e); });
}

TEST_CASE("loadHlMipTexture")
{
  using TexInfo = std::tuple<std::string, size_t, size_t>;

  // clang-format off
  const auto [textureName, width, height] = GENERATE(values<TexInfo>({
  { "bongs2",            128, 128 },
  { "blowjob_machine",   128, 128 },
  }));
  // clang-format on

  auto fs = fs::DiskFileSystem{std::filesystem::current_path()};

  auto logger = TestLogger{};

  const auto wadPath =
    std::filesystem::current_path() / "fixture/test/mdl/LoadMipTexture/hl.wad";
  auto wadFS = fs::WadFileSystem{fs::Disk::openFile(wadPath) | kdl::value()};
  REQUIRE(wadFS.reload());

  const auto file = wadFS.openFile(textureName + ".C") | kdl::value();
  auto reader = file->reader().buffer();
  const auto texture = loadHlMipTexture(reader, gl::TextureMask::Off) | kdl::value();

  CHECK(logger.countMessages(LogLevel::Error) == 0);
  CHECK(logger.countMessages(LogLevel::Warn) == 0);
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
}

} // namespace tb::mdl
