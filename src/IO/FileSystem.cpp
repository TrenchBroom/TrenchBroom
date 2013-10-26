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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FileSystem.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        FileSystem::ExtensionMatcher::ExtensionMatcher(const String& extension) :
        m_extension(extension) {}
        
        bool FileSystem::ExtensionMatcher::operator()(const Path& path, const bool directory) const {
            if (directory)
                return false;
            return StringUtils::caseInsensitiveEqual(path.extension(), m_extension);
        }
        
        FileSystem::~FileSystem() {}
        
        bool FileSystem::directoryExists(const Path& path) const {
            if (path.isAbsolute())
                throw FileSystemException("Path is absolute: '" + path.asString() + "'");
            return doDirectoryExists(path.makeCanonical());
        }
        
        bool FileSystem::fileExists(const Path& path) const {
            if (path.isAbsolute())
                throw FileSystemException("Path is absolute: '" + path.asString() + "'");
            return doFileExists(path.makeCanonical());
        }

        Path::List FileSystem::getDirectoryContents(const Path& path) const {
            if (path.isAbsolute())
                throw FileSystemException("Path is absolute: '" + path.asString() + "'");
            const Path canonicalPath = path.makeCanonical();
            if (!directoryExists(canonicalPath))
                throw FileSystemException("FileSystem does not exist: '" + path.asString() + "'");
            return doGetDirectoryContents(canonicalPath);
        }

        const MappedFile::Ptr FileSystem::openFile(const Path& path) const {
            if (path.isAbsolute())
                throw FileSystemException("Path is absolute: '" + path.asString() + "'");
            const Path canonicalPath = path.makeCanonical();
            if (!fileExists(canonicalPath))
                throw FileSystemException("File does not exist: '" + path.asString() + "'");
            return doOpenFile(canonicalPath);
        }
        
        WritableFileSystem::~WritableFileSystem() {}
        
        void WritableFileSystem::createDirectory(const Path& path) {
            if (path.isAbsolute())
                throw FileSystemException("Path is absolute: '" + path.asString() + "'");
            doCreateDirectory(path.makeCanonical());
        }
        
        void WritableFileSystem::deleteFile(const Path& path) {
            if (path.isAbsolute())
                throw FileSystemException("Path is absolute: '" + path.asString() + "'");
            doDeleteFile(path.makeCanonical());
        }
        
        void WritableFileSystem::moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            if (sourcePath.isAbsolute())
                throw FileSystemException("Source path is absolute: '" + sourcePath.asString() + "'");
            if (destPath.isAbsolute())
                throw FileSystemException("Destination path is absolute: '" + destPath.asString() + "'");
            doMoveFile(sourcePath.makeCanonical(), destPath.makeCanonical(), overwrite);
        }
    }
}
