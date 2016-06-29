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

#include "WadFileSystem.h"

#include "IO/CharArrayReader.h"
#include "IO/DiskIO.h"

namespace TrenchBroom {
    namespace IO {
        namespace WadLayout {
            static const size_t NumEntriesAddress     = 4;
            static const size_t DirOffsetAddress      = 8;
            static const size_t DirEntryTypeOffset    = 4;
            static const size_t DirEntryNameOffset    = 3;
            static const size_t DirEntryNameSize      = 16;
            // static const size_t PalLength             = 256;
            // static const size_t TexWidthOffset        = 16;
            // static const size_t TexDataOffset         = 24;
            // static const size_t MaxTextureSize        = 1024;
        }

        namespace WadEntryType {
            // static const char WEStatus    = 'B';
            // static const char WEConsole   = 'C';
            // static const char WEMip       = 'D';
            // static const char WEPalette   = '@';
        }

        WadFileSystem::WadFileSystem(const Path& path) :
        ImageFileSystem(path, Disk::openFile(path)) {
            initialize();
        }
        
        WadFileSystem::WadFileSystem(const Path& path, MappedFile::Ptr file) :
        ImageFileSystem(path, file) {
            initialize();
        }

        void WadFileSystem::doReadDirectory() {
            CharArrayReader reader(m_file->begin(), m_file->end());

            reader.seekFromBegin(WadLayout::NumEntriesAddress);
            const size_t entryCount = reader.readSize<int32_t>();
            
            reader.seekFromBegin(WadLayout::DirOffsetAddress);
            const size_t directoryOffset = reader.readSize<int32_t>();
            
            reader.seekFromBegin(directoryOffset);
            for (size_t i = 0; i < entryCount; ++i) {
                const size_t entryAddress = reader.readSize<int32_t>();
                const size_t entrySize = reader.readSize<int32_t>();
                
                reader.seekForward(WadLayout::DirEntryTypeOffset);
                const char entryType = reader.readChar<char>();
                reader.seekForward(WadLayout::DirEntryNameOffset);
                const String entryName = reader.readString(WadLayout::DirEntryNameSize) + "." + entryType;
                
                const char* entryBegin = m_file->begin() + entryAddress;
                const char* entryEnd = entryBegin + entrySize;
                assert(entryEnd <= m_file->end());
                
                IO::Path path(entryName);
                MappedFile::Ptr file(new MappedFileView(m_file, path, entryBegin, entryEnd));
                m_root.addFile(path, file);
            }
        }
    }
}
