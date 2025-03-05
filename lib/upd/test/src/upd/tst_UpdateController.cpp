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

#include <QTemporaryFile>
#include <QtTest/QSignalSpy>

#include "upd/TestHttpClient.h"
#include "upd/TestVersion.h"
#include "upd/UpdateController.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "Catch2.h"

namespace upd
{
using namespace std::chrono_literals;

namespace
{

const auto v1 = Release<TestVersion>{
  TestVersion{1},
  false,
  false,
  "v1",
  "v1_url",
  {Asset{"V1 asset", QUrl{"asset_url_v1"}, 123}}};
const auto v2 = Release<TestVersion>{
  TestVersion{2},
  false,
  false,
  "v2",
  "v2_url",
  {Asset{"V2 asset", QUrl{"asset_url_v2"}, 456}}};

template <typename State>
bool waitForState(
  const UpdateController& updateController,
  QEventLoop& loop,
  const std::chrono::milliseconds timeout = 5s)
{
  const auto startTime = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startTime < timeout)
  {
    loop.processEvents();
    if (std::holds_alternative<State>(updateController.state()))
    {
      return true;
    }
  }
  return false;
}

} // namespace

TEST_CASE("UpdateController")
{
  int argc = 0;
  char** argv = nullptr;
  auto app = QCoreApplication{argc, argv};
  auto loop = QEventLoop{};
  auto httpClient = TestHttpClient{};

  SECTION("when updating is disabled")
  {
    auto updateController = UpdateController{httpClient, std::nullopt};
    CHECK(updateController.state() == UpdateControllerState{UpdateDisabledState{}});
  }

  SECTION("when updating is enabled")
  {
    auto logFile = QTemporaryFile{};
    REQUIRE(logFile.open());
    logFile.close();

    auto prepareUpdate =
      PrepareUpdate{[](auto, auto) { return std::optional{QString{"some path"}}; }};

    auto installUpdate = InstallUpdate{[](auto, auto, auto) {}};

    const auto fixturePath = QDir::currentPath() + "/fixture/update_controller";
    const auto workDirPath = fixturePath + "/work";

    auto config = UpdateConfig{
      [](auto& updateController) {
        updateController.template checkForUpdates<TestVersion>(
          TestVersion{1}, false, parseVersion, describeVersion, chooseFirstAsset);
      },
      [&](const auto& downloadedUpdatePath, const auto& updateConfig) {
        return prepareUpdate(downloadedUpdatePath, updateConfig);
      },
      [&](
        const auto& preparedUpdatePath, const auto& updateConfig, const auto restartApp) {
        installUpdate(preparedUpdatePath, updateConfig, restartApp);
      },

      "some_org",
      "some_app",
      "/path/to/scripts",
      "/path/to/app",
      "relative/app",
      workDirPath,
      logFile.fileName()};
    auto updateController = std::make_unique<UpdateController>(httpClient, config);

    qRegisterMetaType<UpdateControllerState>();
    auto stateChangedSpy = QSignalSpy{
      updateController.get(), SIGNAL(stateChanged(const UpdateControllerState&))};
    REQUIRE(stateChangedSpy.isValid());

    const auto spyState = [&]() {
      return get<UpdateControllerState>(stateChangedSpy.last().first());
    };

    SECTION("checkForUpdates")
    {
      updateController->checkForUpdates();
      REQUIRE(std::holds_alternative<CheckingForUpdatesState>(spyState()));

      stateChangedSpy.clear();

      SECTION("state changes to UpdateErrorState when get fails")
      {
        httpClient.pendingGetOperation->errorCallback("some error");
        CHECK(spyState() == UpdateControllerState{UpdateErrorState{"some error"}});
      }

      SECTION("callback is called with result when get succeeds")
      {
        httpClient.pendingGetOperation->successCallback(makeGetReleasesJson({v1, v2}));
        CHECK(
          spyState()
          == UpdateControllerState{
            UpdateAvailableState{makeUpdateInfo<TestVersion>(
                                   TestVersion{1}, v2, describeVersion, chooseFirstAsset)
                                   .value()}});
      }

      SECTION("calling again while check is pending does nothing")
      {
        const auto* pendingOperation = httpClient.pendingGetOperation;
        REQUIRE(pendingOperation != nullptr);

        updateController->checkForUpdates();

        CHECK(httpClient.pendingGetOperation == pendingOperation);
        CHECK(!pendingOperation->cancelled);
        CHECK(stateChangedSpy.count() == 0);
      }

      SECTION("cancelPendingOperation")
      {
        updateController->cancelPendingOperation();
        CHECK(std::holds_alternative<IdleState>(spyState()));
      }
    }

    SECTION("downloadAndPrepareUpdate")
    {
      updateController->checkForUpdates();
      REQUIRE(std::holds_alternative<CheckingForUpdatesState>(spyState()));

      httpClient.pendingGetOperation->successCallback(makeGetReleasesJson({v2}));
      REQUIRE(std::holds_alternative<UpdateAvailableState>(spyState()));

      stateChangedSpy.clear();

      updateController->downloadAndPrepareUpdate();
      REQUIRE(std::holds_alternative<DownloadingUpdateState>(spyState()));

      SECTION("cancelPendingOperation")
      {
        updateController->cancelPendingOperation();
        CHECK(std::holds_alternative<IdleState>(spyState()));
      }

      SECTION("state changes to UpdateErrorState when download fails")
      {
        httpClient.pendingDownloadOperation->errorCallback("some error");
        CHECK(spyState() == UpdateControllerState{UpdateErrorState{"some error"}});
      }

      SECTION("state changes to PreparingUpdateState when download succeeds")
      {
        auto mutex = std::mutex{};
        auto cv = std::condition_variable{};
        auto prepareUpdateFinished = false;
        auto prepareUpdateResult = std::optional<QString>{};

        const auto finishPrepareUpdate = [&](auto result) {
          prepareUpdateResult = std::move(result);
          auto lock = std::lock_guard{mutex};
          prepareUpdateFinished = true;
          cv.notify_one();
        };

        auto packageFile = QFile{fixturePath + "/package/update.zip"};

        prepareUpdate = [&](const auto& downloadedUpdatePath, const auto&) {
          CHECK(downloadedUpdatePath == packageFile.fileName());

          auto lock = std::unique_lock{mutex};
          cv.wait(lock, [&] { return prepareUpdateFinished; });
          return prepareUpdateResult;
        };

        httpClient.pendingDownloadOperation->successCallback(packageFile);
        CHECK(std::holds_alternative<PreparingUpdateState>(spyState()));

        SECTION("state changes to UpdatePendingState when prepareUpdate fails")
        {
          finishPrepareUpdate(std::nullopt);

          CHECK(waitForState<UpdateErrorState>(*updateController, loop));
          CHECK(
            spyState()
            == UpdateControllerState{UpdateErrorState{"Failed to prepare update file"}});
        }

        SECTION("state changes to UpdatePendingState when prepareUpdate succeeds")
        {
          finishPrepareUpdate(QString{"/some/path"});

          CHECK(waitForState<UpdatePendingState>(*updateController, loop));
          CHECK(spyState() == UpdateControllerState{UpdatePendingState{"/some/path"}});

          SECTION("calls installUpdate when destroyed")
          {
            const auto expectedRestartApp = GENERATE(true, false);
            updateController->setRestartApp(expectedRestartApp);

            auto installUpdateCalled = false;
            installUpdate =
              [&](const auto& preparedUpdatePath, const auto&, const auto restartApp) {
                CHECK(preparedUpdatePath == "/some/path");
                CHECK(restartApp == expectedRestartApp);
                installUpdateCalled = true;
              };

            updateController.reset();
            CHECK(installUpdateCalled);
          }
        }
      }

      SECTION("cancelPendingOperation")
      {
        updateController->cancelPendingOperation();
        CHECK(std::holds_alternative<IdleState>(spyState()));
      }
    }
  }
}

} // namespace upd
