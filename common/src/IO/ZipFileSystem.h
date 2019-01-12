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

// FIXME: port to something non-wx
#if 0
#include "StringUtils.h"
#include "IO/ImageFileSystem.h"
#include "IO/Path.h"

#include <memory>

class wxZipInputStream;
class wxZipEntry;

namespace TrenchBroom {
    namespace IO {
        class ZipFileSystem : public ImageFileSystem {
        private:
            class ZipCompressedFile : public File {
            private:
                std::shared_ptr<wxZipInputStream> m_stream;
                std::unique_ptr<wxZipEntry> m_entry;
            public:
                ZipCompressedFile(std::shared_ptr<wxZipInputStream> stream, std::unique_ptr<wxZipEntry> entry);
            private:
                MappedFile::Ptr doOpen() const override;
            };
        public:
            ZipFileSystem(const Path& path, MappedFile::Ptr file);
            ZipFileSystem(std::unique_ptr<FileSystem> next, const Path& path, MappedFile::Ptr file);
        private:
            void doReadDirectory() override;
        };
    }
}
#endif

#endif //TRENCHBROOM_ZIPFILESYSTEM_H
