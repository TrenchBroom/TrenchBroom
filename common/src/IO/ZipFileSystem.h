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

#ifndef TRENCHBROOM_ZIPFILESYSTEM_H
#define TRENCHBROOM_ZIPFILESYSTEM_H


#include "StringUtils.h"
#include "IO/ImageFileSystem.h"
#include "IO/Path.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        class MinizArchive;

        class ZipFileSystem : public ImageFileSystem {
        private:
            class ZipCompressedFile : public File {
            private:
                std::shared_ptr<MinizArchive> m_archive;
                unsigned int m_fileIndex;
            public:
                ZipCompressedFile(std::shared_ptr<MinizArchive> archive, unsigned int fileIndex);
            private:
                MappedFile::Ptr doOpen() const override;
            };
        public:
            ZipFileSystem(const Path& path, MappedFile::Ptr file);
            ZipFileSystem(std::shared_ptr<FileSystem> next, const Path& path, MappedFile::Ptr file);
        private:
            void doReadDirectory() override;
        };
    }
}


#endif //TRENCHBROOM_ZIPFILESYSTEM_H
