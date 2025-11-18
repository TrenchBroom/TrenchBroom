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

#include "UpdateController.h"

#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>

#include "update/FileUtils.h"
#include "update/HttpClient.h"
#include "update/Logging.h"
#include "update/Overload.h"

namespace upd
{
namespace
{

QString describeState(const UpdateControllerState& state)
{
  return std::visit(
    overload(
      [](const IdleState&) { return "Idle"; },
      [](const CheckingForUpdatesState&) { return "Checking for updates"; },
      [](const UpdateAvailableState&) { return "Update available"; },
      [](const UpToDateState&) { return "Up to date"; },
      [](const PreparingUpdateState&) { return "Preparing update"; },
      [](const DownloadingUpdateState&) { return "Downloading update"; },
      [](const UpdatePendingState&) { return "Update pending"; },
      [](const UpdateErrorState&) { return "Update error"; },
      [](const UpdateDisabledState&) { return "Update disabled"; }),
    state);
}

} // namespace

bool operator==(const IdleState&, const IdleState&)
{
  return true;
}

bool operator!=(const IdleState& lhs, const IdleState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const IdleState&)
{
  return lhs << "IdleState{}";
}

bool operator==(const CheckingForUpdatesState&, const CheckingForUpdatesState&)
{
  return true;
}

bool operator!=(const CheckingForUpdatesState& lhs, const CheckingForUpdatesState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const CheckingForUpdatesState&)
{
  return lhs << "CheckingForUpdatesState{}";
}

bool operator==(const UpdateAvailableState& lhs, const UpdateAvailableState& rhs)
{
  return lhs.updateInfo == rhs.updateInfo;
}

bool operator!=(const UpdateAvailableState& lhs, const UpdateAvailableState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UpdateAvailableState& rhs)
{
  return lhs << "UpdateAvailableState{updateInfo: " << rhs.updateInfo << "}";
}

bool operator==(const UpToDateState& lhs, const UpToDateState& rhs)
{
  return lhs.currentVersion == rhs.currentVersion;
}

bool operator!=(const UpToDateState& lhs, const UpToDateState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UpToDateState& rhs)
{
  return lhs << "UpToDateState{currentVersion: " << rhs.currentVersion.toStdString()
             << "}";
}

bool operator==(const DownloadingUpdateState&, const DownloadingUpdateState&)
{
  return true;
}

bool operator!=(const DownloadingUpdateState& lhs, const DownloadingUpdateState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const DownloadingUpdateState&)
{
  return lhs << "DownloadingUpdateState{}";
}

bool operator==(const PreparingUpdateState&, const PreparingUpdateState&)
{
  return true;
}

bool operator!=(const PreparingUpdateState& lhs, const PreparingUpdateState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const PreparingUpdateState&)
{
  return lhs << "PreparingUpdateState{}";
}

bool operator==(const UpdatePendingState& lhs, const UpdatePendingState& rhs)
{
  return lhs.preparedUpdatePath == rhs.preparedUpdatePath
         && lhs.requiresAdminPrivileges == rhs.requiresAdminPrivileges
         && lhs.restartApp == rhs.restartApp;
}

bool operator!=(const UpdatePendingState& lhs, const UpdatePendingState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UpdatePendingState& rhs)
{
  return lhs << "UpdatePendingState{preparedUpdatePath: "
             << rhs.preparedUpdatePath.toStdString()
             << ", requiresAdminPrivileges: " << rhs.requiresAdminPrivileges
             << ", restartApp: " << rhs.restartApp << "}";
}

bool operator==(const UpdateErrorState& lhs, const UpdateErrorState& rhs)
{
  return lhs.errorMessage == rhs.errorMessage;
}

bool operator!=(const UpdateErrorState& lhs, const UpdateErrorState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UpdateErrorState& rhs)
{
  return lhs << "UpdateErrorState{errorMessage: " << rhs.errorMessage.toStdString()
             << "}";
}

bool operator==(const UpdateDisabledState&, const UpdateDisabledState&)
{
  return true;
}

bool operator!=(const UpdateDisabledState& lhs, const UpdateDisabledState& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UpdateDisabledState&)
{
  return lhs << "UpdateDisabledState{}";
}

std::ostream& operator<<(std::ostream& lhs, const UpdateControllerState& rhs)
{
  std::visit([&](const auto& state) { lhs << state; }, rhs);
  return lhs;
}

UpdateController::UpdateController(
  const HttpClient& httpClient, std::optional<UpdateConfig> config, QObject* parent)
  : QObject{parent}
  , m_httpClient{httpClient}
  , m_config{std::move(config)}
  , m_state{
      m_config ? UpdateControllerState{IdleState{}}
               : UpdateControllerState{UpdateDisabledState{}}}
{
  if (m_config)
  {
    if (auto logFile = QFile{m_config->logFilePath}; logFile.exists())
    {
      logFile.remove();
    }
  }

  log("Initializing updater");

  if (m_config)
  {
    log(describeUpdateConfig(*m_config));
  }
  else
  {
    log("Updater disabled");
  }
}

UpdateController::~UpdateController()
{
  if (const auto* updatePendingState = std::get_if<UpdatePendingState>(&m_state))
  {
    log(tr("Installing update"));
    m_config->installUpdate(
      updatePendingState->preparedUpdatePath, *m_config, updatePendingState->restartApp);
  }
}

void UpdateController::checkForUpdates()
{
  if (m_config)
  {
    m_config->checkForUpdates(*this);
  }
}

void UpdateController::downloadAndPrepareUpdate()
{
  using namespace std::chrono_literals;

  std::visit(
    overload(
      [&](const UpdateAvailableState& updateAvailableState) {
        log(tr("Cleaning work directory %1").arg(m_config->workDirPath));
        if (!cleanDirectory(m_config->workDirPath))
        {
          log(tr("Failed to clean work directory"));
          setState(UpdateErrorState{"Failed to clean work directory"});
          return;
        }

        log(tr("Downloading update from %1")
              .arg(updateAvailableState.updateInfo.asset.url.toString()));

        auto op = downloadAsset(
          m_httpClient,
          updateAvailableState.updateInfo.asset,
          [&](const auto& file) {
            const auto downloadedUpdatePath = file.fileName();

            auto* watcher = new QFutureWatcher<UpdateControllerState>{this};
            connect(
              watcher,
              &QFutureWatcher<UpdateControllerState>::finished,
              this,
              [this, downloadedUpdatePath, watcher]() {
                setState(watcher->result());
                log(tr("Removing downloaded update file %1").arg(downloadedUpdatePath));
                QFile{downloadedUpdatePath}.remove();

                watcher->deleteLater();
              });

            log(tr("Preparing update file %1").arg(downloadedUpdatePath));
            watcher->setFuture(QtConcurrent::run([=, this]() -> UpdateControllerState {
              if (
                const auto preparedUpdatePath =
                  m_config->prepareUpdate(downloadedUpdatePath, *m_config))
              {
                return UpdatePendingState{
                  *preparedUpdatePath, m_config->requiresAdminPrivileges};
              }
              return UpdateErrorState{"Failed to prepare update file"};
            }));

            setState(PreparingUpdateState{});
          },
          [&](const QString& error) { setState(UpdateErrorState{error}); });
        setState(DownloadingUpdateState{std::move(op)});
      },
      [](const auto&) {}),
    m_state);
}

void UpdateController::cancelPendingOperation()
{
  std::visit(
    overload(
      [&](CheckingForUpdatesState& state) {
        state.pendingOperation->cancel();
        setState(IdleState{});
      },
      [&](DownloadingUpdateState& state) {
        state.pendingOperation->cancel();
        setState(IdleState{});
      },
      [](auto&) {}),
    m_state);
}

void UpdateController::reset()
{
  std::visit(
    overload(
      [&](const IdleState&) {},
      [&](const CheckingForUpdatesState&) { cancelPendingOperation(); },
      [&](const DownloadingUpdateState&) { cancelPendingOperation(); },
      [&](const auto&) { setState(IdleState{}); }),
    m_state);
}

void UpdateController::setRestartApp(const bool restartApp)
{
  if (auto* updatePendingState = std::get_if<UpdatePendingState>(&m_state))
  {
    updatePendingState->restartApp = restartApp;
  }
}

const UpdateControllerState& UpdateController::state() const
{
  return m_state;
}

void UpdateController::setState(UpdateControllerState state)
{
  m_state = std::move(state);
  log(tr("State changed: %1").arg(describeState(m_state)));

  emit stateChanged(m_state);
}

void UpdateController::log(const QString& msg)
{
  if (m_config)
  {
    logToFile(m_config->logFilePath, msg);
  }
}

} // namespace upd
