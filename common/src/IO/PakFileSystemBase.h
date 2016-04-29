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

#ifndef PakFileSystemBase_h
#define PakFileSystemBase_h

#include "StringUtils.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class PakFileSystemBase : public FileSystem {
        protected:
            class File {
            public:
                virtual ~File();
                
                MappedFile::Ptr open();
            private:
                virtual MappedFile::Ptr doOpen() = 0;
            };
            
            class SimpleFile : public File {
            private:
                MappedFile::Ptr m_file;
            public:
                SimpleFile(const char* begin, const char* end);
            private:
                MappedFile::Ptr doOpen();
            };
            
            class Directory {
            private:
                typedef std::map<String, Directory*> DirMap;
                typedef std::map<String, File*> FileMap;
                
                Path m_path;
                DirMap m_directories;
                FileMap m_files;
            public:
                Directory(const Path& path);
                ~Directory();
                
                void addFile(const Path& path, File* file);
                
                bool directoryExists(const Path& path) const;
                bool fileExists(const Path& path) const;
                
                const Directory& findDirectory(const Path& path) const;
                const MappedFile::Ptr findFile(const Path& path) const;
                Path::List contents() const;
            private:
                Directory& findOrCreateDirectory(const Path& path);
            };
        protected:
            Path m_path;
            MappedFile::Ptr m_file;
            Directory m_root;
        protected:
            PakFileSystemBase(const Path& path, MappedFile::Ptr file);
        public:
            virtual ~PakFileSystemBase();
        protected:
            void initialize();
        private:
            Path doMakeAbsolute(const Path& relPath) const;
            bool doDirectoryExists(const Path& path) const;
            bool doFileExists(const Path& path) const;
            
            Path::List doGetDirectoryContents(const Path& path) const;
            const MappedFile::Ptr doOpenFile(const Path& path) const;
        private:
            virtual void doReadDirectory() = 0;
        };
    }
}

#endif /* PakFileSystemBase_h */
