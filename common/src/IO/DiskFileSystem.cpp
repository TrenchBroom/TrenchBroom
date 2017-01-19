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

#include "DiskFileSystem.h"

#include "Exceptions.h"
#include "StringUtils.h"

#include "IO/DiskIO.h"

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace IO {
        DiskFileSystem::DiskFileSystem(const Path& root, const bool ensureExists) :
        m_root(Disk::fixPath(root)) {
            if (ensureExists && !Disk::directoryExists(m_root))
                throw FileSystemException("Directory not found: '" + m_root.asString() + "'");
        }
        
        Path DiskFileSystem::doMakeAbsolute(const Path& relPath) const {
            return m_root + relPath.makeCanonical();
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
        DiskFileSystem(root, !create) {
            if (create && !Disk::directoryExists(m_root))
                Disk::createDirectory(m_root);
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
