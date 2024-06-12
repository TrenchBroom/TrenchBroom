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

#include "Assets/Material.h"
#include "Assets/MaterialCollection.h"
#include "Assets/MaterialManager.h"
#include "IO/DiskIO.h"
#include "IO/GameConfigParser.h"
#include "Logger.h"
#include "Model/EntityNode.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"

#include "kdl/vector_utils.h"

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom::Model
{
TEST_CASE("GameTest.newMap")
{
  auto logger = NullLogger();

  SECTION("Creates correct worldspawn properties for new maps")
  {
    using T = std::tuple<std::string, MapFormat, std::vector<EntityProperty>>;
    const auto [gameName, mapFormat, expectedProperties] = GENERATE(values<T>({
      {"Quake",
       MapFormat::Valve,
       {
         {"classname", "worldspawn"},
         {"wad", ""},
         {"mapversion", "220"},
       }},
      {"Quake3",
       MapFormat::Quake3_Legacy,
       {
         {"classname", "worldspawn"},
       }},
      {"Quake3",
       MapFormat::Quake3_Valve,
       {
         {"classname", "worldspawn"},
         {"mapversion", "220"},
       }},
    }));

    CAPTURE(gameName, mapFormat);

    const auto configPath =
      std::filesystem::current_path() / "fixture/games" / gameName / "GameConfig.cfg";
    const auto configStr = IO::readTextFile(configPath);
    auto configParser = IO::GameConfigParser{configStr, configPath};
    auto config = configParser.parse();

    const auto gamePath =
      std::filesystem::current_path() / "fixture/test/Model/Game" / gameName;
    auto game = GameImpl{config, gamePath, logger};

    auto world = game.newMap(mapFormat, vm::bbox3{8192.0}, logger) | kdl::value();
    CHECK_THAT(
      world->entity().properties(), Catch::Matchers::UnorderedEquals(expectedProperties));
  }
}

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

// TEST_CASE("GameTest.loadQuake3Shaders")
// {
//   const auto configPath =
//     std::filesystem::current_path() / "fixture/games/Quake3/GameConfig.cfg";
//   const auto configStr = IO::readTextFile(configPath);
//   auto configParser = IO::GameConfigParser{configStr, configPath};
//   auto config = configParser.parse();

//   const auto gamePath =
//     std::filesystem::current_path() / "fixture/test/Model/Game/Quake3";
//   auto logger = NullLogger{};
//   auto game = GameImpl{config, gamePath, logger};

//   auto worldspawn = Entity{};

//   auto materialManager = Assets::MaterialManager{0, 0, logger};
//   game.loadMaterialCollections(materialManager);

//   /*
//    * The shader script contains five entries:
//    * - textures/test/test overrides an existing texture and points it to an editor
//    image
//    * - textures/test/not_existing does not override an existing texture and points to
//    an
//    *   editor image
//    * - textures/test/test2 overrides an existing texture, but the editor image is
//    missing
//    * - textures/test/not_existing2 does not override an existing texture, and no editor
//    *   image
//    * - textures/skies/hub1/dusk has a deeper directory structure, and has an editor
//    image
//    *
//    * Due to the directory structure, the shader script induces four material
//    collections:
//    * - textures
//    * - textures/test
//    * - textures/skies
//    * - textures/skies/hub1
//    *
//    * The file system contains three textures:
//    * - textures/test/test.tga is overridden by the shader script
//    * - textures/test/test2.tga is overridden by the shader script
//    * - textures/test/editor_image.jpg is not overridden by a shader
//    *
//    * In total, we expect the following entries in material collection textures/test:
//    * - test/test -> test/editor_image.jpg
//    * - test/not_existing -> test/editor_image.jpg
//    * - test/editor_image
//    * - test/not_existing2 -> __TB_empty.png
//    * - test/test2 -> __TB_empty.png
//    *
//    * and one entry in material collection textures/skies/hub1:
//    * - skies/hub1/dusk -> test/editor_image.jpg
//    */

//   const auto& materialCollections = materialManager.collections();
//   CHECK(materialCollections.size() == 4);

//   const auto skiesCollection = std::find_if(
//     materialCollections.begin(), materialCollections.end(), [](const auto& c) {
//       return c.path() == "textures/skies/hub1";
//     });

//   CHECK(skiesCollection != materialCollections.end());

//   const auto skiesMaterialNames = kdl::vec_transform(
//     skiesCollection->materials(), [](const auto& material) { return material.name();
//     });

//   CHECK_THAT(
//     skiesMaterialNames,
//     Catch::UnorderedEquals(std::vector<std::string>{
//       "skies/hub1/dusk",
//     }));

//   const auto testCollection = std::find_if(
//     materialCollections.begin(), materialCollections.end(), [](const auto& c) {
//       return c.path() == "textures/test";
//     });

//   CHECK(testCollection != materialCollections.end());

//   const auto testMaterialNames = kdl::vec_transform(
//     testCollection->materials(), [](const auto& material) { return material.name(); });

//   CHECK_THAT(
//     testMaterialNames,
//     Catch::UnorderedEquals(std::vector<std::string>{
//       "test/test",
//       "test/not_existing",
//       "test/editor_image",
//       "test/not_existing2",
//       "test/test2",
//     }));
// }
} // namespace TrenchBroom::Model
