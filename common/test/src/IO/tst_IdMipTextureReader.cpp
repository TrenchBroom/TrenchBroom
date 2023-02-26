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

#include "TestLogger.h"

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/IdMipTextureReader.h"
#include "IO/Path.h"
#include "IO/TextureReader.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("IdMipTextureReaderTest.testLoadWad")
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

  auto fs = DiskFileSystem{IO::Disk::getCurrentWorkingDir()};
  auto paletteFile = fs.openFile(Path{"fixture/test/palette.lmp"});
  const auto palette = Assets::loadPalette(*paletteFile);

  auto nameStrategy = TextureReader::TextureNameStrategy{};
  auto logger = NullLogger{};
  auto textureLoader = IdMipTextureReader{nameStrategy, fs, palette, logger};

  const auto wadPath =
    Disk::getCurrentWorkingDir() + Path{"fixture/test/IO/Wad/cr8_czg.wad"};
  auto wadFS = WadFileSystem{wadPath};

  const auto texture =
    textureLoader.readTexture(wadFS.openFile(Path{textureName}.addExtension("D")));
  CHECK(texture.name() == textureName);
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
}
} // namespace IO
} // namespace TrenchBroom
