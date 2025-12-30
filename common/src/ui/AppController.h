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

#pragma once

#include <QObject>

#include "Result.h"

#include <filesystem>
#include <memory>

class QMenu;
class QNetworkAccessManager;
class QTimer;

namespace kdl
{
class task_manager;
}

namespace upd
{
class HttpClient;
class Updater;
} // namespace upd

namespace tb
{
namespace mdl
{
class GameManager;
struct EnvironmentConfig;
} // namespace mdl

namespace ui
{
class AboutDialog;
class FrameManager;
class RecentDocuments;
class WelcomeWindow;

class AppController : public QObject
{
  Q_OBJECT
private:
  std::unique_ptr<kdl::task_manager> m_taskManager;
  std::unique_ptr<mdl::EnvironmentConfig> m_environmentConfig;
  std::unique_ptr<mdl::GameManager> m_gameManager;

  QNetworkAccessManager* m_networkManager = nullptr;
  QTimer* m_recentDocumentsReloadTimer = nullptr;

  upd::HttpClient* m_httpClient = nullptr;
  upd::Updater* m_updater = nullptr;

  FrameManager* m_frameManager = nullptr;
  RecentDocuments* m_recentDocuments = nullptr;
  std::unique_ptr<WelcomeWindow> m_welcomeWindow;
  std::unique_ptr<AboutDialog> m_aboutDialog;

private:
  AppController(
    std::unique_ptr<kdl::task_manager> taskManager,
    std::unique_ptr<mdl::EnvironmentConfig> environmentConfig,
    std::unique_ptr<mdl::GameManager> gameManager,
    QObject* parent);

public:
  static constexpr auto useSDI =
#ifdef _WIN32
    true;
#else
    false;
#endif

  static Result<AppController*> create(QObject* parent);

  ~AppController() override;

  kdl::task_manager& taskManager();

  const mdl::EnvironmentConfig& environmentConfig() const;

  mdl::GameManager& gameManager();

  upd::Updater& updater();

  FrameManager& frameManager();

  const RecentDocuments& recentDocuments() const;
  RecentDocuments& recentDocuments();

  void askForAutoUpdates();
  void triggerAutoUpdateCheck();

  bool newDocument();
  void openDocument();
  bool openDocument(const std::filesystem::path& path);

  void showWelcomeWindow();

  void showManual();
  void showPreferences();
  void showAboutDialog();

private:
  void connectObservers();
};

} // namespace ui
} // namespace tb
