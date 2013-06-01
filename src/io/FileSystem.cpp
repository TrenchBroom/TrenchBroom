/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Exceptions.h"
#include "IO/FileSystem.h"

#include <wx/dir.h>

namespace TrenchBroom {
    namespace IO {
        bool FileSystem::isDirectory(const Path& path) const {
            return ::wxDirExists(path.asString());
        }

        bool FileSystem::exists(const Path& path) const {
            return ::wxFileExists(path.asString()) || ::wxDirExists(path.asString());
        }

        Path::List FileSystem::directoryContents(const Path& path, const FileSystemFilter contentFilter, const String& namePattern) const {
            if (!isDirectory(path.asString())) {
                FileSystemException e;
                e << path.asString() << " does not exist or is not a directory";
                throw e;
            }
            
            wxDir dir(path.asString());
            if (!dir.IsOpened()) {
                FileSystemException e;
                e << path.asString() << " could not be opened";
                throw e;
            }
            
            int flags = 0;
            switch (contentFilter) {
                case FSDirectories:
                    flags = wxDIR_DIRS;
                    break;
                case FSFiles:
                    flags = wxDIR_FILES;
                    break;
                case FSBoth:
                    flags = wxDIR_DIRS | wxDIR_FILES;
                    break;
            }
            
            Path::List result;
            
            wxString filename;
            if (dir.GetFirst(&filename, namePattern, flags)) {
                result.push_back(Path(filename.ToStdString()));
                
                while (dir.GetNext(&filename))
                    result.push_back(Path(filename.ToStdString()));
            }
            return result;
        }

        MappedFile::Ptr FileSystem::mapFile(const Path& path, const std::ios_base::openmode mode) const {
#ifdef _WIN32
            return MappedFile::Ptr(new WinMappedFile(path, mode));
#else
            return MappedFile::Ptr(new PosixMappedFile(path, mode));
#endif
        }
    }
}
