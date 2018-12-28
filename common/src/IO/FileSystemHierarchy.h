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

#ifndef TrenchBroom_FileSystemHierarchy
#define TrenchBroom_FileSystemHierarchy

#include "Macros.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class FileSystemHierarchy : public virtual FileSystem {
        private:
            typedef std::vector<std::unique_ptr<FileSystem>> FileSystemList;
            FileSystemList m_fileSystems;
        public:
            FileSystemHierarchy();
            virtual ~FileSystemHierarchy() override;
            
            void pushFileSystem(std::unique_ptr<FileSystem> fileSystem);
            void popFileSystem();
            virtual void clear();
        private:
            Path doMakeAbsolute(const Path& relPath) const override;
            bool doDirectoryExists(const Path& path) const override;
            bool doFileExists(const Path& path) const override;
            FileSystem* findFileSystemContaining(const Path& path) const;
            
            Path::List doGetDirectoryContents(const Path& path) const override;
            const MappedFile::Ptr doOpenFile(const Path& path) const override;

            deleteCopyAndAssignment(FileSystemHierarchy)
        };
        
#ifdef _MSC_VER
// MSVC complains about the fact that this class inherits some (pure virtual) method declarations several times from different base classes, even though there is only one definition.
#pragma warning(push)
#pragma warning(disable : 4250)
#endif
        class WritableFileSystemHierarchy : public FileSystemHierarchy, public WritableFileSystem {
        private:
            WritableFileSystem* m_writableFileSystem;
        public:
            WritableFileSystemHierarchy();
            
            void pushReadableFileSystem(std::unique_ptr<FileSystem> fileSystem);
            void pushWritableFileSystem(std::unique_ptr<WritableFileSystem> fileSystem);
            void clear() override;
        private:
            void doCreateFile(const Path& path, const String& contents) override;
            void doCreateDirectory(const Path& path) override;
            void doDeleteFile(const Path& path) override;
            void doCopyFile(const Path& sourcePath, const Path& destPath, bool overwrite) override;
            void doMoveFile(const Path& sourcePath, const Path& destPath, bool overwrite) override;
            
            deleteCopyAndAssignment(WritableFileSystemHierarchy)
        };
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    }
}

#endif /* defined(TrenchBroom_FileSystemHierarchy) */
