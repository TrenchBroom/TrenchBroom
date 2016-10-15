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

#ifndef DiskIO_h
#define DiskIO_h

#include "Functor.h"

#include "IO/FileMatcher.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        namespace Disk {
            bool isCaseSensitive();
            
            Path fixPath(const Path& path);
            
            bool directoryExists(const Path& path);
            bool fileExists(const Path& path);
            
            String replaceForbiddenChars(const String& name);
            
            Path::List getDirectoryContents(const Path& path);
            MappedFile::Ptr openFile(const Path& path);
            Path getCurrentWorkingDir();
            
            template <class M>
            void doFindItems(const Path& searchPath, const M& matcher, const bool recurse, Path::List& result) {
                const Path::List contents = getDirectoryContents(searchPath);
                Path::List::const_iterator it, end;
                for (it = contents.begin(), end = contents.end(); it != end; ++it) {
                    const Path& itemPath = *it;
                    const bool directory = directoryExists(searchPath + itemPath);
                    if (directory && recurse)
                        doFindItems(searchPath + itemPath, matcher, recurse, result);
                    if (matcher(searchPath + itemPath, directory))
                        result.push_back(searchPath + itemPath);
                }
            }
            
            template <typename M>
            Path::List findItems(const Path& path, const M& matcher) {
                Path::List result;
                doFindItems(path, matcher, false, result);
                return result;
            }
            
            Path::List findItems(const Path& path);
            
            template <typename M>
            Path::List findItemsRecursively(const Path& path, const M& matcher) {
                Path::List result;
                doFindItems(path, matcher, true, result);
                return result;
            }
            
            Path::List findItemsRecursively(const Path& path);
            
            void createFile(const Path& path, const String& contents);
            void createDirectory(const Path& path);
            void deleteFile(const Path& path);

            template <typename M>
            void deleteFiles(const Path& sourceDirPath, const M& matcher) {
                const Path::List files = findItems(sourceDirPath, matcher);
                Path::List::const_iterator it, end;
                for (it = files.begin(), end = files.end(); it != end; ++it) {
                    const Path& filePath = *it;
                    deleteFile(filePath);
                }
            }
            
            template <typename M>
            void deleteFilesRecursively(const Path& sourceDirPath, const M& matcher) {
                const Path::List files = findItemsRecursively(sourceDirPath, matcher);
                Path::List::const_iterator it, end;
                for (it = files.begin(), end = files.end(); it != end; ++it) {
                    const Path& filePath = *it;
                    deleteFile(filePath);
                }
            }
            
            void copyFile(const Path& sourcePath, const Path& destPath, bool overwrite);
            
            template <typename M>
            void copyFiles(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                const Path::List files = findItems(sourceDirPath, matcher);
                Path::List::const_iterator it, end;
                for (it = files.begin(), end = files.end(); it != end; ++it) {
                    const Path& filePath = *it;
                    copyFile(filePath, destDirPath, overwrite);
                }
            }
            
            template <typename M>
            void copyFilesRecursively(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                const Path::List files = findItemsRecursively(sourceDirPath, matcher);
                Path::List::const_iterator it, end;
                for (it = files.begin(), end = files.end(); it != end; ++it) {
                    const Path& filePath = *it;
                    copyFile(filePath, destDirPath, overwrite);
                }
            }
            
            void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite);
            
            template <typename M>
            void moveFiles(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                const Path::List files = findItems(sourceDirPath, matcher);
                Path::List::const_iterator it, end;
                for (it = files.begin(), end = files.end(); it != end; ++it) {
                    const Path& filePath = *it;
                    moveFile(filePath, destDirPath, overwrite);
                }
            }
            
            template <typename M>
            void moveFilesRecursively(const Path& sourceDirPath, const M& matcher, const Path& destDirPath, const bool overwrite) {
                const Path::List files = findItemsRecursively(sourceDirPath, matcher);
                Path::List::const_iterator it, end;
                for (it = files.begin(), end = files.end(); it != end; ++it) {
                    const Path& filePath = *it;
                    moveFile(filePath, destDirPath, overwrite);
                }
            }
            
            Path resolvePath(const Path::List& searchPaths, const Path& path);
        }
    }
}

#endif /* DiskIO_h */
