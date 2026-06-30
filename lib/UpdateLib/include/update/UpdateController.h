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

#include "update/GithubApi.h"
#include "update/Release.h"
#include "update/UpdateConfig.h"
#include "update/UpdateInfo.h"
#include "update/Version.h"

#include <functional>
#include <optional>
#include <variant>

namespace upd
{

class HttpClient;
class HttpOperation;

/** The update controller has not performed an update check yet when in this state. */
struct IdleState
{
  friend bool operator==(const IdleState& lhs, const IdleState& rhs);
  friend bool operator!=(const IdleState& lhs, const IdleState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const IdleState& rhs);
};

/** The update controller is currelty checking for updates. */
struct CheckingForUpdatesState
{
  HttpOperation* pendingOperation;

  friend bool operator==(
    const CheckingForUpdatesState& lhs, const CheckingForUpdatesState& rhs);
  friend bool operator!=(
    const CheckingForUpdatesState& lhs, const CheckingForUpdatesState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const CheckingForUpdatesState& rhs);
};

/** An update is available and the update controller is waiting for user input. */
struct UpdateAvailableState
{
  UpdateInfo updateInfo;

  friend bool operator==(
    const UpdateAvailableState& lhs, const UpdateAvailableState& rhs);
  friend bool operator!=(
    const UpdateAvailableState& lhs, const UpdateAvailableState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const UpdateAvailableState& rhs);
};

/** No update is available. */
struct UpToDateState
{
  QString currentVersion;

  friend bool operator==(const UpToDateState& lhs, const UpToDateState& rhs);
  friend bool operator!=(const UpToDateState& lhs, const UpToDateState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const UpToDateState& rhs);
};

/** The update controller is downloading an update. */
struct DownloadingUpdateState
{
  HttpOperation* pendingOperation;

  friend bool operator==(
    const DownloadingUpdateState& lhs, const DownloadingUpdateState& rhs);
  friend bool operator!=(
    const DownloadingUpdateState& lhs, const DownloadingUpdateState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const DownloadingUpdateState& rhs);
};

/** The update controller is preparing an update. */
struct PreparingUpdateState
{
  friend bool operator==(
    const PreparingUpdateState& lhs, const PreparingUpdateState& rhs);
  friend bool operator!=(
    const PreparingUpdateState& lhs, const PreparingUpdateState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const PreparingUpdateState& rhs);
};
;

/** An update is ready to be installed when the application quits. */
struct UpdatePendingState
{
  QString preparedUpdatePath;
  bool requiresAdminPrivileges;
  bool restartApp = false;

  friend bool operator==(const UpdatePendingState& lhs, const UpdatePendingState& rhs);
  friend bool operator!=(const UpdatePendingState& lhs, const UpdatePendingState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const UpdatePendingState& rhs);
};

/** An error occurred. */
struct UpdateErrorState
{
  QString errorMessage;

  friend bool operator==(const UpdateErrorState& lhs, const UpdateErrorState& rhs);
  friend bool operator!=(const UpdateErrorState& lhs, const UpdateErrorState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const UpdateErrorState& rhs);
};

/** The update controller is disabled because it was not configured. */
struct UpdateDisabledState
{
  friend bool operator==(const UpdateDisabledState& lhs, const UpdateDisabledState& rhs);
  friend bool operator!=(const UpdateDisabledState& lhs, const UpdateDisabledState& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const UpdateDisabledState& rhs);
};

using UpdateControllerState = std::variant<
  IdleState,
  CheckingForUpdatesState,
  UpdateAvailableState,
  UpToDateState,
  PreparingUpdateState,
  DownloadingUpdateState,
  UpdatePendingState,
  UpdateErrorState,
  UpdateDisabledState>;

std::ostream& operator<<(std::ostream& lhs, const UpdateControllerState& rhs);

/**
 * The update controller is the central part of the updater. It implements the state
 * management and thereby the update process.
 */
class UpdateController : public QObject
{
  Q_OBJECT

public:
  template <typename Version>
  using CheckForUpdatesCallback = std::function<void(std::optional<Release<Version>>)>;

private:
  const HttpClient& m_httpClient;
  std::optional<UpdateConfig> m_config;

  UpdateControllerState m_state;

public:
  /** Creates a new update controller using the given HTTP client and config. Passing
   * std::nullopt as the config disables the updater. */
  explicit UpdateController(
    const HttpClient& httpClient,
    std::optional<UpdateConfig> config,
    QObject* parent = nullptr);

  ~UpdateController() override;

  /** Check for a new update. Only has an effect while the update controller is in the
   * idle or error state. */
  void checkForUpdates();

  /** The implementation of the update check. */
  template <typename Version>
  void checkForUpdates(
    const Version& currentVersion,
    bool includePreReleases,
    bool includeDraftReleases,
    ParseVersion<Version> parseVersion,
    DescribeVersion<Version> describeVersion_,
    ChooseAsset chooseAsset_)
  {
    if (
      std::holds_alternative<IdleState>(m_state)
      || std::holds_alternative<UpdateErrorState>(m_state))
    {
      auto* pendingOperation = getLatestRelease<Version>(
        m_httpClient,
        m_config->ghOrgName,
        m_config->ghRepoName,
        currentVersion,
        includePreReleases,
        includeDraftReleases,
        std::move(parseVersion),
        [this,
         currentVersion,
         describeVersion = std::move(describeVersion_),
         chooseAsset = std::move(chooseAsset_)](auto release) {
          if (release)
          {
            if (
              const auto updateInfo = makeUpdateInfo<Version>(
                currentVersion, *release, describeVersion, chooseAsset))
            {
              setState(UpdateAvailableState{std::move(*updateInfo)});
            }
            else
            {
              setState(UpdateErrorState{"No suitable asset found"});
            }
          }
          else
          {
            setState(UpToDateState{describeVersion(currentVersion)});
          }
        },
        [&](const auto& error) { setState(UpdateErrorState{error}); });
      setState(CheckingForUpdatesState{pendingOperation});
    }
  }

  /** If an update is available, download and prepare it. Has no effect if no update is
   * available. */
  void downloadAndPrepareUpdate();

  /** Cancel an ongoing update check or download. Has no effect otherwise. */
  void cancelPendingOperation();

  /** Reset the update controller to the idle state. */
  void reset();

  /** Set whether the application should be restarted after the update is installed. */
  void setRestartApp(bool restartApp);

  const UpdateControllerState& state() const;

signals:
  void stateChanged(const UpdateControllerState& state);

private:
  void setState(UpdateControllerState state);

  void log(const QString& message);
};

} // namespace upd
