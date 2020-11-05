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

#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        FileSystem::FileSystem(std::shared_ptr<FileSystem> next) :
        m_next(std::move(next)) {}

        FileSystem::~FileSystem() {}

        bool FileSystem::hasNext() const {
            return m_next != nullptr;
        }

        const FileSystem& FileSystem::next() const {
            if (!m_next) {
                throw FileSystemException("File system chain ends here");
            }
            return *m_next;
        }

        std::shared_ptr<FileSystem> FileSystem::releaseNext() {
            return std::move(m_next);
        }

        bool FileSystem::canMakeAbsolute(const Path& path) const {
            return !path.isAbsolute();
        }

        Path FileSystem::makeAbsolute(const Path& path) const {
            try {
                if (!canMakeAbsolute(path)) {
                    throw FileSystemException("Cannot make absolute path of: '" + path.asString() + "'");
                }

                const auto result = _makeAbsolute(path);
                if (!result.isEmpty()) {
                    return result;
                } else {
                    // The path does not exist in any file system, make it absolute relative to this file system.
                    return doMakeAbsolute(path);
                }
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        bool FileSystem::directoryExists(const Path& path) const {
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }

                return _directoryExists(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        bool FileSystem::fileExists(const Path& path) const {
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }
                return _fileExists(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        std::vector<Path> FileSystem::findItemsWithBaseName(const Path& path, const std::vector<std::string>& extensions) const {
            if (path.isEmpty()) {
                return std::vector<Path>(0);
            }

            const auto directoryPath = path.deleteLastComponent();
            if (!directoryExists(directoryPath)) {
                return std::vector<Path>(0);
            }

            const auto basename = path.basename();
            return findItems(directoryPath, FileBasenameMatcher(basename, extensions));
        }

        std::vector<Path> FileSystem::findItems(const Path& directoryPath) const {
            return findItems(directoryPath, FileTypeMatcher());
        }

        std::vector<Path> FileSystem::findItemsRecursively(const Path& directoryPath) const {
            return findItemsRecursively(directoryPath, FileTypeMatcher());
        }

        std::vector<Path> FileSystem::getDirectoryContents(const Path& directoryPath) const {
            try {
                if (directoryPath.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + directoryPath.asString() + "'");
                }
                if (!directoryExists(directoryPath)) {
                    throw FileSystemException("Directory not found: '" + directoryPath.asString() + "'");
                }

                return _getDirectoryContents(directoryPath);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + directoryPath.asString() + "'", e);
            }
        }

        std::shared_ptr<File> FileSystem::openFile(const Path& path) const {
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }

                return _openFile(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        Path FileSystem::_makeAbsolute(const Path& path) const {
            if (doFileExists(path) || doDirectoryExists(path)) {
                // If the file is present in this file system, make it absolute here.
                return doMakeAbsolute(path);
            } else if (m_next) {
                // Otherwise, try the next one.
                return m_next->_makeAbsolute(path);
            } else {
                // Otherwise, the file does not exist in any file system in this hierarchy.
                // Return the empty path.
                return Path();
            }
        }

        bool FileSystem::_directoryExists(const Path& path) const {
            return doDirectoryExists(path) || (m_next && m_next->_directoryExists(path)) ;
        }

        bool FileSystem::_fileExists(const Path& path) const {
            return doFileExists(path) || (m_next && m_next->_fileExists(path));
        }

        std::vector<Path> FileSystem::_getDirectoryContents(const Path& directoryPath) const {
            auto result = doGetDirectoryContents(directoryPath);
            if (m_next) {
                result = kdl::vec_concat(std::move(result), m_next->_getDirectoryContents(directoryPath));
            }

            kdl::vec_sort_and_remove_duplicates(result);
            return result;
        }

        std::shared_ptr<File> FileSystem::_openFile(const Path& path) const {
            if (doFileExists(path)) {
                return doOpenFile(path);
            } else if (m_next) {
                return m_next->_openFile(path);
            } else {
                throw FileSystemException("File not found: '" + path.asString() + "'");
            }
        }

        bool FileSystem::doCanMakeAbsolute(const Path& /* path */) const {
            return false;
        }

        Path FileSystem::doMakeAbsolute(const Path& path) const {
            throw FileSystemException("Cannot make absolute path of '" + path.asString() + "'");
        }

        WritableFileSystem::WritableFileSystem() = default;
        WritableFileSystem::~WritableFileSystem() = default;

        void WritableFileSystem::createFileAtomic(const Path& path, const std::string& contents) {
            const auto tmpPath = path.addExtension("tmp");
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }
                doCreateFile(tmpPath, contents);
                doMoveFile(tmpPath, path, true);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        void WritableFileSystem::createFile(const Path& path, const std::string& contents) {
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }
                doCreateFile(path, contents);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        void WritableFileSystem::createDirectory(const Path& path) {
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }
                doCreateDirectory(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        void WritableFileSystem::deleteFile(const Path& path) {
            try {
                if (path.isAbsolute()) {
                    throw FileSystemException("Path is absolute: '" + path.asString() + "'");
                }
                doDeleteFile(path);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid path: '" + path.asString() + "'", e);
            }
        }

        void WritableFileSystem::copyFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            try {
                if (sourcePath.isAbsolute()) {
                    throw FileSystemException("Source path is absolute: '" + sourcePath.asString() + "'");
                }
                if (destPath.isAbsolute()) {
                    throw FileSystemException("Destination path is absolute: '" + destPath.asString() + "'");
                }
                doCopyFile(sourcePath, destPath, overwrite);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid source or destination path: '" + sourcePath.asString() + "', '" + destPath.asString() + "'", e);
            }
        }

        void WritableFileSystem::moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            try {
                if (sourcePath.isAbsolute()) {
                    throw FileSystemException("Source path is absolute: '" + sourcePath.asString() + "'");
                }
                if (destPath.isAbsolute()) {
                    throw FileSystemException("Destination path is absolute: '" + destPath.asString() + "'");
                }
                doMoveFile(sourcePath, destPath, overwrite);
            } catch (const PathException& e) {
                throw FileSystemException("Invalid source or destination path: '" + sourcePath.asString() + "', '" + destPath.asString() + "'", e);
            }
        }
    }
}
