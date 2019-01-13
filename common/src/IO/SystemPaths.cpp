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

#if defined __linux__ || defined __FreeBSD__
            static bool getDevMode() {
                QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
                QString value = environment.value("TB_DEV_MODE");
                if (value.isEmpty()) {
                    return false;
                }
                return value != "0";
            }
#endif

#if 0
            Path resourceDirectory() {
#if defined __linux__ || defined __FreeBSD__
                static const bool DevMode = getDevMode();
                if (DevMode)
                    return appDirectory();
#endif
                // FIXME: implement. Will need to return a list of Paths
                assert(0);
                return IO::Path();
      //          return IO::Path(wxStandardPaths::Get().GetResourcesDir().ToStdString());
            }
#endif

            Path userDataDirectory() {
                // FIXME: confirm against wx
                return IO::Path(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString());
            }

            Path logFilePath() {
                return userDataDirectory() + IO::Path("TrenchBroom.log");
            }

            Path findResourceFile(const Path &file) {
                // FIXME: see if TB_DEV_MODE hack is needed
                return IO::Path(QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
                                                       file.asQString(),
                                                       QStandardPaths::LocateOption::LocateFile).toStdString());
            }

            Path findResourceDirectory(const Path &directory) {
                // FIXME: see if TB_DEV_MODE hack is needed
                return IO::Path(QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
                                                       directory.asQString(),
                                                       QStandardPaths::LocateOption::LocateDirectory).toStdString());
            }
        }
    }
}
