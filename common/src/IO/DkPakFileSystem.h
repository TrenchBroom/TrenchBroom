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

#ifndef DkPakFileSystem_h
#define DkPakFileSystem_h

#include "IO/ImageFileSystem.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        class Path;

        class DkPakFileSystem : public ImageFileSystem {
        private:
            class DkCompressedFile : public CompressedFileEntry {
            public:
                using CompressedFileEntry::CompressedFileEntry;
            private:
                std::unique_ptr<char[]> decompress(std::shared_ptr<File> file, size_t uncompressedSize) const override;
            };
        public:
            explicit DkPakFileSystem(const Path& path);
            DkPakFileSystem(std::shared_ptr<FileSystem> next, const Path& path);
        private:
            void doReadDirectory() override;

        };
    }
}

#endif /* DkPakFileSystem_h */
