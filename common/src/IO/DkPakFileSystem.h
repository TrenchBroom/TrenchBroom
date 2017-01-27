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

#include "StringUtils.h"
#include "IO/ImageFileSystem.h"
#include "IO/Path.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class DkPakFileSystem : public ImageFileSystem {
        private:
            class CompressedFile : public File {
            private:
                MappedFile::Ptr m_file;
                const size_t m_uncompressedSize;
            public:
                CompressedFile(MappedFile::Ptr file, size_t uncompressedSize);
            private:
                MappedFile::Ptr doOpen();
                char* decompress() const;
            };
        public:
            DkPakFileSystem(const Path& path, MappedFile::Ptr file);
        private:
            void doReadDirectory();
            
        };
    }
}

#endif /* DkPakFileSystem_h */
