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

#include "FileSystemHierarchy.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/PakFileSystem.h"

namespace TrenchBroom {
    namespace IO {
        FileSystemHierarchy::FileSystemHierarchy() {}

        FileSystemHierarchy::~FileSystemHierarchy() {
            clear();
        }
        
        void FileSystemHierarchy::addFileSystem(FileSystem* fileSystem) {
            assert(fileSystem != NULL);
            m_fileSystems.push_back(fileSystem);
        }

        void FileSystemHierarchy::clear() {
            VectorUtils::clearAndDelete(m_fileSystems);
        }

        bool FileSystemHierarchy::doDirectoryExists(const Path& path) const {
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FileSystem* fileSystem = *it;
                if (fileSystem->directoryExists(path))
                    return true;
            }
            return false;
        }
        
        bool FileSystemHierarchy::doFileExists(const Path& path) const {
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FileSystem* fileSystem = *it;
                if (fileSystem->fileExists(path))
                    return true;
            }
            return false;
        }
        
        Path::List FileSystemHierarchy::doGetDirectoryContents(const Path& path) const {
            Path::List result;
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FileSystem* fileSystem = *it;
                if (fileSystem->directoryExists(path)) {
                    const Path::List contents = fileSystem->getDirectoryContents(path);
                    VectorUtils::append(result, contents);
                }
            }
            
            VectorUtils::sortAndRemoveDuplicates(result);
            return result;
        }
        
        const MappedFile::Ptr FileSystemHierarchy::doOpenFile(const Path& path) const {
            FileSystemList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FileSystem* fileSystem = *it;
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
