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

#include "WadFileSystem.h"

#include "IO/CharArrayReader.h"
#include "IO/DiskIO.h"

namespace TrenchBroom {
    namespace IO {
        namespace WadLayout {
            static const size_t MinFileSize           = 12;
            static const size_t MagicOffset           = 0;
            static const size_t MagicSize             = 4;
            static const size_t NumEntriesAddress     = 4;
            static const size_t DirOffsetAddress      = 8;
            static const size_t DirEntryTypeOffset    = 4;
            static const size_t DirEntryNameOffset    = 3;
            static const size_t DirEntryNameSize      = 16;
            static const size_t DirEntrySize          = 32;
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
            if (m_file->size() < WadLayout::MinFileSize) {
                throw FileSystemException("File does not contain a directory.");
            }

            reader.seekFromBegin(WadLayout::MagicOffset);
            const auto magic = reader.readString(WadLayout::MagicSize);
            if (StringUtils::toLower(magic) != "wad2" && StringUtils::toLower(magic) != "wad3") {
                throw FileSystemException("Unknown wad file type '" + magic + "'");
            }

            reader.seekFromBegin(WadLayout::NumEntriesAddress);
            const auto entryCount = reader.readSize<int32_t>();

            if (m_file->size() < WadLayout::MinFileSize + entryCount * WadLayout::DirEntrySize) {
                throw FileSystemException("File does not contain a directory");
            }

            reader.seekFromBegin(WadLayout::DirOffsetAddress);
            const auto directoryOffset = reader.readSize<int32_t>();

            if (m_file->size() < directoryOffset + entryCount * WadLayout::DirEntrySize) {
                throw FileSystemException("File directory is out of bounds.");
            }
            
            reader.seekFromBegin(directoryOffset);
            for (size_t i = 0; i < entryCount; ++i) {
                const auto entryAddress = reader.readSize<int32_t>();
                const auto entrySize = reader.readSize<int32_t>();

                if (m_file->size() < entryAddress + entrySize) {
                    auto msg = StringStream();
                    msg << "File entry at address " << entryAddress << " is out of bounds";
                    throw FileSystemException(msg.str()) ;
                }

                reader.seekForward(WadLayout::DirEntryTypeOffset);
                const auto entryType = reader.readChar<char>();
                reader.seekForward(WadLayout::DirEntryNameOffset);
                const auto entryName = reader.readString(WadLayout::DirEntryNameSize) + "." + entryType;
                
                const auto* entryBegin = m_file->begin() + entryAddress;
                const auto* entryEnd = entryBegin + entrySize;
                assert(entryEnd <= m_file->end());
                
                const auto path = IO::Path(entryName);
                auto file = std::make_shared<MappedFileView>(m_file, path, entryBegin, entryEnd);
                m_root.addFile(path, file);
            }
        }
    }
}
