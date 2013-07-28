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

#ifndef __TrenchBroom__FileSystem__
#define __TrenchBroom__FileSystem__

#include "StringUtils.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        class FileSystem {
        public:
            typedef enum {
                FSDirectories,
                FSFiles,
                FSBoth
            } FileSystemFilter;
        public:
            Path findRootPath(const Path::List& rootPaths, const Path& relativePath);
            Path resolvePath(const Path::List& rootPaths, const Path& path);
            Path::List resolvePaths(const Path::List& rootPaths, const Path::List& paths);
            Path findCaseSensitivePath(const Path& path);
            
            bool isDirectory(const Path& path) const;
            bool exists(const Path& path) const;
            Path::List directoryContents(const Path& path, const FileSystemFilter contentFilter = FSBoth, const String& extension = "") const;
            
            void createDirectory(const Path& path) const;
            void deleteFile(const Path& path) const;
            void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite) const;
            MappedFile::Ptr mapFile(const Path& path, const std::ios_base::openmode mode) const;
            
            Path appDirectory() const;
            Path logDirectory() const;
            Path resourceDirectory() const;
            Path findFontFile(const String& fontName) const;
        private:
            Path findCaseSensitivePath(const Path::List& list, const Path& path) const;
        };
    }
}

#endif /* defined(__TrenchBroom__FileSystem__) */
