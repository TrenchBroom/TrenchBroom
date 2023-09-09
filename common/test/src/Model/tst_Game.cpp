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

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "IO/DiskIO.h"
#include "IO/GameConfigParser.h"
#include "Logger.h"
#include "Model/EntityNode.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"
#include "TestUtils.h"

#include <kdl/vector_utils.h>

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom::Model
{
TEST_CASE("GameTest.loadCorruptPackages")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/2496

  const auto games = std::vector<std::filesystem::path>{
    "Quake",
    "Daikatana",
    "Quake3",
  };

  for (const auto& game : games)
  {
    const auto configPath =
      std::filesystem::current_path() / "fixture/games/" / game / "GameConfig.cfg";
    const auto configStr = IO::readTextFile(configPath);
    auto configParser = IO::GameConfigParser(configStr, configPath);
    auto config = configParser.parse();

    const auto gamePath =
      std::filesystem::current_path() / "fixture/test/Model/Game/CorruptPak";
    auto logger = NullLogger();
    UNSCOPED_INFO(
      "Should not throw when loading corrupted package file for game " << game);
    CHECK_NOTHROW(GameImpl(config, gamePath, logger));
  }
}

TEST_CASE("GameTest.loadQuake3Shaders")
{
  const auto configPath =
    std::filesystem::current_path() / "fixture/games/Quake3/GameConfig.cfg";
  const auto configStr = IO::readTextFile(configPath);
  auto configParser = IO::GameConfigParser{configStr, configPath};
  auto config = configParser.parse();

  const auto gamePath =
    std::filesystem::current_path() / "fixture/test/Model/Game/Quake3";
  auto logger = NullLogger{};
  auto game = GameImpl{config, gamePath, logger};

  auto worldspawn = Entity{};

  auto textureManager = Assets::TextureManager{0, 0, logger};
  game.loadTextureCollections(textureManager);

  /*
   * The shader script contains five entries:
   * - textures/test/test overrides an existing texture and points it to an editor image
   * - textures/test/not_existing does not override an existing texture and points to an
   *   editor image
   * - textures/test/test2 overrides an existing texture, but the editor image is missing
   * - textures/test/not_existing2 does not override an existing texture, and no editor
   *   image
   * - textures/skies/hub1/dusk has a deeper directory structure, and has an editor image
   *
   * Due to the directory structure, the shader script induces four texture collections:
   * - textures
   * - textures/test
   * - textures/skies
   * - textures/skies/hub1
   *
   * The file system contains three textures:
   * - textures/test/test.tga is overridden by the shader script
   * - textures/test/test2.tga is overridden by the shader script
   * - textures/test/editor_image.jpg is not overridden by a shader
   *
   * In total, we expect the following entries in texture collection textures/test:
   * - test/test -> test/editor_image.jpg
   * - test/not_existing -> test/editor_image.jpg
   * - test/editor_image
   * - test/not_existing2 -> __TB_empty.png
   * - test/test2 -> __TB_empty.png
   *
   * and one entry in texture collection textures/skies/hub1:
   * - skies/hub1/dusk -> test/editor_image.jpg
   */

  const auto& textureCollections = textureManager.collections();
  CHECK(textureCollections.size() == 4);

  const auto skiesCollection =
    std::find_if(textureCollections.begin(), textureCollections.end(), [](const auto& c) {
      return c.path() == "textures/skies/hub1";
    });

  CHECK(skiesCollection != textureCollections.end());

  const auto skiesTextureNames = kdl::vec_transform(
    skiesCollection->textures(), [](const auto& texture) { return texture.name(); });

  CHECK_THAT(
    skiesTextureNames,
    Catch::UnorderedEquals(std::vector<std::string>{
      "skies/hub1/dusk",
    }));

  const auto testCollection =
    std::find_if(textureCollections.begin(), textureCollections.end(), [](const auto& c) {
      return c.path() == "textures/test";
    });

  CHECK(testCollection != textureCollections.end());

  const auto testTextureNames = kdl::vec_transform(
    testCollection->textures(), [](const auto& texture) { return texture.name(); });

  CHECK_THAT(
    testTextureNames,
    Catch::UnorderedEquals(std::vector<std::string>{
      "test/test",
      "test/not_existing",
      "test/editor_image",
      "test/not_existing2",
      "test/test2",
    }));
}
} // namespace TrenchBroom::Model
