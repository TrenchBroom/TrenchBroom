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

#include <QAbstractItemView>
#include <QApplication>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QtTest/QTest>

#include "CmdTool.h"
#include "TestLogger.h"
#include "base/Logger.h"
#include "gl/GlManager.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/GameEngineConfig.h"
#include "mdl/GameEngineProfile.h"
#include "mdl/GameInfo.h"
#include "mdl/GameManager.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/ControlListBox.h"
#include "ui/DisableNativeDialogs.h"
#include "ui/GameEngineDialog.h"
#include "ui/GameEngineProfileListBox.h"
#include "ui/LaunchGameEngineDialog.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/filesystem_utils.h"
#include "kd/path_utils.h"
#include "kd/result.h"

#include <fmt/format.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

const auto GameName = std::string{"Test"};

const auto GameConfig = R"({
    "version": 9,
    "name": "Test",
    "icon": "Icon.png",
    "fileformats": [
        { "format": "Standard" }
    ],
    "filesystem": {
        "searchpath": "id1",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "materials": {
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
})";

struct DialogFixture
{
  LaunchGameEngineDialog dialog;
  GameEngineProfileListBox* profileList = dialog.findChild<GameEngineProfileListBox*>();
  QLineEdit* parameterText =
    dialog.findChild<QLineEdit*>("LaunchGameEngineDialog_ParameterText");
  QPushButton* launchButton =
    dialog.findChild<QPushButton*>("LaunchGameEngineDialog_LaunchButton");
  QPushButton* closeButton =
    dialog.findChild<QPushButton*>("LaunchGameEngineDialog_CloseButton");
  QPushButton* configureButton =
    dialog.findChild<QPushButton*>("LaunchGameEngineDialog_ConfigureEnginesButton");

  DialogFixture(AppController& appController, MapDocument& document)
    : dialog{appController, document}
  {
    contract_assert(profileList != nullptr);
    contract_assert(parameterText != nullptr);
    contract_assert(launchButton != nullptr);
    contract_assert(closeButton != nullptr);
    contract_assert(configureButton != nullptr);

    dialog.show();
    QApplication::processEvents();
  }

  GameEngineDialog& openGameEngineDialog()
  {
    QTest::mouseClick(configureButton, Qt::LeftButton);

    auto* engineDialog = dialog.findChild<GameEngineDialog*>();
    contract_assert(engineDialog != nullptr);
    return *engineDialog;
  }
};

std::string logArgs(const std::filesystem::path& logPath, const std::string& args)
{
  return fmt::format(R"(--logArgs "{}" {})", logPath.generic_string(), args);
}

std::vector<std::string> readLines(const std::filesystem::path& path)
{
  return kdl::read_lines(path) | kdl::value_or(std::vector<std::string>{});
}

std::vector<std::string> waitForLoggedArgs(const std::filesystem::path& logPath)
{
  // The engine is launched as a detached process, so wait until it has logged its
  // arguments. Returns the logged arguments, or nothing if the engine was not launched.

  using namespace std::chrono_literals;

  const auto deadline = std::chrono::steady_clock::now() + 5s;

  auto lines = readLines(logPath);
  while (lines.empty() && std::chrono::steady_clock::now() < deadline)
  {
    std::this_thread::sleep_for(100ms);
    lines = readLines(logPath);
  }
  return lines;
}

auto countFailedLaunches(const LaunchGameEngineDialog& dialog)
{
  return dialog.findChildren<QMessageBox*>().size();
}

} // namespace

