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

#include "FileSystemHierarchy.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/IdPakFileSystem.h"

namespace TrenchBroom {
    namespace IO {
        FileSystemHierarchy::FileSystemHierarchy() {}

        FileSystemHierarchy::~FileSystemHierarchy() {
            clear();
        }
        
        void FileSystemHierarchy::addFileSystem(FileSystem* fileSystem) {
            ensure(fileSystem != nullptr, "fileSystem is null");
            m_fileSystems.push_back(fileSystem);
        }

        void FileSystemHierarchy::clear() {
            VectorUtils::clearAndDelete(m_fileSystems);
        }

        Path FileSystemHierarchy::doMakeAbsolute(const Path& relPath) const {
            const FileSystem* fileSystem = findFileSystemContaining(relPath);
            if (fileSystem != nullptr)
                return fileSystem->makeAbsolute(relPath);
            return Path("");
        }

        bool FileSystemHierarchy::doDirectoryExists(const Path& path) const {
            for (auto it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FileSystem* fileSystem = *it;
                if (fileSystem->directoryExists(path))
                    return true;
            }
            return false;
        }
        
        bool FileSystemHierarchy::doFileExists(const Path& path) const {
            return (findFileSystemContaining(path)) != nullptr;
        }
        
        FileSystem* FileSystemHierarchy::findFileSystemContaining(const Path& path) const {
            for (auto it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                FileSystem* fileSystem = *it;
                if (fileSystem->fileExists(path))
                    return fileSystem;
            }
            return nullptr;
        }

        Path::List FileSystemHierarchy::doGetDirectoryContents(const Path& path) const {
            Path::List result;
            for (auto it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
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
            for (auto it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                const FileSystem* fileSystem = *it;
                if (fileSystem->fileExists(path)) {
                    const MappedFile::Ptr file = fileSystem->openFile(path);
                    if (file.get() != nullptr)
                        return file;
                }
            }
            return MappedFile::Ptr();
        }

        WritableFileSystemHierarchy::WritableFileSystemHierarchy() :
        m_writableFileSystem(nullptr) {}
        
        void WritableFileSystemHierarchy::addReadableFileSystem(FileSystem* fileSystem) {
            addFileSystem(fileSystem);
        }
        
        void WritableFileSystemHierarchy::addWritableFileSystem(WritableFileSystem* fileSystem) {
            assert(m_writableFileSystem == nullptr);
            addFileSystem(fileSystem);
            m_writableFileSystem = fileSystem;
        }
        
        void WritableFileSystemHierarchy::clear() {
            FileSystemHierarchy::clear();
            m_writableFileSystem = nullptr;
        }

        void WritableFileSystemHierarchy::doCreateFile(const Path& path, const String& contents) {
            ensure(m_writableFileSystem != nullptr, "writableFileSystem is null");
            m_writableFileSystem->createFile(path, contents);
        }

        void WritableFileSystemHierarchy::doCreateDirectory(const Path& path) {
            ensure(m_writableFileSystem != nullptr, "writableFileSystem is null");
            m_writableFileSystem->createDirectory(path);
        }
        
        void WritableFileSystemHierarchy::doDeleteFile(const Path& path) {
            ensure(m_writableFileSystem != nullptr, "writableFileSystem is null");
            m_writableFileSystem->deleteFile(path);
        }
        
        void WritableFileSystemHierarchy::doCopyFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            ensure(m_writableFileSystem != nullptr, "writableFileSystem is null");
            m_writableFileSystem->copyFile(sourcePath, destPath, overwrite);
        }
        
        void WritableFileSystemHierarchy::doMoveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            ensure(m_writableFileSystem != nullptr, "writableFileSystem is null");
            m_writableFileSystem->moveFile(sourcePath, destPath, overwrite);
        }
    }
}
