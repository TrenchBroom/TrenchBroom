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

#include "update/InstallUpdate.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include "update/Logging.h"

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

void configureProcess(
  QProcess& process,
  const QString& scriptPath,
  const QString& targetPath,
  const QString& sourcePath,
  const QString& relativeAppPath,
  const QString& logFilePath,
  const bool requiresAdminPrivileges,
  const bool restartApp)
{
  const auto pid = QCoreApplication::applicationPid();
  const auto appToLaunchPath = getAppToLaunchPath(targetPath, relativeAppPath);
  const auto scriptFolderDir = QFileInfo{scriptPath}.absoluteDir();

  auto arguments = QStringList{};
  arguments << QString::number(pid) << targetPath << sourcePath;
  if (restartApp)
  {
    arguments << appToLaunchPath;
  }

  if (requiresAdminPrivileges)
  {
    logToFile(
      logFilePath,
      QString{"Target path requires administrator privileges: %1"}.arg(targetPath));

    // we need to run cmd.exe from powershell with the RunAs verb to ask for admin rights
    process.setProgram("powershell");

    // pass /c and the script path to cmd.exe in order to run the installer script
    arguments.insert(0, "/c");
    arguments.insert(1, scriptPath);

    // surround each argument with quotation marks
    for (auto& argument : arguments)
    {
      argument = QString{R"("%1")"}.arg(argument);
    }

    const auto command =
      QString{
        R"(Start-Process -FilePath "cmd.exe" -ArgumentList '%1' -WindowStyle Hidden -Verb RunAs)"}
        .arg(arguments.join(" "));
    process.setArguments(QStringList{"-Command", command});
  }
  else
  {
    process.setProgram(scriptPath);
    process.setArguments(arguments);
  }

  process.setWorkingDirectory(scriptFolderDir.absolutePath());
  process.setStandardOutputFile(logFilePath, QIODevice::Append);
  process.setStandardErrorFile(logFilePath, QIODevice::Append);
}

} // namespace

bool installUpdate(
  const QString& updateScriptPath,
  const QString& targetPath,
  const QString& sourcePath,
  const QString& relativeAppPath,
  const QString& workDirPath,
  const QString& logFilePath,
  const bool requiresAdminPrivileges,
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
    auto process = QProcess{};
    configureProcess(
      process,
      *scriptPath,
      targetPath,
      sourcePath,
      relativeAppPath,
      logFilePath,
      requiresAdminPrivileges,
      restartApp);

    logToFile(
      logFilePath,
      QString{R"(Starting process:
  program: %1
  arguments: %2
  working directory: %3)"}
        .arg(process.program())
        .arg(process.arguments().join(" "))
        .arg(process.workingDirectory()));

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
