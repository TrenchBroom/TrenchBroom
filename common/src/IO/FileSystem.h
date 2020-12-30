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

#pragma once

#include "Exceptions.h"
#include "Macros.h"
#include "IO/Path.h"

#include <kdl/vector_utils.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class File;
        class Path;

        class FileSystem {
            deleteCopyAndMove(FileSystem)
        protected:
            /**
             * Next filesystem in the search path.
             *
             * NOTE: the use of std::shared_ptr is because there is shared ownership during construction of the
             * filesystems (std::unique_ptr would require std::move'ing the existing chain of filesystems when passing it
             * to the FileSystem constructor, which means if the constructor throws, the existing filesystem chain
             * gets destroyed. FileSystem constructors throw if there is an error creating the filesystem,
             * so std::unique_ptr isn't usable with this design.)
             */
            std::shared_ptr<FileSystem> m_next;
        public: // public API
            explicit FileSystem(std::shared_ptr<FileSystem> next = std::shared_ptr<FileSystem>());
            virtual ~FileSystem();

            bool hasNext() const;
            const FileSystem& next() const;
            std::shared_ptr<FileSystem> releaseNext();

            bool canMakeAbsolute(const Path& path) const;
            Path makeAbsolute(const Path& path) const;

            bool directoryExists(const Path& path) const;
            bool fileExists(const Path& path) const;

            /**
             * Find all files with the same path and base name as the given path, regardless of the extension.
             *
             * @param path the file path
             * @param extensions a list of extensions to match
             * @return a list of paths
             */
            std::vector<Path> findItemsWithBaseName(const Path& path, const std::vector<std::string>& extensions) const;

            /**
             * Find all items in the given directory that match the given matcher.
             *
             * @tparam Matcher the type of the matcher
             * @param directoryPath the path to a directory to search
             * @param matcher the matcher
             * @return the paths to the items that matched the query
             */
            template <class Matcher>
            std::vector<Path> findItems(const Path& directoryPath, const Matcher& matcher) const {
                return findItems(directoryPath, matcher, false);
            }

            /**
             * Find all items in the given directory.
             *
             * @param directoryPath the path to a directory to search
             * @return the paths to the items in the given directory
             */
            std::vector<Path> findItems(const Path& directoryPath) const;

            /**
             * Find all items in the given directory and any sub directories that match the given matcher.
             *
             * @tparam Matcher the type of the matcher
             * @param directoryPath the path to a directory to search
             * @param matcher the matcher
             * @return the paths to the items that matched the query
             */
            template <class Matcher>
            std::vector<Path> findItemsRecursively(const Path& directoryPath, const Matcher& matcher) const {
                return findItems(directoryPath, matcher, true);
            }

            /**
             * Find all items in the given directory and any sub directories.
             *
             * @param directoryPath the path to a directory to search
             * @return the paths to the items that matched the query
             */
            std::vector<Path> findItemsRecursively(const Path& directoryPath) const;

            std::vector<Path> getDirectoryContents(const Path& directoryPath) const;
            std::shared_ptr<File> openFile(const Path& path) const;
        private: // private API to be used for chaining, avoids multiple checks of parameters
            bool _canMakeAbsolute(const Path& path) const;
            Path _makeAbsolute(const Path& path) const;
            bool _directoryExists(const Path& path) const;
            bool _fileExists(const Path& path) const;
            std::vector<Path> _getDirectoryContents(const Path& directoryPath) const;
            std::shared_ptr<File> _openFile(const Path& path) const;

            /**
             * Finds all items matching the given matcher at the given search path, optionally recursively. This method
             * performs parameter checks against the search path.
             *
             * Delegates the query to the next file system if one exists.
             *
             * @tparam M the matcher type
             * @param searchPath the search path at which to search for matches
             * @param matcher the matcher to apply to candidates
             * @param recurse whether or not to recurse into sub directories
             * @return the matching paths
             */
            template <class M>
            std::vector<Path> findItems(const Path& searchPath, const M& matcher, const bool recurse) const {
                try {
                    if (searchPath.isAbsolute()) {
                        throw FileSystemException("Path is absolute: '" + searchPath.asString() + "'");
                    }

                    if (!directoryExists(searchPath)) {
                        throw FileSystemException("Directory not found: '" + searchPath.asString() + "'");
                    }

                    std::vector<Path> result;
                    _findItems(searchPath, matcher, recurse, result);
                    return kdl::vec_sort_and_remove_duplicates(std::move(result));
                } catch (const PathException& e) {
                    throw FileSystemException("Invalid path: '" + searchPath.asString() + "'", e);
                }
            }

            /**
             * Finds all items matching the given matcher at the given search path, optionally recursively, and adds
             * the matches to the given result.
             *
             * Delegates the query to the next file system if one exists.
             *
             * @tparam M the matcher type
             * @param searchPath the search path at which to search for matches
             * @param matcher the matcher to apply to candidates
             * @param recurse whether or not to recurse into sub directories
             * @param result collects the matching paths
             */
            template <class M>
            void _findItems(const Path& searchPath, const M& matcher, const bool recurse, std::vector<Path>& result) const {
                doFindItems(searchPath, matcher, recurse, result);
                if (m_next) {
                    m_next->_findItems(searchPath, matcher, recurse, result);
                }
            }

            /**
             * Finds all items matching the given matcher at the given search path, optionally recursively, and adds
             * the matches to the given result.
             *
             * @tparam M the matcher type
             * @param searchPath the search path at which to search for matches
             * @param matcher the matcher to apply to candidates
             * @param recurse whether or not to recurse into sub directories
             * @param result collects the matching paths
             */
            template <class M>
            void doFindItems(const Path& searchPath, const M& matcher, const bool recurse, std::vector<Path>& result) const {
                if (doDirectoryExists(searchPath)) {
                    for (const auto& itemPath : doGetDirectoryContents(searchPath)) {
                        const auto directory = doDirectoryExists(searchPath + itemPath);
                        if (directory && recurse) {
                            doFindItems(searchPath + itemPath, matcher, recurse, result);
                        }
                        if (matcher(searchPath + itemPath, directory)) {
                            result.push_back(searchPath + itemPath);
                        }
                    }
                }
            }
        private: // subclassing API
            virtual bool doCanMakeAbsolute(const Path& path) const;
            virtual Path doMakeAbsolute(const Path& path) const;

            virtual bool doDirectoryExists(const Path& path) const = 0;
            virtual bool doFileExists(const Path& path) const = 0;

            virtual std::vector<Path> doGetDirectoryContents(const Path& path) const = 0;

            virtual std::shared_ptr<File> doOpenFile(const Path& path) const = 0;
        };

        class WritableFileSystem {
            deleteCopyAndMove(WritableFileSystem)
        public:
            WritableFileSystem();
            virtual ~WritableFileSystem();

            /**
             * Creates a temporary fiel with the given contens, then moves that file to its final
             * location at the given path.
             *
             * If file creation fails, the temporary file may not be cleaned up.
             */
            void createFileAtomic(const Path& path, const std::string& contents);
            void createFile(const Path& path, const std::string& contents);
            void createDirectory(const Path& path);
            void deleteFile(const Path& path);
            void copyFile(const Path& sourcePath, const Path& destPath, bool overwrite);
            void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite);
        private:
            virtual void doCreateFile(const Path& path, const std::string& contents) = 0;
            virtual void doCreateDirectory(const Path& path) = 0;
            virtual void doDeleteFile(const Path& path) = 0;
            virtual void doCopyFile(const Path& sourcePath, const Path& destPath, bool overwrite) = 0;
            virtual void doMoveFile(const Path& sourcePath, const Path& destPath, bool overwrite) = 0;
        };
    }
}

