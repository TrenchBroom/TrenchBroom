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

#include "Assets/EntityModel.h"
#include "Assets/Palette.h"
#include "Error.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/MdlLoader.h"
#include "IO/Reader.h"
#include "Logger.h"
#include "Model/EntityNode.h"

#include "kdl/result.h"
#include "kdl/result_io.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("MdlLoaderTest.loadValidMdl")
{
  auto logger = NullLogger{};

  const auto palettePath = "fixture/test/palette.lmp";
  auto fs = DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile("fixture/test/palette.lmp") | kdl::value();
  const auto palette = Assets::loadPalette(*paletteFile, palettePath) | kdl::value();

  const auto mdlPath = std::filesystem::current_path() / "fixture/test/IO/Mdl/armor.mdl";
  const auto mdlFile = Disk::openFile(mdlPath) | kdl::value();

  auto reader = mdlFile->reader().buffer();
  auto loader = MdlLoader("armor", reader, palette);
  auto modelData = loader.load(logger);

  CHECK(modelData.is_success());
  CHECK(modelData.value().surfaceCount() == 1u);
  CHECK(modelData.value().frameCount() == 1u);

  const auto& surfaces = modelData.value().surfaces();
  const auto& surface = surfaces.front();
  CHECK(surface.skinCount() == 3u);
  CHECK(surface.frameCount() == 1u);
}

TEST_CASE("MdlLoaderTest.loadInvalidMdl")
{
  auto logger = NullLogger{};

  const auto palettePath = "fixture/test/palette.lmp";
  auto fs = DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile("fixture/test/palette.lmp") | kdl::value();
  const auto palette = Assets::loadPalette(*paletteFile, palettePath) | kdl::value();

  const auto mdlPath =
    std::filesystem::current_path() / "fixture/test/IO/Mdl/invalid.mdl";
  const auto mdlFile = Disk::openFile(mdlPath) | kdl::value();

  auto reader = mdlFile->reader().buffer();
  auto loader = MdlLoader("armor", reader, palette);
  CHECK(
    loader.load(logger)
    == Result<Assets::EntityModelData>{Error{"Unknown MDL model version: 538976288"}});
}
} // namespace IO
} // namespace TrenchBroom
