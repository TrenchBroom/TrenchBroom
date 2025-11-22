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

namespace ui
{
class FrameManager;
class MapDocument;
class RecentDocuments;
class WelcomeWindow;

class TrenchBroomApp : public QApplication
{
  Q_OBJECT
private:
  QNetworkAccessManager* m_networkManager = nullptr;
  upd::HttpClient* m_httpClient = nullptr;
  upd::Updater* m_updater = nullptr;
  kdl::task_manager m_taskManager = kdl::task_manager{256};
  std::unique_ptr<FrameManager> m_frameManager;
  std::unique_ptr<RecentDocuments> m_recentDocuments;
  std::unique_ptr<WelcomeWindow> m_welcomeWindow;
  QTimer* m_recentDocumentsReloadTimer = nullptr;

public:
  static TrenchBroomApp& instance();

  TrenchBroomApp(int& argc, char** argv);
  ~TrenchBroomApp() override;

public:
  void askForAutoUpdates();
  void triggerAutoUpdateCheck();

  void parseCommandLineAndShowFrame();

  upd::Updater& updater();
  FrameManager* frameManager();

private:
  static QPalette darkPalette();
  bool loadStyleSheets();
  void loadStyle();

public:
  std::vector<std::filesystem::path> recentDocuments() const;
  void addRecentDocumentMenu(QMenu& menu);
  void removeRecentDocumentMenu(QMenu& menu);
  void updateRecentDocument(const std::filesystem::path& path);

  bool openDocument(const std::filesystem::path& path);
  void openPreferences();
  void openAbout();
  bool initializeGameFactory();

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
  void closeWelcomeWindow();

  MapDocument* topDocument();

private:
  static bool useSDI();
signals:
  void recentDocumentsDidChange();
};

} // namespace ui
} // namespace tb
