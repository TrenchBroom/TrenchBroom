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

#include "update/Logging.h"

namespace upd
{

namespace
{

constexpr int UNZIP_TIMEOUT_MS = 5 * 60 * 1000; // 5 minutes
constexpr int PROCESS_START_TIMEOUT_MS = 5000;  // 5 seconds

void logProcessStart(
  const std::optional<QString>& logFilePath,
  const QString& program,
  const QStringList& arguments)
{
  logToFile(
    logFilePath, QString{"Extraction command: %1 %2"}.arg(program, arguments.join(" ")));
  logToFile(
    logFilePath,
    QString{"Starting extraction (timeout: %1 seconds)..."}.arg(UNZIP_TIMEOUT_MS / 1000));
}

void logProcessResult(
  const std::optional<QString>& logFilePath,
  int exitCode,
  const QString& standardOutput,
  const QString& errorOutput)
{
  if (exitCode == 0)
  {
    logToFile(logFilePath, QString{"Extraction completed successfully"});
  }
  else
  {
    logToFile(
      logFilePath,
      QString{"Extraction failed: command exited with code %1"}.arg(exitCode));
  }

  if (!standardOutput.isEmpty())
  {
    logToFile(logFilePath, QString{"Standard output: %1"}.arg(standardOutput));
  }

  if (!errorOutput.isEmpty())
  {
    logToFile(logFilePath, QString{"Error output: %1"}.arg(errorOutput));
  }
}

bool unzipWithCommand(
  const QString& zipPath,
  const QString& destFolderPath,
  const QString& program,
  const QStringList& arguments,
  const std::optional<QString>& logFilePath)
{
  if (!QFileInfo{zipPath}.exists())
  {
    logToFile(
      logFilePath, QString{"Failed to unzip: archive file not found: %1"}.arg(zipPath));
    return false;
  }

  if (!QFileInfo{destFolderPath}.exists() && !QDir{destFolderPath}.mkpath("."))
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip: could not create destination directory: %1"}.arg(
        destFolderPath));
    return false;
  }

  if (!QFileInfo{destFolderPath}.isDir())
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip: destination is not a directory: %1"}.arg(destFolderPath));
    return false;
  }

  logProcessStart(logFilePath, program, arguments);

  auto process = QProcess{};
  process.setProgram(program);
  process.setArguments(arguments);

  process.start();
  if (!process.waitForStarted(PROCESS_START_TIMEOUT_MS))
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip: could not start extraction command (%1)"}.arg(
        process.errorString()));
    return false;
  }

  if (!process.waitForFinished(UNZIP_TIMEOUT_MS))
  {
    process.kill();
    process.waitForFinished(3000); // Give it 3 seconds to terminate
    logToFile(
      logFilePath,
      QString{"Failed to unzip: extraction command timed out after %1 seconds"}.arg(
        UNZIP_TIMEOUT_MS / 1000));
    return false;
  }

  const auto exitCode = process.exitCode();
  const auto errorOutput = QString::fromUtf8(process.readAllStandardError());
  const auto standardOutput = QString::fromUtf8(process.readAllStandardOutput());

  if (process.exitStatus() != QProcess::NormalExit)
  {
    logProcessResult(logFilePath, exitCode, standardOutput, errorOutput);
    logToFile(
      logFilePath,
      QString{"Failed to unzip: extraction command terminated abnormally (%1)"}.arg(
        process.errorString()));
    return false;
  }

  logProcessResult(logFilePath, exitCode, standardOutput, errorOutput);

  if (exitCode != 0)
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip: extraction command exited with code %1"}.arg(exitCode));
    return false;
  }

  return true;
}

} // namespace

bool unzip(
  const QString& zipPath,
  const QString& destFolderPath,
  const std::optional<QString>& logFilePath)
{
  const auto absoluteZipPath = QFileInfo{zipPath}.absoluteFilePath();
  const auto absoluteDestPath = QDir{destFolderPath}.absolutePath();

  logToFile(
    logFilePath,
    QString{"Unzipping %1 to %2"}.arg(absoluteZipPath).arg(absoluteDestPath));

#if defined(Q_OS_MACOS)
  return unzipWithCommand(
    absoluteZipPath,
    absoluteDestPath,
    "/usr/bin/ditto",
    QStringList{"-xk", absoluteZipPath, absoluteDestPath},
    logFilePath);
#elif defined(Q_OS_LINUX)
  return unzipWithCommand(
    absoluteZipPath,
    absoluteDestPath,
    "/usr/bin/unzip",
    QStringList{"-q", absoluteZipPath, "-d", absoluteDestPath},
    logFilePath);
#elif defined(Q_OS_WIN)
  return unzipWithCommand(
    absoluteZipPath,
    absoluteDestPath,
    "tar.exe",
    QStringList{"-xf", absoluteZipPath, "-C", absoluteDestPath},
    logFilePath);
#else
  logToFile(logFilePath, QString{"Failed to unzip: unsupported platform"});
  return false;
#endif
}

} // namespace upd
