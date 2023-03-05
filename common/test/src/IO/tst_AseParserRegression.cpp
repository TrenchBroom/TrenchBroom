/*
 Copyright (C) 2018 Eric Wasylishen

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

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/AseParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/Reader.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("AseParserTest.parseFailure_2657")
{
  auto logger = NullLogger{};
  auto fs = VirtualFileSystem{};

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path{"fixture/test/IO/ResourceUtils/assets"};
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path{"fixture/test/IO/Ase/steelstorm_player"};
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = Path{"scripts"};
  const auto textureSearchPaths = std::vector<Path>{Path{"models"}};
  fs.mount(
    Path{},
    std::make_unique<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger));

  const auto aseFile = fs.openFile(Path{"player.ase"});
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"player", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());
}

TEST_CASE("AseParserTest.parseFailure_2679")
{
  auto logger = NullLogger{};
  auto fs = VirtualFileSystem{};

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/no_scene_directive");
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = Path{"scripts"};
  const auto textureSearchPaths = std::vector<Path>{Path{"models"}};
  fs.mount(
    Path{},
    std::make_unique<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger));

  const auto aseFile = fs.openFile(Path("wedge_45.ase"));
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"wedge", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());
}

TEST_CASE("AseParserTest.parseFailure_2898_vertex_index")
{
  auto logger = NullLogger{};
  auto fs = VirtualFileSystem{};

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/index_out_of_bounds");
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = Path{"scripts"};
  const auto textureSearchPaths = std::vector<Path>{Path{"models"}};
  fs.mount(
    Path{},
    std::make_unique<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger));

  const auto aseFile = fs.openFile(Path("wedge_45.ase"));
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"wedge", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());
}

TEST_CASE("AseParserTest.parseFailure_2898_no_uv")
{
  auto logger = NullLogger{};
  auto fs = VirtualFileSystem{};

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/index_out_of_bounds");
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = Path{"scripts"};
  const auto textureSearchPaths = std::vector<Path>{Path{"models"}};
  fs.mount(
    Path{},
    std::make_unique<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger));

  const auto aseFile = fs.openFile(Path("wedge_45_no_uv.ase"));
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"wedge", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());
}
} // namespace IO
} // namespace TrenchBroom
