/*
 Copyright (C) 2021 Kristian Duske

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
#include "io/TestEnvironment.h"
#include "mdl/GameConfig.h"
#include "mdl/GameFactory.h"

#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <filesystem>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{
const auto gamesPath = std::filesystem::path{"games"};
const auto userPath = std::filesystem::path{"user"};

void writeGameConfig(
  io::TestEnvironment& env, const std::string& gameDirectory, const std::string& gameName)
{
  env.createDirectory(gamesPath / gameDirectory);
  env.createFile(
    gamesPath / gameDirectory / "GameConfig.cfg",
    fmt::format(
      R"({{
    "version": 9,
    "name": "{}",
    "icon": "Icon.png",
    "fileformats": [
        {{ "format": "Valve" }}
    ],
    "filesystem": {{
        "searchpath": "id1",
        "packageformat": {{ "extension": "pak", "format": "idpak" }}
    }},
    "materials": {{
        "root": "textures",
        "extensions": [".D"],
        "palette": "gfx/palette.lmp",
        "attribute": "wad"
    }},
    "entities": {{
        "definitions": [],
        "defaultcolor": "0.6 0.6 0.6 1.0",
        "modelformats": [ "mdl" ]
    }},
    "tags": {{
        "brush": [],
        "brushface": []
    }}
}})",
      gameName));
}

void writeCompilationProfile(io::TestEnvironment& env, const std::string& directory)
{
  env.createDirectory(userPath / directory);
  env.createFile(userPath / directory / "CompilationProfiles.cfg", R"({
    "profiles": [
        {
            "name": "Full Compile",
            "tasks": [
                {
                    "target": "${WORK_DIR_PATH}/${MAP_BASE_NAME}-compile.map",
                    "type": "export"
                }
            ],
            "workdir": "${MAP_DIR_PATH}"
        }
    ],
    "version": 1
})");
}

void writeGameEngineProfile(io::TestEnvironment& env, const std::string& directory)
{
  env.createDirectory(userPath / directory);
  env.createFile(userPath / directory / "GameEngineProfiles.cfg", R"({
    "profiles": [
        {
            "name": "QuakeSpasm",
            "parameters": "+map ${MAP_BASE_NAME}",
            "path": "/Applications/Quake/QuakeSpasm.app"
        }
    ],
    "version": 1
})");
}

void setupTestEnvironment(io::TestEnvironment& env)
{
  env.createDirectory(gamesPath);
  env.createDirectory(userPath);

  writeGameConfig(env, "Quake", "Quake");
  writeCompilationProfile(env, "Quake");
  writeGameEngineProfile(env, "Quake");

  // This config will fail to parse and should be ignored
  env.createDirectory(gamesPath / "Quake 2");
  env.createFile(gamesPath / "Quake 2" / "GameConfig.cfg", R"({
    asdf
})");

  env.createDirectory(gamesPath / "Quake 3");
  writeGameConfig(env, "Quake 3", "Quake 3");

  // This config will fail to parse and should be ignored
  env.createDirectory(userPath / "Quake 3");
  env.createFile(userPath / "Quake 3" / "CompilationProfiles.cfg", R"({
    asdf
})");

  // This config will fail to parse and should be ignored
  env.createFile(userPath / "Quake 3" / "GameEngineProfiles.cfg", R"({
    asdf
})");

  writeGameConfig(env, "Daikatana", "Daikatana");

  // Successful migration
  writeGameConfig(env, "Migrate1", "Migrate 1");
  writeCompilationProfile(env, "Migrate 1");

  // Already migrated
  writeGameConfig(env, "Migrate2", "Migrate 2");
  writeCompilationProfile(env, "Migrate2");

  // Migration blocked
  writeGameConfig(env, "Migrate3", "Migrate 3");
  writeCompilationProfile(env, "Migrate 3");
  writeGameEngineProfile(env, "Migrate3");
}

} // namespace

TEST_CASE("GameFactory")
{
  auto env = io::TestEnvironment{setupTestEnvironment};
  auto& gameFactory = GameFactory::instance();
  gameFactory.reset();

  SECTION("initialize")
  {
    REQUIRE(gameFactory.initialize({{env.dir() / gamesPath}, env.dir() / userPath}));

    CHECK(gameFactory.userGameConfigsPath() == env.dir() / userPath);
    CHECK(
      gameFactory.gameList()
      == std::vector<std::string>{
        "Daikatana",
        "Migrate 1",
        "Migrate 2",
        "Migrate 3",
        "Quake",
        "Quake 3",
      });

    const auto& quakeConfig = gameFactory.gameConfig("Quake");
    CHECK(quakeConfig.name == "Quake");
    CHECK(quakeConfig.compilationConfig.profiles.size() == 1);
    CHECK(quakeConfig.gameEngineConfig.profiles.size() == 1);

    const auto& quake3Config = gameFactory.gameConfig("Quake 3");
    CHECK(quake3Config.name == "Quake 3");
    CHECK(quake3Config.compilationConfig.profiles.empty());
    CHECK(quake3Config.gameEngineConfig.profiles.empty());

    const auto& quake4Config = gameFactory.gameConfig("Migrate 1");
    CHECK(quake4Config.name == "Migrate 1");
    CHECK(quake4Config.compilationConfig.profiles.size() == 1);
    CHECK(quake4Config.gameEngineConfig.profiles.empty());

    CHECK(!env.directoryExists(userPath / "Migrate 1"));
    CHECK(env.fileExists(userPath / "Migrate1" / "CompilationProfiles.cfg"));

    CHECK(!env.directoryExists(userPath / "Migrate 2"));
    CHECK(env.fileExists(userPath / "Migrate2" / "CompilationProfiles.cfg"));

    CHECK(env.fileExists(userPath / "Migrate 3" / "CompilationProfiles.cfg"));
    CHECK(env.fileExists(userPath / "Migrate3" / "GameEngineProfiles.cfg"));
  }

  SECTION("saveCompilationConfig")
  {
    REQUIRE(gameFactory.initialize({{env.dir() / gamesPath}, env.dir() / userPath}));

    REQUIRE(kdl::vec_contains(gameFactory.gameList(), "Daikatana"));

    auto logger = NullLogger{};
    gameFactory.saveCompilationConfig("Daikatana", {{{"name", "workDir", {}}}}, logger);
    CHECK(env.fileExists(userPath / "Daikatana/CompilationProfiles.cfg"));
  }

  SECTION("saveGameEngineConfig")
  {
    REQUIRE(gameFactory.initialize({{env.dir() / gamesPath}, env.dir() / userPath}));

    REQUIRE(kdl::vec_contains(gameFactory.gameList(), "Daikatana"));

    auto logger = NullLogger{};
    gameFactory.saveGameEngineConfig(
      "Daikatana", {{{"name", "path", "parameters"}}}, logger);
    CHECK(env.fileExists(userPath / "Daikatana/GameEngineProfiles.cfg"));
  }
}

} // namespace tb::mdl
