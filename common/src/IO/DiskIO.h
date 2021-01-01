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

#include "IO/Path.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace IO {
        class File;

        namespace Disk {
            bool isCaseSensitive();

            Path fixPath(const Path& path);

            bool directoryExists(const Path& path);
            bool fileExists(const Path& path);

            std::vector<Path> getDirectoryContents(const Path& path);
            std::shared_ptr<File> openFile(const Path& path);
            std::string readTextFile(const Path& path);
            Path getCurrentWorkingDir();

            template <class M>
            void doFindItems(const Path& searchPath, const M& matcher, const bool recurse, std::vector<Path>& result) {
                for (const Path& itemPath : getDirectoryContents(searchPath)) {
                    const bool directory = directoryExists(searchPath + itemPath);
                    if (directory && recurse)
                        doFindItems(searchPath + itemPath, matcher, recurse, result);
                    if (matcher(searchPath + itemPath, directory))
                        result.push_back(searchPath + itemPath);
                }
            }

            template <typename M>
            std::vector<Path> findItems(const Path& path, const M& matcher) {
                std::vector<Path> result;
                doFindItems(path, matcher, false, result);
                return result;
            }

            std::vector<Path> findItems(const Path& path);

            template <typename M>
            std::vector<Path> findItemsRecursively(const Path& path, const M& matcher) {
                std::vector<Path> result;
                doFindItems(path, matcher, true, result);
                return result;
            }

            std::vector<Path> findItemsRecursively(const Path& path);

            void createFile(const Path& path, const std::string& contents);
            void createDirectory(const Path& path);
            void ensureDirectoryExists(const Path& path);
            void deleteFile(const Path& path);

            template <typename M>
            void deleteFiles(const Path& sourceDirPath, const M& matcher) {
                for (const Path& filePath : findItems(sourceDirPath, matcher))
                    deleteFile(filePath);
            }

            template <typename M>
            void deleteFilesRecursively(const Path& sourceDirPath, const M& matcher) {
                for (const Path& filePath : findItemsRecursively(sourceDirPath, matcher))
                    deleteFile(filePath);
            }

            void copyFile(const Path& sourcePath, const Path& destPath, bool overwrite);

            template <typename M>
            void copyFiles(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                for (const Path& filePath : findItems(sourceDirPath, matcher))
                    copyFile(filePath, destDirPath, overwrite);
            }

            template <typename M>
            void copyFilesRecursively(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                for (const Path& filePath : findItemsRecursively(sourceDirPath, matcher))
                    copyFile(filePath, destDirPath, overwrite);
            }

            void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite);

            template <typename M>
            void moveFiles(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                for (const Path& filePath : findItems(sourceDirPath, matcher))
                    moveFile(filePath, destDirPath, overwrite);
            }

            template <typename M>
            void moveFilesRecursively(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                for (const Path& filePath : findItemsRecursively(sourceDirPath, matcher))
                    moveFile(filePath, destDirPath, overwrite);
            }

            Path resolvePath(const std::vector<Path>& searchPaths, const Path& path);
        }
    }
}

