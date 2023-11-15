/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include <QStandardPaths>
#include <QString>

#include "IO/DiskIO.h"
#include "IO/PathInfo.h"
#include "IO/PathQt.h"

#include <string>
#include <vector>

namespace TrenchBroom::IO::SystemPaths
{

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

std::filesystem::path appDirectory()
{
  return IO::pathFromQString(QCoreApplication::applicationDirPath());
}

std::filesystem::path userDataDirectory()
{
  if (isPortable())
  {
    return appDirectory() / "config";
  }
#if defined __linux__ || defined __FreeBSD__
  // Compatibility with wxWidgets
  return IO::pathFromQString(QDir::homePath()) / ".TrenchBroom";
#else
  return IO::pathFromQString(
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#endif
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
  if (Disk::pathInfo(relativeToExecutable) == PathInfo::File)
  {
    return relativeToExecutable;
  }

  // Compatibility with wxWidgets
  const auto inUserDataDir = userDataDirectory() / file;
  if (Disk::pathInfo(inUserDataDir) == PathInfo::File)
  {
    return inUserDataDir;
  }

  return IO::pathFromQString(QStandardPaths::locate(
    QStandardPaths::AppDataLocation,
    IO::pathAsQString(file),
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
  };

  const auto dirs = QStandardPaths::locateAll(
    QStandardPaths::AppDataLocation,
    IO::pathAsQString(directory),
    QStandardPaths::LocateOption::LocateDirectory);

  for (const auto& dir : dirs)
  {
    const auto path = IO::pathFromQString(dir);
    if (std::find(result.begin(), result.end(), path) == result.end())
    {
      result.push_back(path);
    }
  }
  return result;
}
} // namespace TrenchBroom::IO::SystemPaths
