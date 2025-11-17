/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2024 Léo Peltier

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
#include "io/BspLoader.h"
#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/Reader.h"
#include "mdl/EntityModel.h"
#include "mdl/Palette.h"

#include "kd/result.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{

TEST_CASE("BspLoaderTest.loadValidHlBsp")
{
  auto logger = NullLogger{};

  const auto palettePath = "fixture/test/palette.lmp";
  auto fs = DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile(palettePath) | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  const auto bspPath = std::filesystem::current_path() / "fixture/test/io/Bsp/hl.bsp";
  const auto bspFile = Disk::openFile(bspPath) | kdl::value();

  auto reader = bspFile->reader().buffer();
  auto loader = BspLoader("hl", reader, palette, fs);
  auto bspData = loader.load(logger);

  REQUIRE(bspData);
  CHECK(bspData.value().surfaceCount() == 1u);
  CHECK(bspData.value().frameCount() == 1u);

  const auto& surfaces = bspData.value().surfaces();
  const auto& surface = surfaces.front();
  CHECK(surface.skinCount() == 3u);
  CHECK(surface.frameCount() == 1u);
}

TEST_CASE("BspLoaderTest.loadInvalidBsp")
{
  auto logger = NullLogger{};

  const auto palettePath = "fixture/test/palette.lmp";
  auto fs = DiskFileSystem{std::filesystem::current_path()};
  auto paletteFile = fs.openFile(palettePath) | kdl::value();
  const auto palette = mdl::loadPalette(*paletteFile, palettePath) | kdl::value();

  const auto bspPath =
    std::filesystem::current_path() / "fixture/test/io/Bsp/invalid_version.bsp";
  const auto bspFile = Disk::openFile(bspPath) | kdl::value();

  auto reader = bspFile->reader().buffer();
  auto loader = BspLoader("invalid_version", reader, palette, fs);
  CHECK(
    loader.load(logger)
    == Result<mdl::EntityModelData>{Error{"Unsupported BSP model version: 63"}});
}

} // namespace tb::io
