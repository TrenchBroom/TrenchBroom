/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#if defined __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#elif defined _WIN32
#include <Windows.h>
#elif defined __linux__
#include <unistd.h>
#endif

namespace TrenchBroom {
    namespace IO {
        namespace SystemPaths {
            Path appDirectory() {
                const IO::Path executablePath(wxStandardPaths::Get().GetExecutablePath().ToStdString());
                return executablePath.deleteLastComponent();
            }
            
            Path resourceDirectory() {
#if defined __APPLE__
                return IO::Path(wxStandardPaths::Get().GetResourcesDir().ToStdString());
#else
                return appDirectory() + Path("Resources");
#endif
            }

            Path userDataDirectory() {
                return IO::Path(wxStandardPaths::Get().GetUserDataDir().ToStdString());
            }
        }
    }
}
