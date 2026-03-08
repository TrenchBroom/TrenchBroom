/*
 Copyright (C) 2026 Kristian Duske

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

#include "ui/AppControllerFixture.h"

#include "mdl/EnvironmentConfig.h"
#include "mdl/GameManager.h"
#include "mdl/TestUtils.h"

#include <filesystem>

namespace tb::ui
{
namespace
{
const auto appPath = std::filesystem::path{"__app"};
const auto tempPath = std::filesystem::path{"__temp"};

const auto gamesPath = std::filesystem::path{"games"};
const auto userPath = std::filesystem::path{"user"};

auto createEnvironmentConfig(fs::TestEnvironment& testEnvironment)
{
  return std::make_unique<mdl::EnvironmentConfig>(
    testEnvironment.dir() / appPath,
    testEnvironment.dir() / userPath,
    testEnvironment.dir() / tempPath,
    std::vector<std::filesystem::path>{});
}

auto createGameManager(
  fs::TestEnvironment& testEnvironment,
  const GameManagerInitializer& gameManagerInitializer)
{
  const auto gameConfigSearchDirs = std::vector{testEnvironment.dir() / gamesPath};
  const auto userGameDir = testEnvironment.dir() / userPath;

  auto writeGameConfig = [&](
                           const std::string_view gameName,
                           const std::string_view gameConfig,
                           const std::optional<std::string_view> compilationProfile,
                           const std::optional<std::string_view> gameEngineProfile) {
    testEnvironment.createDirectory(gamesPath / gameName);
    testEnvironment.createFile(gamesPath / gameName / "GameConfig.cfg", gameConfig);

    if (compilationProfile)
    {
      testEnvironment.createDirectory(userPath / gameName);
      testEnvironment.createFile(
        gamesPath / gameName / "CompilationProfiles.cfg", *compilationProfile);
    }

    if (gameEngineProfile)
    {
      testEnvironment.createDirectory(userPath / gameName);
      testEnvironment.createFile(
        gamesPath / gameName / "GameEngineProfiles.cfg", *gameEngineProfile);
    }
  };

  gameManagerInitializer(writeGameConfig);

  auto gameManager = mdl::initializeGameManager(gameConfigSearchDirs, userGameDir)
                     | kdl::transform([](auto manager, const auto&) { return manager; })
                     | kdl::value();

  return std::make_unique<mdl::GameManager>(std::move(gameManager));
}

} // namespace

AppControllerFixture::AppControllerFixture(
  const GameManagerInitializer& gameManagerInitializer)
  : m_appController{
      createTestTaskManager(),
      createEnvironmentConfig(m_testEnvironment),
      createGameManager(m_testEnvironment, gameManagerInitializer)}
{
}

AppControllerFixture::~AppControllerFixture() = default;

AppController& AppControllerFixture::appController()
{
  return m_appController;
}

} // namespace tb::ui
