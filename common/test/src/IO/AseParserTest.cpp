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

#include "IO/AseParser.h"
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/Reader.h"
#include "IO/TextureReader.h"
#include "Logger.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("AseParserTest.loadWithoutException", "[AseParserTest]")
{
  NullLogger logger;

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
  std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/wedge_with_shader");
  fs = std::make_shared<DiskFileSystem>(fs, basePath);

  const auto shaderSearchPath = Path("scripts");
  const auto textureSearchPaths = std::vector<Path>{Path("models")};
  fs = std::make_shared<Quake3ShaderFileSystem>(
    fs, shaderSearchPath, textureSearchPaths, logger);

  const auto aseFile = fs->openFile(Path("models/mapobjects/wedges/wedge_45.ase"));
  auto reader = aseFile->reader().buffer();
  AseParser parser("wedge", reader.stringView(), *fs);

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());
}

TEST_CASE("AseParserTest.fallbackToMaterialName", "[AseParserTest]")
{
  NullLogger logger;

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
  std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/fallback_to_materialname");
  fs = std::make_shared<DiskFileSystem>(fs, basePath);

  const auto shaderSearchPath = Path("scripts");
  const auto textureSearchPaths = std::vector<Path>{Path("textures")};
  fs = std::make_shared<Quake3ShaderFileSystem>(
    fs, shaderSearchPath, textureSearchPaths, logger);

  const auto aseFile = fs->openFile(Path("models/wedge_45.ase"));
  auto reader = aseFile->reader().buffer();
  AseParser parser("wedge", reader.stringView(), *fs);

  auto model = parser.initializeModel(logger);
  CHECK(model != nullptr);

  CHECK_NOTHROW(parser.loadFrame(0, *model, logger));
  CHECK(model->frame(0)->loaded());

  // account for the default texture
  CHECK(model->surface(0).skinCount() == 2u);
  CHECK(model->surface(0).skin(0)->name() == "textures/bigtile");
}

TEST_CASE("AseParserTest.loadDefaultMaterial", "[AseParserTest]")
{
  NullLogger logger;

  const auto defaultAssetsPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/ResourceUtils/assets");
  std::shared_ptr<FileSystem> fs = std::make_shared<DiskFileSystem>(defaultAssetsPath);

  const auto basePath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Ase/load_default_material");
  fs = std::make_shared<DiskFileSystem>(fs, basePath);

  const auto shaderSearchPath = Path("scripts");
  const auto textureSearchPaths = std::vector<Path>{Path("textures")};
  fs = std::make_shared<Quake3ShaderFileSystem>(
    fs, shaderSearchPath, textureSearchPaths, logger);

  const auto aseFile = fs->openFile(Path("models/wedge_45.ase"));
  auto reader = aseFile->reader().buffer();
  AseParser parser("wedge", reader.stringView(), *fs);

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
