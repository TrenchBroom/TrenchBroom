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

#ifndef TrenchBroom_GameFileSystem
#define TrenchBroom_GameFileSystem

#include "SharedPointer.h"
#include "StringUtils.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class GameFileSystem : public FileSystem {
        private:
            typedef std::tr1::shared_ptr<FileSystem> FSPtr;
            typedef std::vector<FSPtr> FileSystemList;
            FileSystemList m_fileSystems;
        public:
            GameFileSystem(const String& pakExtension, const Path& gamePath, const Path& searchPath, const Path::List& additionalSearchPaths = Path::List());
        private:
            void addFileSystem(const String& pakExtension, const Path& path);
            
            bool doDirectoryExists(const Path& path) const;
            bool doFileExists(const Path& path) const;
            
            Path::List doGetDirectoryContents(const Path& path) const;
            const MappedFile::Ptr doOpenFile(const Path& path) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameFileSystem) */
