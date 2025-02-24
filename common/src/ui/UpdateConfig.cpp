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

#include "UpdateConfig.h"

#include <QProcess>

#include "Macros.h" // IWYU pragma: keep
#include "PreferenceManager.h"
#include "Preferences.h"
#include "io/PathQt.h"
#include "io/SystemPaths.h"
#include "ui/UpdateVersion.h"
#include "upd/InstallUpdate.h"
#include "upd/Unzip.h"
#include "upd/UpdateController.h"

#include <filesystem>

namespace tb::ui
{
namespace
{

bool shouldEnableUpdating()
{
#if defined(_WIN32)
  return io::SystemPaths::appFile().filename() == "TrenchBroom.exe";
#elif defined(__APPLE__)
  return io::SystemPaths::appFile().filename() == "TrenchBroom";
#else
  return io::SystemPaths::appFile().filename() == "trenchbroom";
#endif
}

auto getScriptPath()
{
#if defined(_WIN32)
  return io::SystemPaths::findResourceFile(
    std::filesystem::path{"update/install_update.bat"});
#else
  return io::SystemPaths::findResourceFile(
    std::filesystem::path{"update/install_update.sh"});
#endif
}

[[maybe_unused]] std::optional<std::filesystem::path> getAppImagePath()
{
  auto process = QProcess{};
  process.start("readlink", QStringList{"-f", "/proc/self/exe"});
  process.waitForFinished();
  if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
  {
    return std::nullopt;
  }

  const auto output = process.readAllStandardOutput();
  return std::filesystem::path{QString(output).toStdString()};
}

std::optional<std::filesystem::path> getAppFolderPath()
{
#if defined(_WIN32)
  return io::SystemPaths::appDirectory();
#elif defined(__APPLE__)
  return io::SystemPaths::appDirectory().parent_path().parent_path();
#else
  return getAppImagePath();
#endif
}

auto getRelativeAppPath()
{
#if defined(_WIN32)
  return std::filesystem::path{"trenchbroom.exe"};
#elif defined(__APPLE__)
  return std::filesystem::path{"Contents/MacOS/TrenchBroom"};
#else
  return std::filesystem::path{"."};
#endif
}

auto getWorkDirPath()
{
  return io::SystemPaths::tempDirectory() / "TrenchBroom-update";
}

auto getLogFilePath()
{
  return io::SystemPaths::userDataDirectory() / "TrenchBroom-update.log";
}

auto checkForUpdates(upd::UpdateController& updateController)
{
  updateController.checkForUpdates<UpdateVersion>(
    TemporalVersion{2024, 2},
    pref(Preferences::IncludePreReleaseUpdates),
    parseUpdateVersion,
    describeUpdateVersion,
    chooseAsset);
}

auto prepareUpdate(
  const QString& downloadedUpdatePath, const upd::UpdateConfig& updateConfig)
{
#if defined(_WIN32)
  return upd::unzip(
           downloadedUpdatePath,
           updateConfig.workDirPath + "/TrenchBroom",
           updateConfig.logFilePath)
           ? std::optional{updateConfig.workDirPath + "/TrenchBroom"}
           : std::nullopt;
#elif defined(__APPLE__)
  return upd::unzip(
           downloadedUpdatePath, updateConfig.workDirPath, updateConfig.logFilePath)
           ? std::optional{updateConfig.workDirPath + "/TrenchBroom.app"}
           : std::nullopt;
#elif defined(__linux__)
  return upd::unzip(
           downloadedUpdatePath, updateConfig.workDirPath, updateConfig.logFilePath)
           ? std::optional{updateConfig.workDirPath + "/TrenchBroom.AppImage"}
           : std::nullopt;
#endif
}

auto installUpdate(
  const QString& preparedUpdatePath,
  const upd::UpdateConfig& updateConfig,
  const bool restartApp)
{
  return upd::installUpdate(
    updateConfig.updateScriptPath,
    updateConfig.appFolderPath,
    preparedUpdatePath,
    updateConfig.relativeAppPath,
    updateConfig.workDirPath,
    updateConfig.logFilePath,
    restartApp);
}

} // namespace

std::optional<upd::UpdateConfig> makeUpdateConfig()
{
  if (!shouldEnableUpdating())
  {
    return std::nullopt;
  }

  const auto scriptPath = getScriptPath();
  const auto appFolderPath = getAppFolderPath();
  const auto relativeAppPath = getRelativeAppPath();
  const auto workDirPath = getWorkDirPath();
  const auto logFilePath = getLogFilePath();

  if (!appFolderPath)
  {
    return std::nullopt;
  }

  return upd::UpdateConfig{
    std::move(checkForUpdates),
    std::move(prepareUpdate),
    std::move(installUpdate),

    "TrenchBroom",
    "TrenchBroom",
    io::pathAsQPath(scriptPath),
    io::pathAsQPath(*appFolderPath),
    io::pathAsQPath(relativeAppPath),
    io::pathAsQPath(workDirPath),
    io::pathAsQPath(logFilePath),
  };
}

} // namespace tb::ui
