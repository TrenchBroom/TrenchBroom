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

#include "GameFileSystem.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "DiskFileSystem.h"
#include "PakFileSystem.h"

namespace TrenchBroom {
    namespace IO {
        GameFileSystem::GameFileSystem(const String& pakExtension, const Path& gamePath, const Path& searchPath, const Path::List& additionalSearchPaths) {
            if (!gamePath.isEmpty()) {
                addFileSystem(pakExtension, gamePath + searchPath);
                
                Path::List::const_iterator it, end;
                for (it = additionalSearchPaths.begin(), end = additionalSearchPaths.end(); it != end; ++it)
                    addFileSystem(pakExtension, gamePath + *it);
            }
        }

        void GameFileSystem::addFileSystem(const String& pakExtension, const Path& path) {
            if (Disk::directoryExists(path)) {
                FSPtr diskFS(new DiskFileSystem(path));
                if (StringUtils::caseInsensitiveEqual(pakExtension, "pak")) {
                    const Path::List paks = diskFS->findItems(Path(""), FileSystem::ExtensionMatcher(pakExtension));
                    Path::List::const_iterator it, end;
                    for (it = paks.begin(), end = paks.end(); it != end; ++it) {
                        MappedFile::Ptr file = diskFS->openFile(*it);
                        assert(file.get() != NULL);
                        m_fileSystems.push_back(FSPtr(new PakFileSystem(path, file)));
                    }
                } else {
                    throw FileSystemException("Unknown file extension: '" + pakExtension + "'");
                }
                m_fileSystems.push_back(diskFS);
            }
        }

        bool GameFileSystem::doDirectoryExists(const Path& path) const {
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FSPtr fileSystem = *it;
                if (fileSystem->directoryExists(path))
                    return true;
            }
            return false;
        }
        
        bool GameFileSystem::doFileExists(const Path& path) const {
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FSPtr fileSystem = *it;
                if (fileSystem->fileExists(path))
                    return true;
            }
            return false;
        }
        
        Path::List GameFileSystem::doGetDirectoryContents(const Path& path) const {
            Path::List result;
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FSPtr fileSystem = *it;
                if (fileSystem->directoryExists(path)) {
                    const Path::List contents = fileSystem->getDirectoryContents(path);
                    VectorUtils::append(result, contents);
                }
            }
            
            VectorUtils::sortAndRemoveDuplicates(result);
            return result;
        }
        
        const MappedFile::Ptr GameFileSystem::doOpenFile(const Path& path) const {
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FSPtr fileSystem = *it;
                if (fileSystem->fileExists(path)) {
                    const MappedFile::Ptr file = fileSystem->openFile(path);
                    if (file.get() != NULL)
                        return file;
                }
            }
            return MappedFile::Ptr();
        }
    }
}
