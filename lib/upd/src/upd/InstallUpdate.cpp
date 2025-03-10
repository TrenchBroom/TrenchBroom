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

#include "InstallUpdate.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include "upd/Logging.h"

namespace upd
{
namespace
{

std::optional<QString> prepareUpdateScript(
  const QString& updateScriptPath, const QString& workDirPath, const QString& logFilePath)
{
  auto updateScript = QFileInfo{updateScriptPath};
  if (!updateScript.exists(updateScriptPath))
  {
    logToFile(logFilePath, QString{"Update script not found: %1"}.arg(updateScriptPath));
    return std::nullopt;
  }

  const auto workDir = QDir{workDirPath};
  const auto scriptTargetPath = workDir.filePath(updateScript.fileName());
  const auto scriptTargetInfo = QFileInfo{scriptTargetPath};

  if (!QFile::copy(updateScriptPath, scriptTargetPath))
  {
    logToFile(
      logFilePath, QString{"Failed to copy update script to: %1"}.arg(scriptTargetPath));
    return std::nullopt;
  }

  return scriptTargetPath;
}

auto getAppToLaunchPath(QString targetPath, const QString& relativeAppPath)
{
  if (!relativeAppPath.isEmpty())
  {
    if (!targetPath.endsWith(QDir::separator()))
    {
      targetPath.append(QDir::separator());
    }
    return targetPath + relativeAppPath;
  }

  if (targetPath.endsWith(QDir::separator()))
  {
    targetPath.chop(1);
  }

  return targetPath;
}

} // namespace

bool installUpdate(
  const QString& updateScriptPath,
  const QString& targetPath,
  const QString& sourcePath,
  const QString& relativeAppPath,
  const QString& workDirPath,
  const QString& logFilePath,
  const bool restartApp)
{
  if (!QFileInfo{workDirPath}.exists())
  {
    logToFile(logFilePath, QString{"Work dir not found: %1"}.arg(workDirPath));
    return false;
  }

  if (
    const auto scriptPath =
      prepareUpdateScript(updateScriptPath, workDirPath, logFilePath))
  {
    const auto pid = QCoreApplication::applicationPid();
    const auto appToLaunchPath = getAppToLaunchPath(targetPath, relativeAppPath);
    const auto scriptFolderDir = QFileInfo{*scriptPath}.absoluteDir();

    auto process = QProcess{};
    process.setWorkingDirectory(scriptFolderDir.absolutePath());
    process.setProgram(*scriptPath);

    auto arguments = QStringList{};
    arguments << QString::number(pid) << targetPath << sourcePath;
    if (restartApp)
    {
      arguments << appToLaunchPath;
    }
    process.setArguments(arguments);

    process.setStandardOutputFile(logFilePath, QIODevice::Append);
    process.setStandardErrorFile(logFilePath, QIODevice::Append);

    if (!process.startDetached())
    {
      logToFile(
        logFilePath,
        QString{"Failed to start update script %1: %2"}
          .arg(*scriptPath)
          .arg(process.errorString()));
      return false;
    }

    return true;
  }

  return false;
}

} // namespace upd