TEST_CASE("LaunchGameEngineDialog")
{
  auto appControllerFixture = AppControllerFixture{[](const auto& writeGameConfig) {
    writeGameConfig(GameName, GameConfig, std::nullopt, std::nullopt);
  }};
  auto& appController = appControllerFixture.appController();
  auto& gameManager = appController.gameManager();

  auto* gameInfo = gameManager.gameInfo(GameName);
  REQUIRE(gameInfo != nullptr);

  auto document = MapDocument::createDocument(
                    appController.environmentConfig(),
                    *gameInfo,
                    mdl::MapFormat::Standard,
                    vm::bbox3d{8192.0},
                    appController.taskManager(),
                    appController.glManager().resourceManager())
                  | kdl::value();

  const auto mapBaseName =
    kdl::path_remove_extension(document->map().path().filename()).string();

  auto launchLog = kdl::tmp_file{};

  const auto firstProfile = mdl::GameEngineProfile{
    .id = "first-id",
    .name = "First",
    .path = CMD_TOOL_PATH,
    .parameterSpec = logArgs(launchLog, "first"),
  };
  const auto secondProfile = mdl::GameEngineProfile{
    .id = "second-id",
    .name = "Second",
    .path = CMD_TOOL_PATH,
    .parameterSpec = logArgs(launchLog, "second ${MAP_BASE_NAME}"),
  };
  const auto missingProfile = mdl::GameEngineProfile{
    .id = "missing-id",
    .name = "Missing",
    .path = "/does/not/exist",
    .parameterSpec = "",
  };

  auto logger = NullLogger{};
  REQUIRE(gameManager
            .updateGameEngineConfig(
              GameName,
              mdl::GameEngineConfig{{firstProfile, secondProfile, missingProfile}},
              logger)
            .is_success());

  SECTION("createGui")
  {
    SECTION("selects the first profile")
    {
      auto f = DialogFixture{appController, *document};

      CHECK(f.profileList->count() == 3);
      CHECK(f.profileList->currentRow() == 0);
      CHECK(f.parameterText->isEnabled());
      CHECK(
        f.parameterText->text() == QString::fromStdString(firstProfile.parameterSpec));
      CHECK(f.launchButton->isEnabled());
    }

    SECTION("disables the parameters and the launch button if there are no profiles")
    {
      REQUIRE(gameManager.updateGameEngineConfig(GameName, {}, logger).is_success());

      auto f = DialogFixture{appController, *document};

      CHECK(f.profileList->count() == 0);
      CHECK(!f.parameterText->isEnabled());
      CHECK(f.parameterText->text() == "");
      CHECK(!f.launchButton->isEnabled());
    }

    SECTION("offers the launch variables as completions")
    {
      auto f = DialogFixture{appController, *document};

      f.parameterText->clear();
      QTest::keyClicks(f.parameterText, "${MAP_");

      auto* popup = qobject_cast<QAbstractItemView*>(QApplication::activePopupWidget());
      REQUIRE(popup != nullptr);
      REQUIRE(popup->model()->rowCount() == 1);
      CHECK(popup->model()->index(0, 0).data().toString() == "MAP_BASE_NAME");
    }
  }

  SECTION("gameEngineProfileChanged")
  {
    auto f = DialogFixture{appController, *document};

    SECTION("shows the parameters of the selected profile")
    {
      f.profileList->setCurrentRow(1);

      CHECK(f.parameterText->isEnabled());
      CHECK(
        f.parameterText->text() == QString::fromStdString(secondProfile.parameterSpec));
      CHECK(f.launchButton->isEnabled());
    }

    SECTION("disables the parameters and the launch button if no profile is selected")
    {
      f.profileList->setCurrentRow(-1);

      CHECK(!f.parameterText->isEnabled());
      CHECK(f.parameterText->text() == "");
      CHECK(!f.launchButton->isEnabled());
    }
  }

  SECTION("parametersChanged")
  {
    auto f = DialogFixture{appController, *document};

    SECTION("updates the selected profile only")
    {
      f.profileList->setCurrentRow(1);
      QTest::keyClicks(f.parameterText, " edited");

      f.profileList->setCurrentRow(0);
      CHECK(
        f.parameterText->text() == QString::fromStdString(firstProfile.parameterSpec));

      f.profileList->setCurrentRow(1);
      CHECK(
        f.parameterText->text()
        == QString::fromStdString(secondProfile.parameterSpec + " edited"));
    }

    SECTION("keeps the parameters of a profile when it is deselected")
    {
      f.profileList->setCurrentRow(-1);
      f.profileList->setCurrentRow(0);

      CHECK(
        f.parameterText->text() == QString::fromStdString(firstProfile.parameterSpec));
    }
  }

  SECTION("editGameEngines")
  {
    SECTION("keeps the selected profile and its edited parameters")
    {
      auto f = DialogFixture{appController, *document};

      f.profileList->setCurrentRow(1);
      QTest::keyClicks(f.parameterText, " edited");

      f.openGameEngineDialog().accept();

      CHECK(f.profileList->count() == 3);
      CHECK(f.profileList->currentRow() == 1);
      CHECK(
        f.parameterText->text()
        == QString::fromStdString(secondProfile.parameterSpec + " edited"));
    }

    SECTION("selects the last profile if the selected profile was removed")
    {
      auto f = DialogFixture{appController, *document};

      f.profileList->setCurrentRow(2);

      auto& engineDialog = f.openGameEngineDialog();
      auto* engineList = engineDialog.findChild<GameEngineProfileListBox*>();
      auto* removeButton = engineDialog.findChild<QToolButton*>(
        "GameEngineProfileManager_RemoveProfileButton");

      engineList->setCurrentRow(2);
      removeButton->click();
      engineDialog.accept();

      CHECK(f.profileList->count() == 2);
      CHECK(f.profileList->currentRow() == 1);
      CHECK(
        f.parameterText->text() == QString::fromStdString(secondProfile.parameterSpec));
    }

    SECTION("selects the first profile if there were no profiles before")
    {
      REQUIRE(gameManager.updateGameEngineConfig(GameName, {}, logger).is_success());

      auto f = DialogFixture{appController, *document};

      auto& engineDialog = f.openGameEngineDialog();
      auto* addButton =
        engineDialog.findChild<QToolButton*>("GameEngineProfileManager_AddProfileButton");

      addButton->click();
      engineDialog.accept();

      CHECK(f.profileList->count() == 1);
      CHECK(f.profileList->currentRow() == 0);
      CHECK(f.parameterText->isEnabled());
      CHECK(f.launchButton->isEnabled());
    }

    SECTION("disables the parameters and the launch button if all profiles were removed")
    {
      auto f = DialogFixture{appController, *document};

      auto& engineDialog = f.openGameEngineDialog();
      auto* engineList = engineDialog.findChild<GameEngineProfileListBox*>();
      auto* removeButton = engineDialog.findChild<QToolButton*>(
        "GameEngineProfileManager_RemoveProfileButton");

      while (engineList->count() > 0)
      {
        engineList->setCurrentRow(0);
        removeButton->click();
      }
      engineDialog.accept();

      CHECK(f.profileList->count() == 0);
      CHECK(!f.parameterText->isEnabled());
      CHECK(f.parameterText->text() == "");
      CHECK(!f.launchButton->isEnabled());
    }
  }

  SECTION("launchEngine")
  {
    auto f = DialogFixture{appController, *document};

    SECTION("launches the selected profile when the launch button is clicked")
    {
      f.profileList->setCurrentRow(1);
      QTest::mouseClick(f.launchButton, Qt::LeftButton);

      CHECK(
        waitForLoggedArgs(launchLog) == std::vector<std::string>{"second", mapBaseName});
      CHECK(f.dialog.isVisible());
    }

    SECTION("launches the selected profile with its edited parameters")
    {
      QTest::keyClicks(f.parameterText, " edited");
      QTest::mouseClick(f.launchButton, Qt::LeftButton);

      CHECK(waitForLoggedArgs(launchLog) == std::vector<std::string>{"first", "edited"});
    }

    SECTION("launches the selected profile once when return is pressed")
    {
      const auto disableNativeDialogs = DisableNativeDialogs{};

      f.profileList->setCurrentRow(2);

      // The parameters have the focus while they are edited. Without it, another button
      // with the focus could be the default button that return triggers.
      f.parameterText->setFocus();
      QTest::keyClick(f.parameterText, Qt::Key_Return);

      CHECK(countFailedLaunches(f.dialog) == 1);
    }

    SECTION("launches a profile when it is double clicked")
    {
      const auto renderers = f.profileList->findChildren<ControlListBoxItemRenderer*>();
      REQUIRE(renderers.size() == 3);

      // a double click is preceded by a click that selects the profile
      QTest::mouseClick(renderers[1], Qt::LeftButton);
      REQUIRE(f.profileList->currentRow() == 1);

      QTest::mouseDClick(renderers[1], Qt::LeftButton);

      CHECK(
        waitForLoggedArgs(launchLog) == std::vector<std::string>{"second", mapBaseName});
    }

    SECTION("launches a profile when it is double clicked while it is not selected")
    {
      const auto renderers = f.profileList->findChildren<ControlListBoxItemRenderer*>();
      REQUIRE(renderers.size() == 3);

      // clicking the selected profile with the control modifier deselects it
      QTest::mouseClick(renderers[0], Qt::LeftButton, Qt::ControlModifier);
      REQUIRE(f.profileList->selectedRow() == -1);

      QTest::mouseDClick(renderers[0], Qt::LeftButton, Qt::ControlModifier);

      CHECK(waitForLoggedArgs(launchLog) == std::vector<std::string>{"first"});
    }

    SECTION("does not launch when return accepts a completion")
    {
      const auto disableNativeDialogs = DisableNativeDialogs{};

      f.profileList->setCurrentRow(2);
      QTest::keyClicks(f.parameterText, "${MAP_");

      // while the completion popup is visible, it receives all key events
      auto* popup = QApplication::activePopupWidget();
      REQUIRE(popup != nullptr);

      QTest::keyClick(popup, Qt::Key_Down);
      QTest::keyClick(popup, Qt::Key_Return);

      CHECK(f.parameterText->text() == "${MAP_BASE_NAME}");
      CHECK(countFailedLaunches(f.dialog) == 0);
    }

    SECTION("shows an error if the engine cannot be launched")
    {
      const auto disableNativeDialogs = DisableNativeDialogs{};

      f.profileList->setCurrentRow(2);
      QTest::mouseClick(f.launchButton, Qt::LeftButton);

      const auto* messageBox = f.dialog.findChild<QMessageBox*>();
      REQUIRE(messageBox != nullptr);
      CHECK(messageBox->isVisible());
      CHECK(messageBox->text().startsWith("Could not launch game engine"));
    }
  }

  SECTION("done")
  {
    SECTION("saves the edited parameters")
    {
      auto f = DialogFixture{appController, *document};

      QTest::keyClicks(f.parameterText, " edited");
      QTest::mouseClick(f.closeButton, Qt::LeftButton);

      CHECK(!f.dialog.isVisible());

      REQUIRE(gameInfo->gameEngineConfig.profiles.size() == 3);
      CHECK(
        gameInfo->gameEngineConfig.profiles[0].parameterSpec
        == firstProfile.parameterSpec + " edited");
    }

    SECTION("logs an error if the game engine config cannot be saved")
    {
      // The game manager does not know the game of this document.
      auto testLogger = TestLogger{};
      auto unknownGameDocument = MapDocument::createDocument(
                                   appController.environmentConfig(),
                                   mdl::QuakeGameInfo,
                                   mdl::MapFormat::Valve,
                                   vm::bbox3d{8192.0},
                                   appController.taskManager(),
                                   appController.glManager().resourceManager())
                                 | kdl::value();
      unknownGameDocument->setTargetLogger(&testLogger);

      auto f = DialogFixture{appController, *unknownGameDocument};

      const auto errorCount = testLogger.countMessages(LogLevel::Error);
      QTest::mouseClick(f.closeButton, Qt::LeftButton);

      CHECK(!f.dialog.isVisible());
      CHECK(testLogger.countMessages(LogLevel::Error) == errorCount + 1);
    }
  }
}

} // namespace tb::ui
