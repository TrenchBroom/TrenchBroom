/*
 Copyright (C) 2010 Kristian Duske

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

#include "SystemPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>

#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "io/PathQt.h"

#include <algorithm>
#include <vector>

namespace tb::io::SystemPaths
{
namespace
{

std::filesystem::path appImageDirectory()
{
  return appDirectory() / ".." / "share" / "TrenchBroom";
}

} // namespace

std::filesystem::path appFile()
{
  return io::pathFromQString(QCoreApplication::applicationFilePath());
}

std::filesystem::path appDirectory()
{
  return io::pathFromQString(QCoreApplication::applicationDirPath());
}

std::filesystem::path userDataDirectory()
{
  if (isPortable())
  {
    return appDirectory() / "config";
  }
#if defined __linux__ || defined __FreeBSD__
  // Compatibility with wxWidgets
  return io::pathFromQString(QDir::homePath()) / ".TrenchBroom";
#else
  return io::pathFromQString(
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#endif
}

std::filesystem::path userGamesDirectory()
{
  return userDataDirectory() / "games";
}

std::filesystem::path tempDirectory()
{
  return io::pathFromQString(
    QStandardPaths::writableLocation(QStandardPaths::TempLocation));
}

std::filesystem::path logFilePath()
{
  return userDataDirectory() / "TrenchBroom.log";
}

std::filesystem::path findResourceFile(const std::filesystem::path& file)
{
  // Special case for running debug builds on Linux, we want to search
  // next to the executable for resources
  const auto relativeToExecutable = appDirectory() / file;
  if (fs::Disk::pathInfo(relativeToExecutable) == fs::PathInfo::File)
  {
    return relativeToExecutable;
  }

  // Compatibility with wxWidgets
  const auto inUserDataDir = userDataDirectory() / file;
  if (fs::Disk::pathInfo(inUserDataDir) == fs::PathInfo::File)
  {
    return inUserDataDir;
  }

  // Compatibility with AppImage runtime
  const auto inAppImageDir = appImageDirectory() / file;
  if (fs::Disk::pathInfo(inAppImageDir) == fs::PathInfo::File)
  {
    return inAppImageDir;
  }

  return io::pathFromQString(QStandardPaths::locate(
    QStandardPaths::AppDataLocation,
    io::pathAsQPath(file),
    QStandardPaths::LocateOption::LocateFile));
}

std::vector<std::filesystem::path> findResourceDirectories(
  const std::filesystem::path& directory)
{
  auto result = std::vector<std::filesystem::path>{
    // Special case for running debug builds on Linux
    appDirectory() / directory,
    // Compatibility with wxWidgets
    userDataDirectory() / directory,
    // Compatibility with AppImage
    appImageDirectory() / directory,
  };

  const auto dirs = QStandardPaths::locateAll(
    QStandardPaths::AppDataLocation,
    io::pathAsQPath(directory),
    QStandardPaths::LocateOption::LocateDirectory);

  for (const auto& dir : dirs)
  {
    const auto path = io::pathFromQString(dir);
    if (std::ranges::find(result, path) == result.end())
    {
      result.push_back(path);
    }
  }
  return result;
}

bool portableState = false;

bool isPortable()
{
  return portableState;
}

void setPortable()
{
  setPortable(true);
}

void setPortable(bool newState)
{
  portableState = newState;
}

} // namespace tb::io::SystemPaths
