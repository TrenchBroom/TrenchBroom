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

#include "IO/WalTextureReader.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"

#include "Catch2.h"
#include "TestLogger.h"

namespace TrenchBroom
{
namespace IO
{
static const auto fixturePath = Path("fixture/test/IO/Wal");

TEST_CASE("WalTextureReaderTest.testLoadQ2WalDir", "[WalTextureReaderTest]")
{
  DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
  const Assets::Palette palette =
    Assets::Palette::loadFile(fs, Path("fixture/test/colormap.pcx"));

  TextureReader::PathSuffixNameStrategy nameStrategy(fixturePath.length());
  NullLogger logger;
  WalTextureReader textureReader(nameStrategy, fs, logger, palette);

  using TexInfo = std::tuple<Path, size_t, size_t, Assets::GameData>;

  // clang-format off
  const auto 
  [path,                        width, height, gameData] = GENERATE(values<TexInfo>({
  { Path("rtz/b_pv_v1a1.wal"),  128,   256,    Assets::Q2Data{0, 0, 0} },
  { Path("rtz/b_pv_v1a2.wal"),  128,   256,    Assets::Q2Data{0, 0, 0} },
  { Path("rtz/b_pv_v1a3.wal"),  128,   128,    Assets::Q2Data{0, 0, 0} },
  { Path("rtz/b_rc_v16.wal"),   128,   128,    Assets::Q2Data{0, 0, 0} },
  { Path("rtz/b_rc_v16w.wal"),  128,   128,    Assets::Q2Data{0, 0, 0} },
  { Path("rtz/b_rc_v28.wal"),   128,    64,    Assets::Q2Data{0, 0, 0} },
  { Path("rtz/b_rc_v4.wal"),    128,   128,    Assets::Q2Data{0, 0, 0} },
  { Path("lavatest.wal"),       64,     64,    Assets::Q2Data{9, 8, 700} },
  { Path("watertest.wal"),      64,     64,    Assets::Q2Data{9, 32, 120} },
  }));
  // clang-format on

  INFO(path);
  INFO(width);
  INFO(height);

  const auto file = fs.openFile(fixturePath + path);
  REQUIRE(file != nullptr);

  const auto texture = textureReader.readTexture(file);
  const auto name = path.deleteExtension().asString("/");
  CHECK(texture.name() == name);
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
  CHECK(texture.gameData() == gameData);
}
} // namespace IO
} // namespace TrenchBroom
