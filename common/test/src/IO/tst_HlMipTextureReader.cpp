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
#include "IO/HlMipTextureReader.h"
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
TEST_CASE("HlMipTextureReaderTest.testLoadWad")
{
  using TexInfo = std::tuple<std::string, size_t, size_t>;

  // clang-format off
  const auto [textureName, width, height] = GENERATE(values<TexInfo>({
  { "bongs2",            128, 128 },
  { "blowjob_machine",   128, 128 },
  }));
  // clang-format on

  auto fs = DiskFileSystem{IO::Disk::getCurrentWorkingDir()};

  auto nameStrategy = TextureReader::TextureNameStrategy{};
  auto logger = TestLogger{};
  auto textureLoader = HlMipTextureReader{nameStrategy, fs, logger};

  const auto wadPath = Disk::getCurrentWorkingDir() + Path{"fixture/test/IO/HL/hl.wad"};
  auto wadFS = WadFileSystem{wadPath};

  const auto texture =
    textureLoader.readTexture(wadFS.openFile(Path{textureName}.addExtension("C")));
  CHECK(logger.countMessages(LogLevel::Error) == 0);
  CHECK(logger.countMessages(LogLevel::Warn) == 0);
  CHECK(texture.name() == textureName);
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
}
} // namespace IO
} // namespace TrenchBroom
