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
#include "fs/TestEnvironment.h"
#include "mdl/GameManager.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
namespace
{

const auto gamesPath = std::filesystem::path{"games"};
const auto userPath = std::filesystem::path{"user"};

void writeGameConfig(
  fs::TestEnvironment& env, const std::string& gameDirectory, const std::string& gameName)
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

void writeCompilationProfile(fs::TestEnvironment& env, const std::string& directory)
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

void writeGameEngineProfile(fs::TestEnvironment& env, const std::string& directory)
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

} // namespace

TEST_CASE("GameManager")
{
  using namespace Catch::Matchers;

  auto logger = NullLogger{};

  SECTION("create")
  {
    SECTION("loads existing game configs")
    {
      auto env = fs::TestEnvironment{};

      env.createDirectory(gamesPath);
      env.createDirectory(userPath);

      writeGameConfig(env, "Quake", "Quake");
      writeCompilationProfile(env, "Quake");
      writeGameEngineProfile(env, "Quake");

      const auto gameConfigSearchDirs = std::vector{env.dir() / gamesPath};
      const auto userGameDir = env.dir() / userPath;
      const auto expectedGameConfigPath =
        gameConfigSearchDirs.front() / "Quake" / "GameConfig.cfg";

      initializeGameManager(gameConfigSearchDirs, userGameDir)
        | kdl::transform([&](const auto& gameManager, const auto&) {
            const auto& gameInfos = gameManager.gameInfos();
            REQUIRE(gameInfos.size() == 1);

            const auto& gameInfo = gameInfos.front();
            CHECK(gameInfo.gameConfig.name == "Quake");

            CHECK(gameInfo.gamePathPreference.path == "Games/Quake/Path");
            CHECK(
              gameInfo.defaultEnginePathPreference.path == "Games/Quake/Default Engine");

            CHECK(!gameInfo.compilationConfigParseFailed);
            CHECK(gameInfo.compilationConfig.profiles.size() == 1);

            CHECK(!gameInfo.gameEngineConfigParseFailed);
            CHECK(gameInfo.gameEngineConfig.profiles.size() == 1);
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }

    SECTION("skips game configs with parse errors")
    {
      auto env = fs::TestEnvironment{};

      env.createDirectory(gamesPath);
      env.createDirectory(userPath);

      writeGameConfig(env, "Quake", "Quake");
      writeCompilationProfile(env, "Quake");
      writeGameEngineProfile(env, "Quake");

      // This config will fail to parse and should be ignored
      env.createDirectory(gamesPath / "Quake 2");
      env.createFile(gamesPath / "Quake 2" / "GameConfig.cfg", "{asdf}");

      const auto gameConfigSearchDirs = std::vector{env.dir() / gamesPath};
      const auto userGameDir = env.dir() / userPath;
      const auto expectedGameConfigPath =
        gameConfigSearchDirs.front() / "Quake" / "GameConfig.cfg";

      initializeGameManager(gameConfigSearchDirs, userGameDir)
        | kdl::transform([&](const auto& gameManager, const auto&) {
            const auto& gameInfos = gameManager.gameInfos();
            REQUIRE(gameInfos.size() == 1);

            const auto& gameInfo = gameInfos.front();
            CHECK(gameInfo.gameConfig.name == "Quake");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }

    SECTION("skips compilation and engine configs with parse errors")
    {
      auto env = fs::TestEnvironment{};

      env.createDirectory(gamesPath);
      env.createDirectory(userPath);

      env.createDirectory(gamesPath / "Quake 3");
      writeGameConfig(env, "Quake 3", "Quake 3");

      // This config will fail to parse and should be ignored
      env.createDirectory(userPath / "Quake 3");
      env.createFile(userPath / "Quake 3" / "CompilationProfiles.cfg", "{asdf}");

      // This config will fail to parse and should be ignored
      env.createFile(userPath / "Quake 3" / "GameEngineProfiles.cfg", "{asdf}");

      const auto gameConfigSearchDirs = std::vector{env.dir() / gamesPath};
      const auto userGameDir = env.dir() / userPath;
      const auto expectedGameConfigPath =
        gameConfigSearchDirs.front() / "Quake" / "GameConfig.cfg";

      initializeGameManager(gameConfigSearchDirs, userGameDir)
        | kdl::transform([&](const auto& gameManager, const auto&) {
            const auto& gameInfos = gameManager.gameInfos();
            REQUIRE(gameInfos.size() == 1);

            const auto& gameInfo = gameInfos.front();
            CHECK(gameInfo.gameConfig.name == "Quake 3");

            CHECK(gameInfo.compilationConfigParseFailed);
            CHECK(gameInfo.compilationConfig.profiles.empty());

            CHECK(gameInfo.gameEngineConfigParseFailed);
            CHECK(gameInfo.gameEngineConfig.profiles.empty());
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }

    SECTION("migrates configuration locations")
    {
      auto env = fs::TestEnvironment{};

      env.createDirectory(gamesPath);
      env.createDirectory(userPath);

      // Successful migration
      writeGameConfig(env, "Migrate1", "Migrate 1");
      writeCompilationProfile(env, "Migrate 1");
      writeGameEngineProfile(env, "Migrate 1");

      // Already migrated
      writeGameConfig(env, "Migrate2", "Migrate 2");
      writeCompilationProfile(env, "Migrate2");

      // Migration blocked
      writeGameConfig(env, "Migrate3", "Migrate 3");
      writeCompilationProfile(env, "Migrate 3");
      writeGameEngineProfile(env, "Migrate3");

      const auto gameConfigSearchDirs = std::vector{env.dir() / gamesPath};
      const auto userGameDir = env.dir() / userPath;
      const auto expectedGameConfigPath =
        gameConfigSearchDirs.front() / "Quake" / "GameConfig.cfg";

      initializeGameManager(gameConfigSearchDirs, userGameDir)
        | kdl::transform([&](const auto& gameManager, const auto&) {
            const auto& gameInfos = gameManager.gameInfos();
            CHECK_THAT(
              gameInfos | std::views::transform([](const auto& gameInfo) {
                return gameInfo.gameConfig.name;
              }),
              RangeEquals(std::vector{"Migrate 1", "Migrate 2", "Migrate 3"}));

            const auto& migrate1 = gameInfos[0];
            CHECK(migrate1.compilationConfig.profiles.size() == 1);
            CHECK(migrate1.gameEngineConfig.profiles.size() == 1);

            const auto& migrate2 = gameInfos[1];
            CHECK(migrate2.compilationConfig.profiles.size() == 1);
            CHECK(migrate2.gameEngineConfig.profiles.empty());

            const auto& migrate3 = gameInfos[2];
            CHECK(migrate3.compilationConfig.profiles.empty());
            CHECK(migrate3.gameEngineConfig.profiles.size() == 1);

            CHECK(!env.directoryExists(userPath / "Migrate 1"));
            CHECK(env.fileExists(userPath / "Migrate1" / "CompilationProfiles.cfg"));

            CHECK(!env.directoryExists(userPath / "Migrate 2"));
            CHECK(env.fileExists(userPath / "Migrate2" / "CompilationProfiles.cfg"));

            CHECK(env.fileExists(userPath / "Migrate 3" / "CompilationProfiles.cfg"));
            CHECK(env.fileExists(userPath / "Migrate3" / "GameEngineProfiles.cfg"));
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }
  }

  SECTION("updateCompilationConfig")
  {
    auto env = fs::TestEnvironment{};

    env.createDirectory(gamesPath);
    env.createDirectory(userPath);

    writeGameConfig(env, "Quake", "Quake");

    const auto gameConfigSearchDirs = std::vector{env.dir() / gamesPath};
    const auto userGameDir = env.dir() / userPath;
    const auto expectedGameConfigPath =
      gameConfigSearchDirs.front() / "Quake" / "GameConfig.cfg";

    initializeGameManager(gameConfigSearchDirs, userGameDir)
      | kdl::transform([&](auto gameManager, const auto&) {
          const auto compilationConfig = CompilationConfig{
            .profiles = {
              {
                .name = "name",
                .workDirSpec = "workDir",
                .tasks = {},
              },
            }};

          REQUIRE(gameManager.updateCompilationConfig("Quake", compilationConfig, logger)
                    .is_success());

          const auto* gameInfo = gameManager.gameInfo("Quake");
          REQUIRE(gameInfo != nullptr);
          CHECK(gameInfo->compilationConfig == compilationConfig);
          CHECK(env.fileExists(userPath / "Quake/CompilationProfiles.cfg"));
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("updateGameEngineConfig")
  {
    auto env = fs::TestEnvironment{};

    env.createDirectory(gamesPath);
    env.createDirectory(userPath);

    writeGameConfig(env, "Quake", "Quake");

    const auto gameConfigSearchDirs = std::vector{env.dir() / gamesPath};
    const auto userGameDir = env.dir() / userPath;
    const auto expectedGameConfigPath =
      gameConfigSearchDirs.front() / "Quake" / "GameConfig.cfg";

    initializeGameManager(gameConfigSearchDirs, userGameDir)
      | kdl::transform([&](auto gameManager, const auto&) {
          const auto gameEngineConfig = GameEngineConfig{
            .profiles = {
              {
                .name = "name",
                .path = "workDir",
                .parameterSpec = "parameters",
              },
            }};

          REQUIRE(gameManager.updateGameEngineConfig("Quake", gameEngineConfig, logger)
                    .is_success());

          const auto* gameInfo = gameManager.gameInfo("Quake");
          REQUIRE(gameInfo != nullptr);
          CHECK(gameInfo->gameEngineConfig == gameEngineConfig);
          CHECK(env.fileExists(userPath / "Quake/GameEngineProfiles.cfg"));
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }
}

} // namespace tb::mdl
