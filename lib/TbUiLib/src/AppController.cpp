/*
 Copyright (C) 2025 Kristian Duske

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

#include "ui/AppController.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QTimer>

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "gl/GlManager.h"
#include "gl/ResourceManager.h"
#include "gl/VboManager.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameManager.h"
#include "mdl/MapHeader.h"
#include "ui/AboutDialog.h"
#include "ui/ActionManager.h"
#include "ui/CrashDialog.h"
#include "ui/FileDialogDefaultDir.h"
#include "ui/GameDialog.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/PreferenceDialog.h"
#include "ui/QPathUtils.h"
#include "ui/RecentDocuments.h"
#include "ui/SystemPaths.h"
#include "ui/UpdateConfig.h"
#include "ui/WelcomeWindow.h"
#include "update/QtHttpClient.h"
#include "update/Updater.h"

#include "kd/const_overload.h"
#include "kd/task_manager.h"
#include "kd/vector_utils.h"

#include <fmt/format.h>

#include <chrono>

namespace tb::ui
{
namespace
{

auto createEnvironmentConfig()
{
  return std::make_unique<mdl::EnvironmentConfig>(mdl::EnvironmentConfig{
    .appFolderPath = SystemPaths::appDirectory(),
    .userDataFolderPath = SystemPaths::userDataDirectory(),
    .tempFolderPath = SystemPaths::tempDirectory(),
    .defaultAssetFolderPaths =
      SystemPaths::findResourceDirectories(std::filesystem::path{"defaults"}),
  });
}

auto createGameManager()
{
  return mdl::initializeGameManager(
           ui::SystemPaths::findResourceDirectories("games"),
           ui::SystemPaths::userGamesDirectory())
         | kdl::transform([](auto gameManager, const auto& warnings) {
             if (!warnings.empty())
             {
               const auto msg = fmt::format(
                 R"(Some game configurations could not be loaded. The following errors occurred:

{})",
                 kdl::str_join(warnings, "\n\n"));

               QMessageBox::critical(
                 nullptr, "TrenchBroom", QString::fromStdString(msg), QMessageBox::Ok);
             }

             return std::make_unique<mdl::GameManager>(std::move(gameManager));
           });
}

auto createMapWindowManager(AppController& appController)
{
  return new MapWindowManager{appController, AppController::useSDI};
}

auto createRecentDocuments(QObject* parent)
{
  auto checkRecentDocument = [](const auto& path) {
    auto ec = std::error_code{};
    return std::filesystem::is_regular_file(path, ec) && !ec;
  };

  return new RecentDocuments{10, std::move(checkRecentDocument), parent};
}

std::optional<std::tuple<std::string, mdl::MapFormat>> detectOrQueryGameAndFormat(
  AppController& appController, const std::filesystem::path& path)
{
  return fs::Disk::withInputStream(path, mdl::readMapHeader)
         | kdl::transform(
           [&](auto detectedGameNameAndMapFormat)
             -> std::optional<std::tuple<std::string, mdl::MapFormat>> {
             auto [gameName, mapFormat] = detectedGameNameAndMapFormat;
             const auto& gameManager = appController.gameManager();
             const auto gameList = gameManager.gameInfos()
                                   | std::views::transform([](const auto& gameInfo) {
                                       return gameInfo.gameConfig.name;
                                     })
                                   | kdl::ranges::to<std::vector>();

             if (
               gameName == std::nullopt || !kdl::vec_contains(gameList, *gameName)
               || mapFormat == mdl::MapFormat::Unknown)
             {
               auto queriedGameNameAndMapFormat =
                 GameDialog::showOpenDocumentDialog(appController);
               if (!queriedGameNameAndMapFormat)
               {
                 return std::nullopt;
               }

               std::tie(gameName, mapFormat) = *queriedGameNameAndMapFormat;
             }

             return std::optional{std::tuple{std::move(*gameName), mapFormat}};
           })
         | kdl::transform_error([](const auto&) { return std::nullopt; }) | kdl::value();
}

} // namespace

AppController::AppController(
  std::unique_ptr<kdl::task_manager> taskManager,
  std::unique_ptr<mdl::EnvironmentConfig> environmentConfig,
  std::unique_ptr<mdl::GameManager> gameManager)
  : m_taskManager{std::move(taskManager)}
  , m_environmentConfig{std::move(environmentConfig)}
  , m_gameManager{std::move(gameManager)}
  , m_glManager{std::make_unique<gl::GlManager>(
      [](const auto& path) { return SystemPaths::findResourceFile(path); })}
  , m_networkManager{new QNetworkAccessManager{this}}
  , m_reloadRecentDocumentsTimer{new QTimer{this}}
  , m_processResourcesTimer{new QTimer{this}}
  , m_httpClient{new upd::QtHttpClient{*m_networkManager}}
  , m_updater{new upd::Updater{*m_httpClient, makeUpdateConfig(), this}}
  , m_mapWindowManager{createMapWindowManager(*this)}
  , m_recentDocuments{createRecentDocuments(this)}
  , m_actionManager{std::make_unique<ActionManager>()}
  , m_welcomeWindow{std::make_unique<WelcomeWindow>(*this)}
  , m_aboutDialog{std::make_unique<AboutDialog>(*this)}
{
  using namespace std::chrono_literals;

  connectObservers();

  m_reloadRecentDocumentsTimer->start(1s);
  m_processResourcesTimer->start(20ms);
}

Result<std::unique_ptr<AppController>> AppController::create()
{
  return createGameManager() | kdl::transform([&](auto gameManager) {
           auto taskManager =
             std::make_unique<kdl::task_manager>(std::thread::hardware_concurrency());
           auto environmentConfig = createEnvironmentConfig();

           return std::make_unique<AppController>(
             std::move(taskManager),
             std::move(environmentConfig),
             std::move(gameManager));
         });
}

AppController::~AppController()
{
  processGlResources();
}

kdl::task_manager& AppController::taskManager()
{
  return *m_taskManager;
}

gl::GlManager& AppController::glManager()
{
  return *m_glManager;
}

const mdl::EnvironmentConfig& AppController::environmentConfig() const
{
  return *m_environmentConfig;
}

mdl::GameManager& AppController::gameManager()
{
  return *m_gameManager;
}

upd::Updater& AppController::updater()
{
  return *m_updater;
}

MapWindowManager& AppController::mapWindowManager()
{
  return *m_mapWindowManager;
}

const RecentDocuments& AppController::recentDocuments() const
{
  return *m_recentDocuments;
}

RecentDocuments& AppController::recentDocuments()
{
  return KDL_CONST_OVERLOAD(recentDocuments());
}

ActionManager& AppController::actionManager()
{
  return *m_actionManager;
}

void AppController::askForAutoUpdates()
{
  if (pref(Preferences::AskForAutoUpdates))
  {
    auto& prefs = PreferenceManager::instance();

    const auto enableAutoCheck =
      QMessageBox::question(
        nullptr,
        "TrenchBroom",
        tr(
          R"(TrenchBroom can check for updates automatically. Would you like to enable this now?)"),
        QMessageBox::Yes | QMessageBox::No)
      == QMessageBox::Yes;

    prefs.set(Preferences::AutoCheckForUpdates, enableAutoCheck);
    prefs.set(Preferences::AskForAutoUpdates, false);
    prefs.saveChanges();
  }
}

void AppController::triggerAutoUpdateCheck()
{
  if (pref(Preferences::AutoCheckForUpdates))
  {
    m_updater->checkForUpdates();
  }
}

bool AppController::newDocument()
{
  const auto gameNameAndMapFormat = GameDialog::showNewDocumentDialog(*this);
  if (!gameNameAndMapFormat)
  {
    return false;
  }

  const auto [gameName, mapFormat] = *gameNameAndMapFormat;
  const auto* gameInfo = gameManager().gameInfo(gameName);
  contract_assert(gameInfo != nullptr);

  return m_mapWindowManager->createDocument(
           *gameInfo, mapFormat, MapDocument::DefaultWorldBounds)
         | kdl::transform([&]() {
             m_welcomeWindow->close();
             return true;
           })
         | kdl::transform_error([&](const auto& e) {
             QMessageBox::critical(
               nullptr, "TrenchBroom", e.msg.c_str(), QMessageBox::Ok);
             return false;
           })
         | kdl::value();
}

void AppController::openDocument()
{
  const auto pathStr = QFileDialog::getOpenFileName(
    nullptr,
    tr("Open Map"),
    fileDialogDefaultDirectory(FileDialogDir::Map),
    "Map files (*.map);;Any files (*.*)");

  if (const auto path = pathFromQString(pathStr); !path.empty())
  {
    updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::Map, pathStr);
    openDocument(path);
  }
}

bool AppController::openDocument(const std::filesystem::path& path)
{
  // if std::filesystem::absolute fails, the file won't be found and we'll log it later
  auto ec = std::error_code{};
  auto absPath =
    path.is_absolute() ? path : std::filesystem::absolute(path, ec).lexically_normal();

  const auto checkFileExists = [&]() {
    return fs::Disk::pathInfo(absPath) == fs::PathInfo::File
             ? Result<void>{}
             : Result<void>{Error{fmt::format("{} not found", path)}};
  };

  return checkFileExists() | kdl::or_else([&](const auto& e) {
           m_recentDocuments->removePath(absPath);
           return Result<void>{e};
         })
         | kdl::and_then([&]() {
             const auto gameNameAndMapFormat = detectOrQueryGameAndFormat(*this, absPath);
             if (!gameNameAndMapFormat)
             {
               return Result<bool>{false};
             }

             const auto [gameName, mapFormat] = *gameNameAndMapFormat;
             const auto* gameInfo = gameManager().gameInfo(gameName);
             contract_assert(gameInfo != nullptr);

             return m_mapWindowManager->loadDocument(
                      *gameInfo,
                      mapFormat,
                      MapDocument::DefaultWorldBounds,
                      std::move(absPath))
                    | kdl::transform([&]() {
                        m_welcomeWindow->close();
                        return true;
                      });
           })
         | kdl::transform_error([&](const auto& e) {
             QMessageBox::critical(
               nullptr, "TrenchBroom", e.msg.c_str(), QMessageBox::Ok);
             return false;
           })
         | kdl::value();
}

void AppController::showWelcomeWindow()
{
  m_welcomeWindow->show();
  m_welcomeWindow->raise();
}

void AppController::showManual()
{
  const auto manualPath = SystemPaths::findResourceFile("manual/index.html");
  const auto manualPathUrl = QUrl::fromLocalFile(pathAsQString(manualPath));
  QDesktopServices::openUrl(manualPathUrl);
}

void AppController::showPreferences()
{
  auto* topMapWindow = mapWindowManager().topMapWindow();
  auto* topDocument = topMapWindow ? &topMapWindow->document() : nullptr;

  auto dialog = PreferenceDialog{*this, topDocument};
  dialog.exec();
}

void AppController::showAboutDialog()
{
  m_aboutDialog->show();
  m_aboutDialog->raise();
}

void AppController::debugShowCrashReportDialog()
{
  const auto reportPath = ui::SystemPaths::userDataDirectory() / "crashreport.txt";
  const auto mapPath = ui::SystemPaths::userDataDirectory() / "crashreport.map";
  const auto logPath = ui::SystemPaths::userDataDirectory() / "crashreport.log";

  auto dialog = CrashDialog{"Debug crash", reportPath, mapPath, logPath};
  dialog.exec();
}

void AppController::connectObservers()
{
  connect(
    m_recentDocuments,
    &RecentDocuments::loadDocument,
    this,
    [this](const std::filesystem::path& path) { openDocument(path); });
  connect(
    m_reloadRecentDocumentsTimer,
    &QTimer::timeout,
    m_recentDocuments,
    &RecentDocuments::reload);
  connect(
    m_processResourcesTimer, &QTimer::timeout, this, &AppController::processGlResources);
}

void AppController::processGlResources()
{
  using namespace std::chrono_literals;

  if (m_glManager->initialized())
  {
    auto taskRunner = [&](auto task) { return taskManager().run_task(std::move(task)); };

    auto errorHandler = [&](const auto&, const auto& error) {
      if (auto* topWindow = mapWindowManager().topMapWindow())
      {
        topWindow->logger().error() << error;
      }
    };

    auto processContext = tb::gl::ProcessContext{true, errorHandler};

    m_glManager->resourceManager().process(taskRunner, processContext, 20ms);
    m_glManager->vboManager().destroyPendingVbos();
  }
}

} // namespace tb::ui
