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

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("AseParserTest.loadWithoutException")
{
  auto logger = NullLogger{};

  const auto defaultAssetsPath =
    std::filesystem::current_path() / "fixture/test/IO/ResourceUtils/assets";
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    std::filesystem::current_path() / "fixture/test/IO/Ase/wedge_with_shader";
  fs.mount("", std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = "scripts";
  const auto textureSearchPaths = std::vector<std::filesystem::path>{"models"};
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  const auto aseFile = fs.openFile("models/mapobjects/wedges/wedge_45.ase").value();
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"wedge", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());
}

TEST_CASE("AseParserTest.fallbackToMaterialName")
{
  auto logger = NullLogger{};

  const auto defaultAssetsPath =
    std::filesystem::current_path() / "fixture/test/IO/ResourceUtils/assets";
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    std::filesystem::current_path() / "fixture/test/IO/Ase/fallback_to_materialname";
  fs.mount("", std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = std::filesystem::path{"scripts"};
  const auto textureSearchPaths = std::vector<std::filesystem::path>{"textures"};
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  const auto aseFile = fs.openFile("models/wedge_45.ase").value();
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"wedge", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());

  // account for the default texture
  CHECK(model->surface(0).skinCount() == 2u);
  CHECK(model->surface(0).skin(0)->name() == "textures/bigtile");
}

TEST_CASE("AseParserTest.loadDefaultMaterial")
{
  auto logger = NullLogger{};

  const auto defaultAssetsPath =
    std::filesystem::current_path() / "fixture/test/IO/ResourceUtils/assets";
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(defaultAssetsPath));

  const auto basePath =
    std::filesystem::current_path() / "fixture/test/IO/Ase/load_default_material";
  fs.mount("", std::make_unique<DiskFileSystem>(basePath));

  const auto shaderSearchPath = std::filesystem::path{"scripts"};
  const auto textureSearchPaths = std::vector<std::filesystem::path>{"textures"};
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  const auto aseFile = fs.openFile("models/wedge_45.ase").value();
  auto reader = aseFile->reader().buffer();
  auto parser = AseParser{"wedge", reader.stringView(), fs};

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());

  // account for the default texture
  CHECK(model->surface(0).skinCount() == 2u);
  // shader name is correct, but we loaded the default material

  const auto* texture = model->surface(0).skin(0);
  CHECK(texture->name() == "textures/bigtile");
  CHECK(texture->width() == 32u);
  CHECK(texture->height() == 32u);
}
} // namespace IO
} // namespace TrenchBroom
