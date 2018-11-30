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

#ifndef TrenchBroom_FileSystem
#define TrenchBroom_FileSystem

#include "Functor.h"
#include "StringUtils.h"
#include "IO/DiskIO.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class FileSystem {
        public:
            FileSystem();
            FileSystem(const FileSystem& other);
            virtual ~FileSystem();
            
            FileSystem& operator=(const FileSystem& other);

            Path makeAbsolute(const Path& relPath) const;
            
            bool directoryExists(const Path& path) const;
            bool fileExists(const Path& path) const;

            template <class Matcher>
            Path::List findItems(const Path& path, const Matcher& matcher) const {
                Path::List result;
                doFindItems(path, matcher, false, result);
                return result;
            }
            Path::List findItems(const Path& path) const;
            
            template <class Matcher>
            Path::List findItemsRecursively(const Path& path, const Matcher& matcher) const {
                Path::List result;
                doFindItems(path, matcher, true, result);
                return result;
            }
            Path::List findItemsRecursively(const Path& path) const;
            
            Path::List getDirectoryContents(const Path& path) const;
            const MappedFile::Ptr openFile(const Path& path) const;

            /**
             * Resolves file system links. The default implementation just returns the given path, but subclasses may
             * override doResolve if they support file system links. This method is called when a file is opened.
             *
             * @param path the path to resolve
             * @return the resolved path
             */
            Path resolve(const Path& path) const;
        private:
            template <class M>
            void doFindItems(const Path& searchPath, const M& matcher, const bool recurse, Path::List& result) const {
                for (const Path& itemPath : getDirectoryContents(searchPath)) {
                    const bool directory = directoryExists(searchPath + itemPath);
                    if (directory && recurse)
                        doFindItems(searchPath + itemPath, matcher, recurse, result);
                    if (matcher(searchPath + itemPath, directory))
                        result.push_back(searchPath + itemPath);
                }
            }

            virtual Path doMakeAbsolute(const Path& relPath) const = 0;
            virtual bool doDirectoryExists(const Path& path) const = 0;
            virtual bool doFileExists(const Path& path) const = 0;
            
            virtual Path::List doGetDirectoryContents(const Path& path) const = 0;

            virtual const MappedFile::Ptr doOpenFile(const Path& path) const = 0;
            virtual Path doResolve(const Path& path) const;
        };
        
        class WritableFileSystem : public virtual FileSystem {
        public:
            WritableFileSystem();
            WritableFileSystem(const WritableFileSystem& other);
            virtual ~WritableFileSystem();
            
            WritableFileSystem& operator=(const WritableFileSystem& other);
            
            void createFile(const Path& path, const String& contents);
            void createDirectory(const Path& path);
            void deleteFile(const Path& path);
            void copyFile(const Path& sourcePath, const Path& destPath, bool overwrite);
            void moveFile(const Path& sourcePath, const Path& destPath, bool overwrite);
        private:
            virtual void doCreateFile(const Path& path, const String& contents) = 0;
            virtual void doCreateDirectory(const Path& path) = 0;
            virtual void doDeleteFile(const Path& path) = 0;
            virtual void doCopyFile(const Path& sourcePath, const Path& destPath, bool overwrite) = 0;
            virtual void doMoveFile(const Path& sourcePath, const Path& destPath, bool overwrite) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_FileSystem) */
