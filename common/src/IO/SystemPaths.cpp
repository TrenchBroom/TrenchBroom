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

#include "IO/Path.h"
#include "IO/DiskIO.h"

#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QString>
#include <QStandardPaths>

namespace TrenchBroom {
    namespace IO {
        namespace SystemPaths {
            Path appDirectory() {
                return IO::Path(QCoreApplication::applicationDirPath().toStdString());
            }

            Path userDataDirectory() {
                return IO::Path(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString());
            }

            Path logFilePath() {
                return userDataDirectory() + IO::Path("TrenchBroom.log");
            }

            Path findResourceFile(const Path &file) {
                const auto relativeToExecutable = appDirectory() + file;
                if (Disk::fileExists(relativeToExecutable)) {
                    // This is for running debug builds on Linux
                    return relativeToExecutable;
                }

                return IO::Path(QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                       file.asQString(),
                                                       QStandardPaths::LocateOption::LocateFile).toStdString());
            }

            Path findResourceDirectory(const Path &directory) {
                const auto relativeToExecutable = appDirectory() + directory;
                if (Disk::directoryExists(relativeToExecutable)) {
                    // This is for running debug builds on Linux
                    return relativeToExecutable;
                }

                // FIXME: confirm against wx
                return IO::Path(QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                       directory.asQString(),
                                                       QStandardPaths::LocateOption::LocateDirectory).toStdString());
            }
        }
    }
}
