/*
 Copyright (C) 2010 Kristian Duske

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
#include "TestUtils.h"
#include "io/GameConfigParser.h"
#include "mdl/GameImpl.h"
#include "mdl/WorldNode.h"

#include <filesystem>

#include "Catch2.h"

namespace tb::mdl
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
    const auto configStr = io::readTextFile(configPath);
    auto configParser = io::GameConfigParser{configStr, configPath};
    auto config = configParser.parse();

    const auto gamePath =
      std::filesystem::current_path() / "fixture/test/mdl/Game" / gameName;
    auto game = GameImpl{config, gamePath, logger};

    auto world = game.newMap(mapFormat, vm::bbox3d{8192.0}, logger) | kdl::value();
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
    const auto configStr = io::readTextFile(configPath);
    auto configParser = io::GameConfigParser(configStr, configPath);
    auto config = configParser.parse();

    const auto gamePath =
      std::filesystem::current_path() / "fixture/test/mdl/Game/CorruptPak";
    auto logger = NullLogger();
    UNSCOPED_INFO(
      "Should not throw when loading corrupted package file for game " << game);
    CHECK_NOTHROW(GameImpl(config, gamePath, logger));
  }
}

} // namespace tb::mdl
