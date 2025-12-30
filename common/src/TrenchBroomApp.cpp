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
#include "mdl/GameManager.h"
#include "mdl/Map.h"
#include "ui/AboutDialog.h"
#include "ui/ActionExecutionContext.h"
#include "ui/AppController.h"
#include "ui/CrashDialog.h"
#include "ui/CrashReporter.h"
#include "ui/FrameManager.h"
#include "ui/GameDialog.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"
#include "ui/PreferenceDialog.h"
#include "ui/QPathUtils.h"
#include "ui/RecentDocuments.h"
#include "ui/SystemPaths.h"
#include "ui/WelcomeWindow.h"
#include "update/Updater.h"

#include "kd/const_overload.h"
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

#include <csignal>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

namespace tb::ui
{
namespace
{

auto createAppController(QObject* parent)
{
  return AppController::create(parent) | kdl::if_error([](auto e) {
           const auto msg =
             fmt::format(R"(Game configurations could not be loaded: {})", e.msg);

           QMessageBox::critical(
             nullptr, "TrenchBroom", QString::fromStdString(msg), QMessageBox::Ok);
           QCoreApplication::exit(1);
         })
         | kdl::value();
}

} // namespace

TrenchBroomApp& TrenchBroomApp::instance()
{
  return *dynamic_cast<TrenchBroomApp*>(qApp);
}

TrenchBroomApp::TrenchBroomApp(int& argc, char** argv)
  : QApplication{argc, argv}
  , m_appController{createAppController(this)}
{
  using namespace std::chrono_literals;

  // When this flag is enabled, font and palette changes propagate as though the user
  // had manually called the corresponding QWidget methods.
  setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);

  // Don't show icons in menus, they are scaled down and don't look very good.
  setAttribute(Qt::AA_DontShowIconsInMenus);

  setupCrashReporter();

  loadStyleSheets();
  loadStyle();

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
  // destroy preference manager before app
  PreferenceManager::destroyInstance();
}

void TrenchBroomApp::askForAutoUpdates()
{
  appController().askForAutoUpdates();
}

void TrenchBroomApp::triggerAutoUpdateCheck()
{
  appController().triggerAutoUpdateCheck();
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

const AppController& TrenchBroomApp::appController() const
{
  return *m_appController;
}

AppController& TrenchBroomApp::appController()
{
  return KDL_CONST_OVERLOAD(appController());
}

const mdl::EnvironmentConfig TrenchBroomApp::environmentConfig() const
{
  return appController().environmentConfig();
}

mdl::GameManager& TrenchBroomApp::gameManager()
{
  return appController().gameManager();
}


upd::Updater& TrenchBroomApp::updater()
{
  return appController().updater();
}

FrameManager* TrenchBroomApp::frameManager()
{
  return &appController().frameManager();
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
  const auto path = ui::SystemPaths::findResourceFile("stylesheets/base.qss");
  if (auto file = QFile{ui::pathAsQPath(path)}; file.exists())
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
  if (pref(Preferences::Theme) == Preferences::DarkTheme)
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
  return appController().recentDocuments().recentDocuments();
}

std::vector<std::filesystem::path> TrenchBroomApp::recentDocuments()
{
  return KDL_CONST_OVERLOAD(recentDocuments());
}

void TrenchBroomApp::addRecentDocumentMenu(QMenu& menu)
{
  appController().recentDocuments().addMenu(menu);
}

void TrenchBroomApp::removeRecentDocumentMenu(QMenu& menu)
{
  appController().recentDocuments().removeMenu(menu);
}

void TrenchBroomApp::updateRecentDocument(const std::filesystem::path& path)
{
  appController().recentDocuments().updatePath(path);
}

bool TrenchBroomApp::openDocument(const std::filesystem::path& path)
{
  return appController().openDocument(path);
}

void TrenchBroomApp::openPreferences()
{
  appController().showPreferences();
}

void TrenchBroomApp::openAbout()
{
  appController().showAboutDialog();
}

bool TrenchBroomApp::newDocument()
{
  return appController().newDocument();
}

void TrenchBroomApp::openDocument()
{
  appController().openDocument();
}

void TrenchBroomApp::showManual()
{
  appController().showManual();
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
  const auto reportPath = ui::SystemPaths::userDataDirectory() / "crashreport.txt";
  const auto mapPath = ui::SystemPaths::userDataDirectory() / "crashreport.map";
  const auto logPath = ui::SystemPaths::userDataDirectory() / "crashreport.log";

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
      return true;
    }
    return false;
  }
  else if (event->type() == QEvent::ApplicationActivate)
  {
    if (appController().frameManager().allFramesClosed())
    {
      showWelcomeWindow();
    }
  }
  return QApplication::event(event);
}
#endif

void TrenchBroomApp::openFilesOrWelcomeFrame(const QStringList& fileNames)
{
  const auto filesToOpen = AppController::useSDI && !fileNames.empty()
                             ? QStringList{fileNames.front()}
                             : fileNames;

  auto anyDocumentOpened = false;
  for (const auto& fileName : filesToOpen)
  {
    const auto path = ui::pathFromQString(fileName);
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
  appController().showWelcomeWindow();
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

} // namespace tb::ui
