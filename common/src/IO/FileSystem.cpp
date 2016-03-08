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

#include "FileSystem.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        FileSystem::TypeMatcher::TypeMatcher(const bool files, const bool directories) :
        m_files(files),
        m_directories(directories) {}
        
        bool FileSystem::TypeMatcher::operator()(const Path& path, const bool directory) const {
            if (m_files && !directory)
                return true;
            if (m_directories && directory)
                return true;
            return false;
        }

        FileSystem::ExtensionMatcher::ExtensionMatcher(const String& extension) :
        m_extension(extension) {}
        
        bool FileSystem::ExtensionMatcher::operator()(const Path& path, const bool directory) const {
            if (directory)
                return false;
            return StringUtils::caseInsensitiveEqual(path.extension(), m_extension);
        }
        
        FileSystem::~FileSystem() {}
        
        bool FileSystem::directoryExists(const Path& path) const {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                return doDirectoryExists(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }
        
        bool FileSystem::fileExists(const Path& path) const {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                return doFileExists(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        Path::List FileSystem::findItems(const Path& path) const {
            return findItems(path, TypeMatcher());
        }
        
        Path::List FileSystem::findItemsRecursively(const Path& path) const {
            return findItemsRecursively(path, TypeMatcher());
        }

        Path::List FileSystem::getDirectoryContents(const Path& path) const {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                if (!directoryExists(path))
                    throw FileSystemException("Directory not found: '" + path.asString() + "'");
                return doGetDirectoryContents(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        const MappedFile::Ptr FileSystem::openFile(const Path& path) const {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                if (!fileExists(path))
                    throw FileSystemException("File not found: '" + path.asString() + "'");
                return doOpenFile(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }
        
        WritableFileSystem::~WritableFileSystem() {}
        
        void WritableFileSystem::createDirectory(const Path& path) {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                doCreateDirectory(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }
        
        void WritableFileSystem::deleteFile(const Path& path) {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                doDeleteFile(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }
        
        void WritableFileSystem::moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            try {
                if (sourcePath.isAbsolute())
                    throw FileSystemException("Source path is absolute: '" + sourcePath.asString() + "'");
                if (destPath.isAbsolute())
                    throw FileSystemException("Destination path is absolute: '" + destPath.asString() + "'");
                doMoveFile(sourcePath, destPath, overwrite);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid source or destination path: '" + sourcePath.asString() + "', '" + destPath.asString() + "'", e);
            }
        }
    }
}
