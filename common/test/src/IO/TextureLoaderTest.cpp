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

#include "IO/TextureLoader.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("TextureLoaderTest.testLoad", "[TextureLoaderTest]")
{
  const std::vector<IO::Path> paths({Path("fixture/test/IO/Wad/cr8_czg.wad")});

  const IO::Path root = IO::Disk::getCurrentWorkingDir();
  const std::vector<IO::Path> fileSearchPaths{root};
  const IO::DiskFileSystem fileSystem(root, true);

  const Model::TextureConfig textureConfig{
    Model::TextureFilePackageConfig{Model::PackageFormatConfig{{"wad"}, "idmip"}},
    Model::PackageFormatConfig{{"D"}, "idmip"},
    IO::Path{"fixture/test/palette.lmp"},
    "wad",
    IO::Path{},
    {}};

  auto logger = NullLogger();
  auto textureManager = Assets::TextureManager(0, 0, logger);

  IO::TextureLoader textureLoader(fileSystem, fileSearchPaths, textureConfig, logger);
  textureLoader.loadTextures(paths, textureManager);

  using TexInfo = std::tuple<std::string, size_t, size_t>;
  const auto expectedTextures = std::vector<TexInfo>{
    {"cr8_czg_1", 64, 64},
    {"cr8_czg_2", 64, 64},
    {"cr8_czg_3", 64, 128},
    {"cr8_czg_4", 64, 128},
    {"cr8_czg_5", 64, 128},
    {"speedM_1", 128, 128},
    {"cap4can-o-jam", 64, 64},
    {"can-o-jam", 64, 64},
    {"eat_me", 64, 64},
    {"coffin1", 128, 128},
    {"coffin2", 128, 128},
    {"czg_fronthole", 128, 128},
    {"czg_backhole", 128, 128},
    {"u_get_this", 64, 64},
    {"for_sux-m-ass", 64, 64},
    {"dex_5", 128, 128},
    {"polished_turd", 64, 64},
    {"crackpipes", 128, 128},
    {"bongs2", 128, 128},
    {"blowjob_machine", 128, 128},
    {"lasthopeofhuman", 128, 128},
  };

  CHECK(textureManager.textures().size() == expectedTextures.size());
  for (const auto& [name, width, height] : expectedTextures)
  {
    const auto* texture = textureManager.texture(name);
    CHECK(texture != nullptr);
    CHECK(texture->name() == name);
    CHECK(texture->width() == width);
    CHECK(texture->height() == height);
  }
}

TEST_CASE("TextureLoaderTest.testLoadExclusions", "[TextureLoaderTest]")
{
  const std::vector<IO::Path> paths({Path("fixture/test/IO/Wad/cr8_czg.wad")});

  const IO::Path root = IO::Disk::getCurrentWorkingDir();
  const std::vector<IO::Path> fileSearchPaths{root};
  const IO::DiskFileSystem fileSystem(root, true);

  const Model::TextureConfig textureConfig{
    Model::TextureFilePackageConfig{Model::PackageFormatConfig{{"wad"}, "idmip"}},
    Model::PackageFormatConfig{{"D"}, "idmip"},
    IO::Path{"fixture/test/palette.lmp"},
    "wad",
    IO::Path{},
    {"*-jam", "coffin2", "czg_*"}};

  auto logger = NullLogger();
  auto textureManager = Assets::TextureManager(0, 0, logger);

  IO::TextureLoader textureLoader(fileSystem, fileSearchPaths, textureConfig, logger);
  textureLoader.loadTextures(paths, textureManager);

  using TexInfo = std::tuple<std::string, size_t, size_t>;
  const auto expectedTextures = std::vector<TexInfo>{
    {"cr8_czg_1", 64, 64},
    {"cr8_czg_2", 64, 64},
    {"cr8_czg_3", 64, 128},
    {"cr8_czg_4", 64, 128},
    {"cr8_czg_5", 64, 128},
    {"speedM_1", 128, 128},
    {"eat_me", 64, 64},
    {"coffin1", 128, 128},
    {"u_get_this", 64, 64},
    {"for_sux-m-ass", 64, 64},
    {"dex_5", 128, 128},
    {"polished_turd", 64, 64},
    {"crackpipes", 128, 128},
    {"bongs2", 128, 128},
    {"blowjob_machine", 128, 128},
    {"lasthopeofhuman", 128, 128},
  };

  CHECK(textureManager.textures().size() == expectedTextures.size());
  for (const auto& [name, width, height] : expectedTextures)
  {
    const auto* texture = textureManager.texture(name);
    CHECK(texture != nullptr);
    CHECK(texture->name() == name);
    CHECK(texture->width() == width);
    CHECK(texture->height() == height);
  }
}
} // namespace IO
} // namespace TrenchBroom
