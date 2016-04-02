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

#ifndef TrenchBroom_FileSystemHierarchy
#define TrenchBroom_FileSystemHierarchy

#include "SharedPointer.h"
#include "StringUtils.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class FileSystemHierarchy : public FileSystem {
        private:
            typedef std::vector<FileSystem*> FileSystemList;
            FileSystemList m_fileSystems;
        public:
            FileSystemHierarchy();
            virtual ~FileSystemHierarchy();
            
            void addFileSystem(FileSystem* fileSystem);
            void clear();
        private:
            bool doDirectoryExists(const Path& path) const;
            bool doFileExists(const Path& path) const;
            
            Path::List doGetDirectoryContents(const Path& path) const;
            const MappedFile::Ptr doOpenFile(const Path& path) const;
        private:
            FileSystemHierarchy(const FileSystemHierarchy& other);
            FileSystemHierarchy& operator=(FileSystemHierarchy other);
        };
    }
}

#endif /* defined(TrenchBroom_FileSystemHierarchy) */
