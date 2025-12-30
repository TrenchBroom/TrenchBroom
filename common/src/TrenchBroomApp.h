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

#pragma once

#include <QApplication>

#include "mdl/EnvironmentConfig.h"

#include "kd/task_manager.h"

#include <filesystem>
#include <memory>
#include <vector>

class QMenu;
class QNetworkAccessManager;
class QSettings;
class QTimer;

namespace upd
{
class HttpClient;
class Updater;
} // namespace upd

namespace tb
{
class Logger;

namespace mdl
{
struct EnvironmentConfig;
class GameManager;
} // namespace mdl

namespace ui
{
class AppController;
class FrameManager;
class MapDocument;
class RecentDocuments;
class WelcomeWindow;

class TrenchBroomApp : public QApplication
{
  Q_OBJECT
private:
  AppController* m_appController = nullptr;

public:
  static TrenchBroomApp& instance();

  TrenchBroomApp(int& argc, char** argv);
  ~TrenchBroomApp() override;

public:
  void askForAutoUpdates();
  void triggerAutoUpdateCheck();

  void parseCommandLineAndShowFrame();

  const AppController& appController() const;
  AppController& appController();

  const mdl::EnvironmentConfig environmentConfig() const;
  mdl::GameManager& gameManager();
  upd::Updater& updater();
  FrameManager* frameManager();

private:
  static QPalette darkPalette();
  bool loadStyleSheets();
  void loadStyle();

public:
  std::vector<std::filesystem::path> recentDocuments() const;
  std::vector<std::filesystem::path> recentDocuments();

  void addRecentDocumentMenu(QMenu& menu);
  void removeRecentDocumentMenu(QMenu& menu);
  void updateRecentDocument(const std::filesystem::path& path);

  bool openDocument(const std::filesystem::path& path);
  void openPreferences();
  void openAbout();

  bool newDocument();
  void openDocument();
  void showManual();
  void showPreferences();
  void showAboutDialog();
  void debugShowCrashReportDialog();

  bool notify(QObject* receiver, QEvent* event) override;

#ifdef __APPLE__
  bool event(QEvent* event) override;
#endif
  void openFilesOrWelcomeFrame(const QStringList& fileNames);

  void showWelcomeWindow();

  MapDocument* topDocument();
};

} // namespace ui
} // namespace tb
