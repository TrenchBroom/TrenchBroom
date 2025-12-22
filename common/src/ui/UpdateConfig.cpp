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

#include <QDir>                      // IWYU pragma: keep
#include <QFileInfo>                 // IWYU pragma: keep
#include <QNtfsPermissionCheckGuard> // IWYU pragma: keep
#include <QProcess>

#include "Macros.h" // IWYU pragma: keep
#include "PreferenceManager.h"
#include "Preferences.h"
#include "ui/GetVersion.h"
#include "ui/QPathUtils.h"
#include "ui/SystemPaths.h"
#include "ui/UpdateVersion.h"
#include "update/InstallUpdate.h"
#include "update/Unzip.h"
#include "update/UpdateController.h" // IWYU pragma: keep

#include <cstdlib>
#include <filesystem>
#include <string_view> // IWYU pragma: keep

namespace tb::ui
{
namespace
{

bool shouldEnableUpdating()
{
#if defined(_WIN32)
  return SystemPaths::appFile().filename() == "TrenchBroom.exe";
#elif defined(__APPLE__)
  return SystemPaths::appFile().filename() == "TrenchBroom";
#else
  return std::getenv("APPIMAGE") != nullptr;
#endif
}

auto getScriptPath()
{
#if defined(_WIN32)
  return SystemPaths::findResourceFile(
    std::filesystem::path{"update/install_update.bat"});
#else
  return SystemPaths::findResourceFile(std::filesystem::path{"update/install_update.sh"});
#endif
}

std::optional<std::filesystem::path> getAppFolderPath()
{
#if defined(_WIN32)
  return SystemPaths::appDirectory();
#elif defined(__APPLE__)
  return SystemPaths::appDirectory().parent_path().parent_path();
#else
  auto appImage = std::string_view{std::getenv("APPIMAGE")};
  if (!appImage.empty() && appImage.back() == '/')
  {
    appImage.remove_suffix(1);
  }
  return !appImage.empty() ? std::optional{std::filesystem::path{appImage}}
                           : std::nullopt;
#endif
}

[[maybe_unused]] bool checkPathWritable(const std::filesystem::path& path)
{
  auto info = QFileInfo{path};
  if (info.exists() && !info.isWritable())
  {
    return false;
  }

  auto dir = info.dir();
  do
  {
    auto dirInfo = QFileInfo{};
    if (dirInfo.exists() && !dirInfo.isWritable())
    {
      return false;
    }
  } while (dir.cdUp());

  return true;
}

bool getRequiresAdminPrivileges([[maybe_unused]] const std::filesystem::path& targetPath)
{
#if defined _WIN32
  // enable NTFS permission checks:
  const auto permissionGuard = QNtfsPermissionCheckGuard{};
  Q_ASSERT(qAreNtfsPermissionChecksEnabled());

  return !checkPathWritable(targetPath);
#endif
  return false;
}

auto getRelativeAppPath()
{
#if defined(_WIN32)
  return std::filesystem::path{"trenchbroom.exe"};
#elif defined(__APPLE__)
  return std::filesystem::path{"Contents/MacOS/TrenchBroom"};
#else
  return std::filesystem::path{};
#endif
}

auto getWorkDirPath()
{
  return SystemPaths::tempDirectory() / "TrenchBroom-update";
}

auto getLogFilePath()
{
  return SystemPaths::userDataDirectory() / "TrenchBroom-update.log";
}

auto makeCheckForUpdates(const UpdateVersion& currentVersion)
{
  return [currentVersion](auto& updateController) {
    updateController.template checkForUpdates<UpdateVersion>(
      currentVersion,
      pref(Preferences::IncludePreReleaseUpdates),
      pref(Preferences::IncludeDraftReleaseUpdates),
      parseUpdateVersion,
      describeUpdateVersion,
      chooseAsset);
  };
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
    updateConfig.requiresAdminPrivileges,
    restartApp);
}

} // namespace

std::optional<upd::UpdateConfig> makeUpdateConfig()
{
  if (!shouldEnableUpdating())
  {
    return std::nullopt;
  }

  const auto currentVersion = parseUpdateVersion(QString{"v%1"}.arg(getBuildVersion()));
  if (!currentVersion)
  {
    return std::nullopt;
  }

  const auto appFolderPath = getAppFolderPath();
  if (!appFolderPath)
  {
    return std::nullopt;
  }

  auto checkForUpdates = makeCheckForUpdates(*currentVersion);

  const auto scriptPath = getScriptPath();
  const auto relativeAppPath = getRelativeAppPath();
  const auto workDirPath = getWorkDirPath();
  const auto logFilePath = getLogFilePath();

  const auto requiresAdminPrivileges = getRequiresAdminPrivileges(*appFolderPath);

  return upd::UpdateConfig{
    std::move(checkForUpdates),
    std::move(prepareUpdate),
    std::move(installUpdate),

    "TrenchBroom",
    "TrenchBroom",
    pathAsQPath(scriptPath),
    pathAsQPath(*appFolderPath),
    requiresAdminPrivileges,
    pathAsQPath(relativeAppPath),
    pathAsQPath(workDirPath),
    pathAsQPath(logFilePath),
  };
}

} // namespace tb::ui
