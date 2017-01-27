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

#include "FileSystem.h"

#include "Exceptions.h"

#include "IO/FileMatcher.h"

namespace TrenchBroom {
    namespace IO {
        FileSystem::FileSystem() {}

        FileSystem::FileSystem(const FileSystem& other) {}

        FileSystem::~FileSystem() {}

        FileSystem& FileSystem::operator=(const FileSystem& other) { return *this; }

        Path FileSystem::makeAbsolute(const Path& relPath) const {
            return doMakeAbsolute(relPath);
        }

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
            return findItems(path, FileTypeMatcher());
        }

        Path::List FileSystem::findItemsRecursively(const Path& path) const {
            return findItemsRecursively(path, FileTypeMatcher());
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

        WritableFileSystem::WritableFileSystem() {}

        /*
         GCC complains about the call to the base class initializer missing, and Clang complains
         about it being there. We decide to keep it there and silence the Clang warning.
         */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wabstract-vbase-init"
#endif
        WritableFileSystem::WritableFileSystem(const WritableFileSystem& other) :
        FileSystem() {}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        WritableFileSystem::~WritableFileSystem() {}

        WritableFileSystem& WritableFileSystem::operator=(const WritableFileSystem& other) { return *this; }

        void WritableFileSystem::createFile(const Path& path, const String& contents) {
            try {
                if (path.isAbsolute())
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                doCreateFile(path, contents);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

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

        void WritableFileSystem::copyFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            try {
                if (sourcePath.isAbsolute())
                    throw FileSystemException("Source path is absolute: '" + sourcePath.asString() + "'");
                if (destPath.isAbsolute())
                    throw FileSystemException("Destination path is absolute: '" + destPath.asString() + "'");
                doCopyFile(sourcePath, destPath, overwrite);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid source or destination path: '" + sourcePath.asString() + "', '" + destPath.asString() + "'", e);
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
