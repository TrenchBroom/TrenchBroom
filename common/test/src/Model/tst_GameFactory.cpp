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

#include "IO/TestEnvironment.h"
#include "Model/GameConfig.h"
#include "Model/GameError.h"
#include "Model/GameFactory.h"

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{

static const auto gamesPath = std::filesystem::path{"games"};
static const auto userPath = std::filesystem::path{"user"};

static void setupTestEnvironment(IO::TestEnvironment& env)
{
  env.createDirectory(gamesPath);
  env.createDirectory(gamesPath / "Quake");
  env.createFile(gamesPath / "Quake/GameConfig.cfg", R"({
    "version": 8,
    "name": "Quake",
    "icon": "Icon.png",
    "fileformats": [
        { "format": "Valve" }
    ],
    "filesystem": {
        "searchpath": "id1",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "textures": {
        "root": "textures",
        "extensions": [".D"],
        "palette": "gfx/palette.lmp",
        "attribute": "wad"
    },
    "entities": {
        "definitions": [],
        "defaultcolor": "0.6 0.6 0.6 1.0",
        "modelformats": [ "mdl" ]
    },
    "tags": {
        "brush": [],
        "brushface": []
    }
})");

  env.createDirectory(userPath);
  env.createDirectory(userPath / "Quake");
  env.createFile(userPath / "Quake/CompilationProfiles.cfg", R"({
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

  env.createFile(userPath / "Quake/GameEngineProfiles.cfg", R"({
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

TEST_CASE("GameFactory.initialize")
{
  const auto env = IO::TestEnvironment{setupTestEnvironment};

  auto& gameFactory = GameFactory::instance();
  CHECK(
    gameFactory.initialize({{env.dir() / gamesPath}, env.dir() / userPath}).is_success());

  CHECK(gameFactory.userGameConfigsPath() == env.dir() / userPath);
  CHECK(gameFactory.gameList() == std::vector<std::string>{"Quake"});

  const auto& gameConfig = gameFactory.gameConfig("Quake");
  CHECK(gameConfig.name == "Quake");
  CHECK(gameConfig.compilationConfig.profiles.size() == 1);
  CHECK(gameConfig.gameEngineConfig.profiles.size() == 1);
}

TEST_CASE("GameFactory.detectGame")
{
  using namespace std::string_literals;

  auto env = IO::TestEnvironment{setupTestEnvironment};

  auto& gameFactory = GameFactory::instance();
  REQUIRE_NOTHROW(
    gameFactory.initialize({{env.dir() / gamesPath}, env.dir() / userPath}));

  const auto detectGame = [&](const auto& mapFile) {
    return env.withTempFile(
      mapFile, [&](const auto& path) { return gameFactory.detectGame(path); });
  };

  CHECK(detectGame(R"(// Game: Quake
// Format: Quake2
)") == std::pair{"Quake"s, MapFormat::Quake2});


  CHECK(detectGame(R"(// Game: Quake
// Format: Quake2
{
"classname" "worldspawn"
{
( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) attribsExplicit 56 -32 0 1 1 8 9 700
( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) attribsOmitted 32 32 0 1 1
( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) attribsExplicitlyZero 16 96 0 1 1 0 0 0
( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0
( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1 0 0 0
}
})") == std::pair{"Quake"s, MapFormat::Quake2});
}
} // namespace Model
} // namespace TrenchBroom
