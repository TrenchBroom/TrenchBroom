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

#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Result.h"
#include "TrenchBroomStackWalker.h"
#include "io/DiskIO.h"
#include "io/MapHeader.h"
#include "io/PathInfo.h"
#include "io/PathQt.h"
#include "io/SystemPaths.h"
#include "mdl/GameFactory.h"
#include "mdl/MapFormat.h"
#include "ui/AboutDialog.h"
#include "ui/Actions.h"
#include "ui/CrashDialog.h"
#include "ui/FrameManager.h"
#include "ui/GLContextManager.h"
#include "ui/GameDialog.h"
#include "ui/GetVersion.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"
#include "ui/PreferenceDialog.h"
#include "ui/QtUtils.h"
#include "ui/RecentDocuments.h"
#include "ui/WelcomeWindow.h"

#include "kdl/vector_utils.h"
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
#include <QPalette>
#include <QProxyStyle>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTimer>
#include <QUrl>

#include "kdl/path_utils.h"
#include "kdl/string_utils.h"

#include <fmt/format.h>

#include <chrono>
#include <clocale>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif

namespace tb::ui
{
namespace
{

// returns the topmost MapDocument as a shared pointer, or the empty shared pointer
std::shared_ptr<MapDocument> topDocument()
{
  if (const auto* frameManager = TrenchBroomApp::instance().frameManager())
  {
    if (const auto* frame = frameManager->topFrame())
    {
      return frame->document();
    }
  }
  return {};
}

std::optional<std::tuple<std::string, mdl::MapFormat>> detectOrQueryGameAndFormat(
  const std::filesystem::path& path)
{
  return io::Disk::withInputStream(path, io::readMapHeader)
         | kdl::transform(
           [&](auto detectedGameNameAndMapFormat)
             -> std::optional<std::tuple<std::string, mdl::MapFormat>> {
             auto [gameName, mapFormat] = detectedGameNameAndMapFormat;
             const auto& gameFactory = mdl::GameFactory::instance();

             if (
               gameName == std::nullopt
               || !kdl::vec_contains(gameFactory.gameList(), *gameName)
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

#if defined(_WIN32) && defined(_MSC_VER)
LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs);
#else
[[noreturn]] static void CrashHandler(int signum);
#endif

TrenchBroomApp::TrenchBroomApp(int& argc, char** argv)
  : QApplication{argc, argv}
  , m_taskManager{std::thread::hardware_concurrency()}
{
  using namespace std::chrono_literals;

  // When this flag is enabled, font and palette changes propagate as though the user
  // had manually called the corresponding QWidget methods.
  setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);

  // Don't show icons in menus, they are scaled down and don't look very good.
  setAttribute(Qt::AA_DontShowIconsInMenus);

#if defined(_WIN32) && defined(_MSC_VER)
  // with MSVC, set our own handler for segfaults so we can access the context
  // pointer, to allow StackWalker to read the backtrace.
  // see also: http://crashrpt.sourceforge.net/docs/html/exception_handling.html
  SetUnhandledExceptionFilter(TrenchBroomUnhandledExceptionFilter);
#else
  signal(SIGSEGV, CrashHandler);
#endif

  // always set this locale so that we can properly parse floats from text files
  // regardless of the platforms locale
  std::setlocale(LC_NUMERIC, "C");

  setApplicationName("TrenchBroom");
  // Needs to be "" otherwise Qt adds this to the paths returned by QStandardPaths
  // which would cause preferences to move from where they were with wx
  setOrganizationName("");
  setOrganizationDomain("io.github.trenchbroom");

  if (!initializeGameFactory())
  {
    QCoreApplication::exit(1);
    return;
  }

  loadStyleSheets();
  loadStyle();

  // these must be initialized here and not earlier
  m_frameManager = std::make_unique<FrameManager>(useSDI());

  m_recentDocuments = std::make_unique<RecentDocuments>(
    10, [](const auto& path) { return std::filesystem::exists(path); });
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

void TrenchBroomApp::parseCommandLineAndShowFrame()
{
  auto parser = QCommandLineParser{};
  parser.addOption(QCommandLineOption("portable"));
  parser.process(*this);
  openFilesOrWelcomeFrame(parser.positionalArguments());
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
  if (auto file = QFile{io::pathAsQString(path)}; file.exists())
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
  const auto absPath =
    path.is_absolute() ? path : std::filesystem::absolute(path).lexically_normal();

  const auto checkFileExists = [&]() {
    return io::Disk::pathInfo(absPath) == io::PathInfo::File
             ? Result<void>{}
             : Result<void>{Error{"'" + path.string() + "' not found"}};
  };

  auto* frame = static_cast<MapFrame*>(nullptr);
  try
  {
    auto& gameFactory = mdl::GameFactory::instance();
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

               auto [gameName, mapFormat] = *gameNameAndMapFormat;
               auto game = gameFactory.createGame(gameName, frame->logger());
               ensure(game.get() != nullptr, "game is null");

               closeWelcomeWindow();

               return frame->openDocument(game, mapFormat, absPath);
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
  catch (const Exception& e)
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

bool TrenchBroomApp::initializeGameFactory()
{
  const auto gamePathConfig = mdl::GamePathConfig{
    io::SystemPaths::findResourceDirectories("games"),
    io::SystemPaths::userDataDirectory() / "games",
  };
  auto& gameFactory = mdl::GameFactory::instance();
  return gameFactory.initialize(gamePathConfig) | kdl::transform([](auto errors) {
           if (!errors.empty())
           {
             const auto msg = fmt::format(
               R"(Some game configurations could not be loaded. The following errors occurred:

{})",
               kdl::str_join(errors, "\n\n"));

             QMessageBox::critical(
               nullptr, "TrenchBroom", QString::fromStdString(msg), QMessageBox::Ok);
           }
         })
         | kdl::if_error([](auto e) { qCritical() << QString::fromStdString(e.msg); })
         | kdl::is_success();
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

    auto& gameFactory = mdl::GameFactory::instance();
    auto game = gameFactory.createGame(gameName, frame->logger());
    ensure(game.get() != nullptr, "game is null");

    closeWelcomeWindow();
    return frame->newDocument(game, mapFormat) | kdl::transform_error([&](auto e) {
             frame->close();

             QMessageBox::critical(nullptr, "", QString::fromStdString(e.msg));
             return false;
           })
           | kdl::value();
  }
  catch (const Exception& e)
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
  const auto manualPathString = manualPath.string();
  const auto manualPathUrl =
    QUrl::fromLocalFile(QString::fromStdString(manualPathString));
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
 * If we catch exceptions in main() that are otherwise uncaught, Qt prints a warning to
 * override QCoreApplication::notify() and catch exceptions there instead.
 */
bool TrenchBroomApp::notify(QObject* receiver, QEvent* event)
{
#ifdef _MSC_VER
  __try
  {
    return QApplication::notify(receiver, event);

    // We have to choose between capturing the stack trace (using __try/__except) and
    // getting the C++ exception object (using C++ try/catch) - take the stack trace.
  }
  __except (TrenchBroomUnhandledExceptionFilter(GetExceptionInformation()))
  {
    // Unreachable, see TrenchBroomUnhandledExceptionFilter
    return false;
  }
#else
  try
  {
    return QApplication::notify(receiver, event);
  }
  catch (const std::exception& e)
  {
    // Unfortunately we can't portably get the stack trace of the exception itself
    tb::ui::reportCrashAndExit("<uncaught exception>", e.what());
  }
#endif
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

bool TrenchBroomApp::useSDI()
{
#ifdef _WIN32
  return true;
#else
  return false;
#endif
}


namespace
{
std::string makeCrashReport(const std::string& stacktrace, const std::string& reason)
{
  auto ss = std::stringstream{};
  ss << "OS:\t" << QSysInfo::prettyProductName().toStdString() << std::endl;
  ss << "Qt:\t" << qVersion() << std::endl;
  ss << "GL_VENDOR:\t" << GLContextManager::GLVendor << std::endl;
  ss << "GL_RENDERER:\t" << GLContextManager::GLRenderer << std::endl;
  ss << "GL_VERSION:\t" << GLContextManager::GLVersion << std::endl;
  ss << "TrenchBroom Version:\t" << getBuildVersion().toStdString() << std::endl;
  ss << "TrenchBroom Build:\t" << getBuildIdStr().toStdString() << std::endl;
  ss << "Reason:\t" << reason << std::endl;
  ss << "Stack trace:" << std::endl;
  ss << stacktrace << std::endl;
  return ss.str();
}

// returns the empty path for unsaved maps, or if we can't determine the current map
std::filesystem::path savedMapPath()
{
  const auto document = topDocument();
  return document && document->path().is_absolute() ? document->path()
                                                    : std::filesystem::path{};
}

std::filesystem::path crashReportBasePath()
{
  const auto mapPath = savedMapPath();
  const auto crashLogPath = !mapPath.empty()
                              ? mapPath.parent_path() / mapPath.stem() += "-crash.txt"
                              : io::pathFromQString(QStandardPaths::writableLocation(
                                  QStandardPaths::DocumentsLocation))
                                  / "trenchbroom-crash.txt";

  // ensure it doesn't exist
  auto index = 0;
  auto testCrashLogPath = crashLogPath;
  while (io::Disk::pathInfo(testCrashLogPath) == io::PathInfo::File)
  {
    ++index;

    const auto testCrashLogName =
      fmt::format("{}-{}.txt", crashLogPath.stem().string(), index);
    testCrashLogPath = crashLogPath.parent_path() / testCrashLogName;
  }

  return kdl::path_remove_extension(testCrashLogPath);
}

bool inReportCrashAndExit = false;
bool crashReportGuiEnabled = true;

} // namespace

void setCrashReportGUIEnbled(const bool guiEnabled)
{
  crashReportGuiEnabled = guiEnabled;
}

void reportCrashAndExit(const std::string& stacktrace, const std::string& reason)
{
  // just abort if we reenter reportCrashAndExit (i.e. if it crashes)
  if (inReportCrashAndExit)
  {
    std::abort();
  }

  inReportCrashAndExit = true;

  // get the crash report as a string
  const auto report = makeCrashReport(stacktrace, reason);

  // write it to the crash log file
  const auto basePath = crashReportBasePath();

  // ensure the containing directory exists
  io::Disk::createDirectory(basePath.parent_path()) | kdl::transform([&](auto) {
    const auto reportPath = kdl::path_add_extension(basePath, ".txt");
    auto logPath = kdl::path_add_extension(basePath, ".log");
    auto mapPath = kdl::path_add_extension(basePath, ".map");

    io::Disk::withOutputStream(reportPath, [&](auto& stream) {
      stream << report;
      std::cerr << "wrote crash log to " << reportPath.string() << std::endl;
    }) | kdl::transform_error([](const auto& e) {
      std::cerr << "could not write crash log: " << e.msg << std::endl;
    });

    // save the map
    auto doc = topDocument();
    if (doc.get() && doc->game())
    {
      doc->saveDocumentTo(mapPath);
      std::cerr << "wrote map to " << mapPath.string() << std::endl;
    }
    else
    {
      mapPath = std::filesystem::path{};
    }

    // Copy the log file
    if (!QFile::copy(
          io::pathAsQString(io::SystemPaths::logFilePath()), io::pathAsQString(logPath)))
    {
      logPath = std::filesystem::path{};
    }

    if (crashReportGuiEnabled)
    {
      auto dialog = CrashDialog{reason, reportPath, mapPath, logPath};
      dialog.exec();
    }
  }) | kdl::transform_error([](const auto& e) {
    std::cerr << "could not create crash folder: " << e.msg << std::endl;
  });

  // write the crash log to stderr
  std::cerr << "crash log:" << std::endl;
  std::cerr << report << std::endl;

  std::abort();
}

bool isReportingCrash()
{
  return inReportCrashAndExit;
}

#if defined(_WIN32) && defined(_MSC_VER)
LONG WINAPI TrenchBroomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs)
{
  reportCrashAndExit(
    TrenchBroomStackWalker::getStackTraceFromContext(pExceptionPtrs->ContextRecord),
    std::to_string(pExceptionPtrs->ExceptionRecord->ExceptionCode));
  // return EXCEPTION_EXECUTE_HANDLER; unreachable
}
#else
static void CrashHandler(int /* signum */)
{
  tb::ui::reportCrashAndExit(tb::TrenchBroomStackWalker::getStackTrace(), "SIGSEGV");
}
#endif

} // namespace tb::ui
