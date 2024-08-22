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

#include "Assets/Material.h"
#include "Assets/MaterialCollection.h"
#include "Assets/Palette.h"
#include "Error.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/MaterialUtils.h"
#include "IO/ReadMipTexture.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"
#include "TestLogger.h"

#include "kdl/result.h"

#include <filesystem>
#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{

TEST_CASE("readIdMipTexture")
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

  const auto palettePath = "fixture/test/palette.lmp";
  auto fs = DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile("fixture/test/palette.lmp") | kdl::value();
  const auto palette = Assets::loadPalette(*paletteFile, palettePath) | kdl::value();

  auto logger = NullLogger{};

  const auto wadPath =
    std::filesystem::current_path() / "fixture/test/IO/Wad/cr8_czg.wad";
  auto wadFS = WadFileSystem{Disk::openFile(wadPath) | kdl::value()};
  REQUIRE(wadFS.reload().is_success());

  const auto file = wadFS.openFile(textureName + ".D") | kdl::value();
  auto reader = file->reader().buffer();
  const auto texture =
    readIdMipTexture(reader, palette, Assets::TextureMask::Off) | kdl::value();

  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
}

TEST_CASE("readHlMipTexture")
{
  using TexInfo = std::tuple<std::string, size_t, size_t>;

  // clang-format off
  const auto [textureName, width, height] = GENERATE(values<TexInfo>({
  { "bongs2",            128, 128 },
  { "blowjob_machine",   128, 128 },
  }));
  // clang-format on

  auto fs = DiskFileSystem{std::filesystem::current_path()};

  auto logger = TestLogger{};

  const auto wadPath = std::filesystem::current_path() / "fixture/test/IO/HL/hl.wad";
  auto wadFS = WadFileSystem{Disk::openFile(wadPath) | kdl::value()};
  REQUIRE(wadFS.reload().is_success());

  const auto file = wadFS.openFile(textureName + ".C") | kdl::value();
  auto reader = file->reader().buffer();
  const auto texture = readHlMipTexture(reader, Assets::TextureMask::Off) | kdl::value();

  CHECK(logger.countMessages(LogLevel::Error) == 0);
  CHECK(logger.countMessages(LogLevel::Warn) == 0);
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
}

} // namespace TrenchBroom::IO
