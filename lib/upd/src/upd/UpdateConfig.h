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

#include <QString>

#include <functional>

namespace upd
{

class UpdateController;
struct UpdateConfig;

using CheckForUpdates = std::function<void(UpdateController&)>;
using PrepareUpdate = std::function<std::optional<QString>(
  const QString& downloadedUpdatePath, const UpdateConfig& updateConfig)>;
using InstallUpdate = std::function<void(
  const QString& preparedUpdatePath, const UpdateConfig&, const bool restartApp)>;

/**
 * Configuration for the update process.
 */
struct UpdateConfig
{
  /** A function that performs the update check by calling
   * UpdateController::checkForUpdates with the appropriate parameters. */
  CheckForUpdates checkForUpdates;
  /** A function that prepares a downloaded update for installation. */
  PrepareUpdate prepareUpdate;
  /** A function that installs a prepared update. */
  InstallUpdate installUpdate;

  /** The GitHub organization name. */
  QString ghOrgName;
  /** The GitHub repository name. */
  QString ghRepoName;
  /** The path to the update script. This script is invoked by the application when it
   * terminates while an update is pending. */
  QString updateScriptPath;
  /**
   * The path to the folder containing the application.
   * On Windows, this is the folder containing the application executable. On macOS, this
   * is the app bundle, and on Linux, this is the AppImage.
   */
  QString appFolderPath;
  /** The relative path to the executable in the app folder. This is used to restart the
   * app after the update was installed. */
  QString relativeAppPath;
  /** The path to a working directory to use when preparing the update. The update script
   * will also be copied here. */
  QString workDirPath;
  /** The path to the log file. */
  QString logFilePath;
};

QString describeUpdateConfig(const UpdateConfig& config);

} // namespace upd
