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

#ifndef ImageFileSystem_h
#define ImageFileSystem_h

#include "StringUtils.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class ImageFileSystem : public FileSystem {
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
                SimpleFile(MappedFile::Ptr file);
            private:
                MappedFile::Ptr doOpen() override;
            };
            
            class Directory {
            private:
                typedef std::map<Path, Directory*, Path::Less<StringUtils::CaseInsensitiveStringLess>> DirMap;
                typedef std::map<Path, File*,      Path::Less<StringUtils::CaseInsensitiveStringLess>> FileMap;
                
                Path m_path;
                DirMap m_directories;
                FileMap m_files;
            public:
                Directory(const Path& path);
                ~Directory();
                
                void addFile(const Path& path, MappedFile::Ptr file);
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
            ImageFileSystem(const Path& path, MappedFile::Ptr file);
        public:
            virtual ~ImageFileSystem() override;
        protected:
            void initialize();
        private:
            Path doMakeAbsolute(const Path& relPath) const override;
            bool doDirectoryExists(const Path& path) const override;
            bool doFileExists(const Path& path) const override;
            
            Path::List doGetDirectoryContents(const Path& path) const override;
            const MappedFile::Ptr doOpenFile(const Path& path) const override;
        private:
            virtual void doReadDirectory() = 0;
        };
    }
}

#endif /* ImageFileSystem_h */
