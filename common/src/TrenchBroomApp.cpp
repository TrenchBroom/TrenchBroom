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
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"
#include "ui/PreferenceDialog.h"
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
