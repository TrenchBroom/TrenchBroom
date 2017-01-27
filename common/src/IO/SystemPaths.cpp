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

#include "IO/DiskFileSystem.h"
#include "IO/Path.h"

#include <wx/stdpaths.h>
#include <wx/utils.h>

namespace TrenchBroom {
    namespace IO {
        namespace SystemPaths {
            Path appDirectory() {
                return IO::Path(wxStandardPaths::Get().GetExecutablePath().ToStdString()).deleteLastComponent();
            }
            
#if defined __linux__ || defined __FreeBSD__
            static bool getDevMode() {
                wxString value;
                if (!wxGetEnv("TB_DEV_MODE", &value))
                    return false;
                return value != "0";
            }
#endif
            
            Path resourceDirectory() {
#if defined __linux__ || defined __FreeBSD__
                static const bool DevMode = getDevMode();
                if (DevMode)
                    return appDirectory();
#endif
                return IO::Path(wxStandardPaths::Get().GetResourcesDir().ToStdString());
            }

            Path userDataDirectory() {
                return IO::Path(wxStandardPaths::Get().GetUserDataDir().ToStdString());
            }
            
            Path logFilePath() {
                return userDataDirectory() + IO::Path("TrenchBroom.log");
            }
        }
    }
}

