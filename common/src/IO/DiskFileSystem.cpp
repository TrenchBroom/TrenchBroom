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

#include "DiskFileSystem.h"

#include "Exceptions.h"
#include "StringUtils.h"

#include "IO/DiskIO.h"

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace IO {
        DiskFileSystem::DiskFileSystem(const Path& root, const bool ensureExists) :
        DiskFileSystem(nullptr, root, ensureExists) {}

        DiskFileSystem::DiskFileSystem(std::unique_ptr<FileSystem> next, const Path& root, const bool ensureExists) :
        FileSystem(std::move(next)),
        m_root(root) {
            if (ensureExists && !Disk::directoryExists(m_root)) {
                throw FileSystemException("Directory not found: '" + m_root.asString() + "'");
            }
        }

        Path DiskFileSystem::makeAbsolute(const Path& relPath) const {
            try {
                if (relPath.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + relPath.asString() + "'");
                }

                return m_root + relPath.makeCanonical();
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + relPath.asString() + "'", e);
            }
        }
        
        bool DiskFileSystem::doDirectoryExists(const Path& path) const {
            return Disk::directoryExists(makeAbsolute(path));
        }
        
        bool DiskFileSystem::doFileExists(const Path& path) const {
            return Disk::fileExists(makeAbsolute(path));
        }
        
        Path::List DiskFileSystem::doGetDirectoryContents(const Path& path) const {
            return Disk::getDirectoryContents(makeAbsolute(path));
        }
        
        const MappedFile::Ptr DiskFileSystem::doOpenFile(const Path& path) const {
            return Disk::openFile(makeAbsolute(path));
        }
        
        WritableDiskFileSystem::WritableDiskFileSystem(const Path& root, const bool create) :
        WritableDiskFileSystem(nullptr, root, create) {}

        WritableDiskFileSystem::WritableDiskFileSystem(std::unique_ptr<FileSystem> next, const Path& root, const bool create) :
        FileSystem(std::move(next)), // pass the next pointer to the single shared superclass instance
        DiskFileSystem(root, !create), // no need to pass the next pointer here
        WritableFileSystem() { // nor here, since neither of these constructors will call the FileSystem constructor
            if (create && !Disk::directoryExists(m_root)) {
                Disk::createDirectory(m_root);
            }
        }

        void WritableDiskFileSystem::doCreateFile(const Path& path, const String& contents) {
            Disk::createFile(makeAbsolute(path), contents);
        }

        void WritableDiskFileSystem::doCreateDirectory(const Path& path) {
            Disk::createDirectory(makeAbsolute(path));
        }
        
        void WritableDiskFileSystem::doDeleteFile(const Path& path) {
            Disk::deleteFile(makeAbsolute(path));
        }
        
        void WritableDiskFileSystem::doCopyFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            Disk::copyFile(makeAbsolute(sourcePath), makeAbsolute(destPath), overwrite);
        }

        void WritableDiskFileSystem::doMoveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            Disk::moveFile(makeAbsolute(sourcePath), makeAbsolute(destPath), overwrite);
        }
    }
}
