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

#include "update/Unzip.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include "update/FileUtils.h"
#include "update/Logging.h"

namespace upd
{

bool unzip(
  const QString& zipPath,
  const QString& destFolderPath,
  const std::optional<QString>& logFilePath)
{
  if (!QFileInfo{destFolderPath}.exists() && !QDir{destFolderPath}.mkpath("."))
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip the archive: %1 could not be created"}.arg(
        destFolderPath));
    return false;
  }

  if (!QFileInfo{destFolderPath}.isDir())
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip the archive: %1 is not a folder"}.arg(destFolderPath));
    return false;
  }

  auto process = QProcess{};
#if defined(_WIN32)
  auto arguments = QStringList{};
  arguments
    << "-Command"
    << QString{"Expand-Archive -Path '%1' -DestinationPath '%2'"}.arg(zipPath).arg(
         destFolderPath);

  process.setProgram("powershell");
  process.setArguments(arguments);
#elif defined(__APPLE__) || defined(__linux__)
  auto arguments = QStringList{};
  arguments << zipPath << "-d" << destFolderPath;

  process.setProgram("unzip");
  process.setArguments(arguments);
#else
  logToFile(logFilePath, "Failed to unzip the archive: Unsupported platform");
  return false;
#endif

  if (logFilePath)
  {
    process.setStandardOutputFile(*logFilePath, QIODevice::Append);
    process.setStandardErrorFile(*logFilePath, QIODevice::Append);
  }

  process.start();
  process.waitForFinished(60000);

  if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip the archive: Exit code %1"}.arg(process.exitCode()));
    return false;
  }

  return true;
}

} // namespace upd
