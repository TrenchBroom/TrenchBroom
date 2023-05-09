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
  CHECK_NOTHROW(gameFactory.initialize({{env.dir() / gamesPath}, env.dir() / userPath}));

  CHECK(gameFactory.userGameConfigsPath() == env.dir() / userPath);
  CHECK(gameFactory.gameList() == std::vector<std::string>{"Quake"});

  const auto& gameConfig = gameFactory.gameConfig("Quake");
  CHECK(gameConfig.name == "Quake");
  CHECK(gameConfig.compilationConfig.profiles.size() == 1);
  CHECK(gameConfig.gameEngineConfig.profiles.size() == 1);
}
} // namespace Model
} // namespace TrenchBroom
