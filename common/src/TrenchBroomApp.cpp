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

#include "TrenchBroomApp.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Result.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "io/MapHeader.h"
#include "io/PathQt.h"
#include "io/SystemPaths.h"
#include "mdl/Game.h" // IWYU pragma: keep
#include "mdl/GameManager.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "ui/AboutDialog.h"
#include "ui/Actions.h"
#include "ui/CrashDialog.h"
#include "ui/CrashReporter.h"
#include "ui/FrameManager.h"
#include "ui/GameDialog.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"
#include "ui/PreferenceDialog.h"
#include "ui/QtUtils.h"
#include "ui/RecentDocuments.h"
#include "ui/UpdateConfig.h"
#include "ui/WelcomeWindow.h"
#include "update/QtHttpClient.h"
#include "update/Updater.h"

#include "kd/contracts.h"
#include "kd/vector_utils.h"
#ifdef __APPLE__
#include "ui/ActionBuilder.h"
#endif

#include <QColor>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPalette>
#include <QProxyStyle>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTimer>
#include <QUrl>

#include <fmt/format.h>
#include <fmt/std.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace tb::ui
{
namespace
{

std::optional<std::tuple<std::string, mdl::MapFormat>> detectOrQueryGameAndFormat(
  const std::filesystem::path& path)
{
  return fs::Disk::withInputStream(path, io::readMapHeader)
         | kdl::transform(
           [&](auto detectedGameNameAndMapFormat)
             -> std::optional<std::tuple<std::string, mdl::MapFormat>> {
             auto [gameName, mapFormat] = detectedGameNameAndMapFormat;
             const auto& gameManager = TrenchBroomApp::instance().gameManager();
             const auto gameList = gameManager.gameInfos()
                                   | std::views::transform([](const auto& gameInfo) {
                                       return gameInfo.gameConfig.name;
                                     })
                                   | kdl::ranges::to<std::vector>();

             if (
               gameName == std::nullopt || !kdl::vec_contains(gameList, *gameName)
               || mapFormat == mdl::MapFormat::Unknown)
             {
               auto queriedGameNameAndMapFormat = GameDialog::showOpenDocumentDialog();
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

TrenchBroomApp& TrenchBroomApp::instance()
{
  return *dynamic_cast<TrenchBroomApp*>(qApp);
}

TrenchBroomApp::TrenchBroomApp(int& argc, char** argv)
  : QApplication{argc, argv}
  , m_networkManager{new QNetworkAccessManager{this}}
  , m_httpClient{new upd::QtHttpClient{*m_networkManager}}
  , m_updater{new upd::Updater{*m_httpClient, makeUpdateConfig(), this}}
  , m_taskManager{std::thread::hardware_concurrency()}
{
  using namespace std::chrono_literals;

  // When this flag is enabled, font and palette changes propagate as though the user
  // had manually called the corresponding QWidget methods.
  setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);

  // Don't show icons in menus, they are scaled down and don't look very good.
  setAttribute(Qt::AA_DontShowIconsInMenus);

  setupCrashReporter();

  setApplicationName("TrenchBroom");
  // Needs to be "" otherwise Qt adds this to the paths returned by QStandardPaths
  // which would cause preferences to move from where they were with wx
  setOrganizationName("");
  setOrganizationDomain("io.github.trenchbroom");

  m_gameManager = createGameManager();

  loadStyleSheets();
  loadStyle();

  // these must be initialized here and not earlier
  m_frameManager = std::make_unique<FrameManager>(useSDI());

  m_recentDocuments = std::make_unique<RecentDocuments>(
    10, [](const auto& path) { return fs::Disk::pathInfo(path) == fs::PathInfo::File; });
  connect(
    m_recentDocuments.get(),
    &RecentDocuments::loadDocument,
    this,
    [this](const std::filesystem::path& path) { openDocument(path); });
  connect(
    m_recentDocuments.get(),
    &RecentDocuments::didChange,
    this,
    &TrenchBroomApp::recentDocumentsDidChange);
  m_recentDocuments->reload();
  m_recentDocumentsReloadTimer = new QTimer{};
  connect(
    m_recentDocumentsReloadTimer,
    &QTimer::timeout,
    m_recentDocuments.get(),
    &RecentDocuments::reload);
  m_recentDocumentsReloadTimer->start(1s);

#ifdef __APPLE__
  setQuitOnLastWindowClosed(false);

  auto* menuBar = new QMenuBar{};
  auto actionMap = std::unordered_map<const Action*, QAction*>{};

  auto menuBuilderResult = populateMenuBar(*menuBar, actionMap, [](const Action& action) {
    auto context = ActionExecutionContext{nullptr, nullptr};
    action.execute(context);
  });

  addRecentDocumentMenu(*menuBuilderResult.recentDocumentsMenu);

  auto context = ActionExecutionContext{nullptr, nullptr};
  for (auto [tbAction, qtAction] : actionMap)
  {
    qtAction->setEnabled(tbAction->enabled(context));
    if (qtAction->isCheckable())
    {
      qtAction->setChecked(tbAction->checked(context));
    }
  }
#endif
}

TrenchBroomApp::~TrenchBroomApp()
{
  PreferenceManager::destroyInstance();
}

std::unique_ptr<mdl::GameManager> TrenchBroomApp::createGameManager()
{
  return mdl::initializeGameManager(
           io::SystemPaths::findResourceDirectories("games"),
           io::SystemPaths::userGamesDirectory())
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
           })
         | kdl::if_error([](auto e) {
             const auto msg =
               fmt::format(R"(Game configurations could not be loaded: {})", e.msg);

             QMessageBox::critical(
               nullptr, "TrenchBroom", QString::fromStdString(msg), QMessageBox::Ok);
             QCoreApplication::exit(1);
           })
         | kdl::value();
}

void TrenchBroomApp::askForAutoUpdates()
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

void TrenchBroomApp::triggerAutoUpdateCheck()
{
  if (pref(Preferences::AutoCheckForUpdates))
  {
    m_updater->checkForUpdates();
  }
}

void TrenchBroomApp::parseCommandLineAndShowFrame()
{
  auto parser = QCommandLineParser{};
  parser.addOption(QCommandLineOption("portable"));
  parser.addOption(QCommandLineOption("enableDraftReleaseUpdates"));
  parser.process(*this);

  if (parser.isSet("enableDraftReleaseUpdates"))
  {
    auto& prefs = PreferenceManager::instance();
    prefs.set(Preferences::EnableDraftReleaseUpdates, true);
    prefs.set(Preferences::IncludeDraftReleaseUpdates, true);
  }

  openFilesOrWelcomeFrame(parser.positionalArguments());
}

mdl::GameManager& TrenchBroomApp::gameManager()
{
  return *m_gameManager;
}


upd::Updater& TrenchBroomApp::updater()
{
  return *m_updater;
}

FrameManager* TrenchBroomApp::frameManager()
{
  return m_frameManager.get();
}

QPalette TrenchBroomApp::darkPalette()
{
  const auto button = QColor{35, 35, 35};
  const auto text = QColor{207, 207, 207};
  const auto highlight = QColor{62, 112, 205};

  // Build an initial palette based on the button color
  auto palette = QPalette{button};

  // Window colors
  palette.setColor(QPalette::Active, QPalette::Window, QColor{50, 50, 50});
  palette.setColor(QPalette::Inactive, QPalette::Window, QColor{40, 40, 40});
  palette.setColor(QPalette::Disabled, QPalette::Window, QColor{50, 50, 50}.darker(200));

  // List box backgrounds, text entry backgrounds, menu backgrounds
  palette.setColor(QPalette::Base, button.darker(130));

  // Button text
  palette.setColor(QPalette::Active, QPalette::ButtonText, text);
  palette.setColor(QPalette::Inactive, QPalette::ButtonText, text);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, text.darker(200));

  // WindowText is supposed to be against QPalette::Window
  palette.setColor(QPalette::Active, QPalette::WindowText, text);
  palette.setColor(QPalette::Inactive, QPalette::WindowText, text);
  palette.setColor(QPalette::Disabled, QPalette::WindowText, text.darker(200));

  // Menu text, text edit text, table cell text
  palette.setColor(QPalette::Active, QPalette::Text, text.darker(115));
  palette.setColor(QPalette::Inactive, QPalette::Text, text.darker(115));

  // Disabled menu item text color
  palette.setColor(QPalette::Disabled, QPalette::Text, QColor{102, 102, 102});

  // Disabled menu item text shadow
  palette.setColor(QPalette::Disabled, QPalette::Light, button.darker(200));

  // Highlight (selected list box row, selected grid cell background, selected tab text
  palette.setColor(QPalette::Active, QPalette::Highlight, highlight);
  palette.setColor(QPalette::Inactive, QPalette::Highlight, highlight);
  palette.setColor(QPalette::Disabled, QPalette::Highlight, highlight);

  return palette;
}

bool TrenchBroomApp::loadStyleSheets()
{
  const auto path = io::SystemPaths::findResourceFile("stylesheets/base.qss");
  if (auto file = QFile{io::pathAsQPath(path)}; file.exists())
  {
    // closed automatically by destructor
    file.open(QFile::ReadOnly | QFile::Text);
    qApp->setStyleSheet(QTextStream{&file}.readAll());

    return true;
  }
  return false;
}

void TrenchBroomApp::loadStyle()
{
  // We can't use auto mnemonics in TrenchBroom. e.g. by default with Qt, Alt+D opens
  // the "Debug" menu, Alt+S activates the "Show default properties" checkbox in the
  // entity inspector. Flying with Alt held down and pressing WASD is a fundamental
  // behaviour in TB, so we can't have shortcuts randomly activating.
  //
  // Previously were calling `qt_set_sequence_auto_mnemonic(false);` in main(), but it
  // turns out we also need to suppress an Alt press followed by release from focusing
  // the menu bar (https://github.com/TrenchBroom/TrenchBroom/issues/3140), so the
  // following QProxyStyle disables that completely.

  class TrenchBroomProxyStyle : public QProxyStyle
  {
  public:
    explicit TrenchBroomProxyStyle(const QString& key)
      : QProxyStyle{key}
    {
    }

    explicit TrenchBroomProxyStyle(QStyle* style = nullptr)
      : QProxyStyle{style}
    {
    }

    int styleHint(
      StyleHint hint,
      const QStyleOption* option = nullptr,
      const QWidget* widget = nullptr,
      QStyleHintReturn* returnData = nullptr) const override
    {
      return hint == QStyle::SH_MenuBar_AltKeyNavigation
               ? 0
               : QProxyStyle::styleHint(hint, option, widget, returnData);
    }
  };

  // Apply either the Fusion style + dark palette, or the system style
  if (pref(Preferences::Theme) == Preferences::darkTheme())
  {
    setStyle(new TrenchBroomProxyStyle{"Fusion"});
    setPalette(darkPalette());
  }
  else
  {
    // System
    setStyle(new TrenchBroomProxyStyle{});
  }
}

std::vector<std::filesystem::path> TrenchBroomApp::recentDocuments() const
{
  return m_recentDocuments->recentDocuments();
}

void TrenchBroomApp::addRecentDocumentMenu(QMenu& menu)
{
  m_recentDocuments->addMenu(menu);
}

void TrenchBroomApp::removeRecentDocumentMenu(QMenu& menu)
{
  m_recentDocuments->removeMenu(menu);
}

void TrenchBroomApp::updateRecentDocument(const std::filesystem::path& path)
{
  m_recentDocuments->updatePath(path);
}

bool TrenchBroomApp::openDocument(const std::filesystem::path& path)
{
  // if std::filesystem::absolute fails, the file won't be found and we'll log it later
  auto ec = std::error_code{};
  const auto absPath =
    path.is_absolute() ? path : std::filesystem::absolute(path, ec).lexically_normal();

  const auto checkFileExists = [&]() {
    return fs::Disk::pathInfo(absPath) == fs::PathInfo::File
             ? Result<void>{}
             : Result<void>{Error{fmt::format("{} not found", path)}};
  };

  auto* frame = static_cast<MapFrame*>(nullptr);
  try
  {
    return checkFileExists() | kdl::or_else([&](const auto& e) {
             m_recentDocuments->removePath(absPath);
             return Result<void>{e};
           })
           | kdl::and_then([&]() {
               const auto gameNameAndMapFormat = detectOrQueryGameAndFormat(absPath);
               if (!gameNameAndMapFormat)
               {
                 return Result<bool>{false};
               }

               frame = m_frameManager->newFrame(m_taskManager);

               const auto [gameName, mapFormat] = *gameNameAndMapFormat;
               auto game = gameManager().createGame(gameName, frame->logger());
               contract_assert(game != nullptr);

               closeWelcomeWindow();

               return frame->openDocument(std::move(game), mapFormat, absPath);
             })
           | kdl::transform_error([&](const auto& e) {
               if (frame)
               {
                 frame->close();
               }
               QMessageBox::critical(
                 nullptr, "TrenchBroom", e.msg.c_str(), QMessageBox::Ok);
               return false;
             })
           | kdl::value();
  }
  catch (const std::exception& e)
  {
    if (frame)
    {
      frame->close();
    }
    QMessageBox::critical(nullptr, "TrenchBroom", e.what(), QMessageBox::Ok);
    return false;
  }
  catch (...)
  {
    if (frame)
    {
      frame->close();
    }
    throw;
  }
}

void TrenchBroomApp::openPreferences()
{
  auto dialog = PreferenceDialog{topDocument()};
  dialog.exec();
}

void TrenchBroomApp::openAbout()
{
  AboutDialog::showAboutDialog();
}

bool TrenchBroomApp::newDocument()
{
  auto* frame = static_cast<MapFrame*>(nullptr);
  try
  {
    const auto gameNameAndMapFormat = GameDialog::showNewDocumentDialog();
    if (!gameNameAndMapFormat)
    {
      return false;
    }

    const auto [gameName, mapFormat] = *gameNameAndMapFormat;

    frame = m_frameManager->newFrame(m_taskManager);

    auto game = gameManager().createGame(gameName, frame->logger());
    contract_assert(game != nullptr);

    closeWelcomeWindow();
    return frame->newDocument(std::move(game), mapFormat)
           | kdl::transform_error([&](auto e) {
               frame->close();

               QMessageBox::critical(nullptr, "", QString::fromStdString(e.msg));
               return false;
             })
           | kdl::value();
  }
  catch (const std::exception& e)
  {
    if (frame)
    {
      frame->close();
    }

    QMessageBox::critical(nullptr, "", e.what());
    return false;
  }
}

void TrenchBroomApp::openDocument()
{
  const auto pathStr = QFileDialog::getOpenFileName(
    nullptr,
    tr("Open Map"),
    fileDialogDefaultDirectory(FileDialogDir::Map),
    "Map files (*.map);;Any files (*.*)");

  if (const auto path = io::pathFromQString(pathStr); !path.empty())
  {
    updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::Map, pathStr);
    openDocument(path);
  }
}

void TrenchBroomApp::showManual()
{
  const auto manualPath = io::SystemPaths::findResourceFile("manual/index.html");
  const auto manualPathUrl = QUrl::fromLocalFile(io::pathAsQString(manualPath));
  QDesktopServices::openUrl(manualPathUrl);
}

void TrenchBroomApp::showPreferences()
{
  openPreferences();
}

void TrenchBroomApp::showAboutDialog()
{
  openAbout();
}

void TrenchBroomApp::debugShowCrashReportDialog()
{
  const auto reportPath = io::SystemPaths::userDataDirectory() / "crashreport.txt";
  const auto mapPath = io::SystemPaths::userDataDirectory() / "crashreport.map";
  const auto logPath = io::SystemPaths::userDataDirectory() / "crashreport.log";

  auto dialog = CrashDialog{"Debug crash", reportPath, mapPath, logPath};
  dialog.exec();
}

/**
 * If we catch exceptions in main() that are otherwise uncaught, Qt prints a warning
 * to override QCoreApplication::notify() and catch exceptions there instead.
 */
bool TrenchBroomApp::notify(QObject* receiver, QEvent* event)
{
  auto result = false;
  runWithCrashReporting([&]() { result = QApplication::notify(receiver, event); });
  return result;
}

#ifdef __APPLE__
bool TrenchBroomApp::event(QEvent* event)
{
  if (event->type() == QEvent::FileOpen)
  {
    const auto* openEvent = static_cast<QFileOpenEvent*>(event);
    const auto path = std::filesystem::path{openEvent->file().toStdString()};
    if (openDocument(path))
    {
      closeWelcomeWindow();
      return true;
    }
    return false;
  }
  else if (event->type() == QEvent::ApplicationActivate)
  {
    if (m_frameManager && m_frameManager->allFramesClosed())
    {
      showWelcomeWindow();
    }
  }
  return QApplication::event(event);
}
#endif

void TrenchBroomApp::openFilesOrWelcomeFrame(const QStringList& fileNames)
{
  const auto filesToOpen =
    useSDI() && !fileNames.empty() ? QStringList{fileNames.front()} : fileNames;

  auto anyDocumentOpened = false;
  for (const auto& fileName : filesToOpen)
  {
    const auto path = io::pathFromQString(fileName);
    if (!path.empty() && openDocument(path))
    {
      anyDocumentOpened = true;
    }
  }

  if (!anyDocumentOpened)
  {
    showWelcomeWindow();
  }
}

void TrenchBroomApp::showWelcomeWindow()
{
  if (!m_welcomeWindow)
  {
    // must be initialized after m_recentDocuments!
    m_welcomeWindow = std::make_unique<WelcomeWindow>();
  }
  m_welcomeWindow->show();
}

void TrenchBroomApp::closeWelcomeWindow()
{
  if (m_welcomeWindow)
  {
    m_welcomeWindow->close();
  }
}

MapDocument* TrenchBroomApp::topDocument()
{
  if (const auto* frameManager = this->frameManager())
  {
    if (auto* frame = frameManager->topFrame())
    {
      return &frame->document();
    }
  }
  return nullptr;
}

bool TrenchBroomApp::useSDI()
{
#ifdef _WIN32
  return true;
#else
  return false;
#endif
}

} // namespace tb::ui
