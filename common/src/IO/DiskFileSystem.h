/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_DiskFileSystem
#define TrenchBroom_DiskFileSystem

#include "IO/MappedFile.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        namespace Disk {
            bool isCaseSensitive();
            
            Path fixPath(const Path& path);
            
            bool directoryExists(const Path& path);
            bool fileExists(const Path& path);
            
            Path::List getDirectoryContents(const Path& path);
            MappedFile::Ptr openFile(const Path& path);
            Path getCurrentWorkingDir();
            
            IO::Path resolvePath(const Path::List& searchPaths, const Path& path);
        }
        
        class DiskFileSystem : public FileSystem {
        protected:
            Path m_root;
        public:
            DiskFileSystem(const Path& root, const bool ensureExists = true);
            
            const Path& getPath() const;
            const Path makeAbsolute(const Path& relPath) const;
        protected:
            Path fixPath(const Path& path) const;
            Path fixCase(const Path& path) const;
        private:
            bool doDirectoryExists(const Path& path) const;
            bool doFileExists(const Path& path) const;
            
            Path::List doGetDirectoryContents(const Path& path) const;
            const MappedFile::Ptr doOpenFile(const Path& path) const;
        };
        
        class WritableDiskFileSystem : public DiskFileSystem, public WritableFileSystem {
        public:
            WritableDiskFileSystem(const Path& root, const bool create);
        private:
            void doCreateDirectory(const Path& path);
            void doDeleteFile(const Path& path);
            void doMoveFile(const Path& sourcePath, const Path& destPath, const bool overwrite);
        };
    }
}

#endif /* defined(TrenchBroom_DiskFileSystem) */
