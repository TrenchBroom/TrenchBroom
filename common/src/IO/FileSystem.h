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

#ifndef TrenchBroom_FileSystem
#define TrenchBroom_FileSystem

#include "StringUtils.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class FileSystem {
        public:
            class TypeMatcher {
            private:
                bool m_files;
                bool m_directories;
            public:
                TypeMatcher(const bool files = true, const bool directories = true);
                bool operator()(const Path& path, const bool directory) const;
            };

            class ExtensionMatcher {
            private:
                String m_extension;
            public:
                ExtensionMatcher(const String& extension);
                bool operator()(const Path& path, const bool directory) const;
            };
        public:
            virtual ~FileSystem();
            
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
        private:
            template <class Matcher>
            void doFindItems(const Path& searchPath, const Matcher& matcher, const bool recurse, Path::List& result) const {
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
            
            virtual bool doDirectoryExists(const Path& path) const = 0;
            virtual bool doFileExists(const Path& path) const = 0;
            
            virtual Path::List doGetDirectoryContents(const Path& path) const = 0;

            virtual const MappedFile::Ptr doOpenFile(const Path& path) const = 0;
        };
        
        class WritableFileSystem {
        public:
            virtual ~WritableFileSystem();
            
            void createDirectory(const Path& path);
            void deleteFile(const Path& path);
            void moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite);
        private:
            virtual void doCreateDirectory(const Path& path) = 0;
            virtual void doDeleteFile(const Path& path) = 0;
            virtual void doMoveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_FileSystem) */
