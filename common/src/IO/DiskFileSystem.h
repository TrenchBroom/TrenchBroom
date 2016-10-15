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

#ifndef TrenchBroom_DiskFileSystem
#define TrenchBroom_DiskFileSystem

#include "IO/MappedFile.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        class DiskFileSystem : public virtual FileSystem {
        protected:
            Path m_root;
        public:
            DiskFileSystem(const Path& root, bool ensureExists = true);
        private:
            Path doMakeAbsolute(const Path& relPath) const;
            bool doDirectoryExists(const Path& path) const;
            bool doFileExists(const Path& path) const;
            
            Path::List doGetDirectoryContents(const Path& path) const;
            const MappedFile::Ptr doOpenFile(const Path& path) const;
        };
        
#ifdef _MSC_VER
// MSVC complains about the fact that this class inherits some (pure virtual) method declarations several times from different base classes, even though there is only one definition.
#pragma warning(push)
#pragma warning(disable : 4250)
#endif
        class WritableDiskFileSystem : public DiskFileSystem, public WritableFileSystem {
        public:
            WritableDiskFileSystem(const Path& root, bool create);
        private:
            void doCreateFile(const Path& path, const String& contents);
            void doCreateDirectory(const Path& path);
            void doDeleteFile(const Path& path);
            void doCopyFile(const Path& sourcePath, const Path& destPath, bool overwrite);
            void doMoveFile(const Path& sourcePath, const Path& destPath, bool overwrite);
        };
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    }
}

#endif /* defined(TrenchBroom_DiskFileSystem) */
