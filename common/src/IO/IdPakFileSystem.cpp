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

#include "IdPakFileSystem.h"

#include "CollectionUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        namespace PakLayout {
            static const size_t HeaderAddress     = 0x0;
            static const size_t HeaderMagicLength = 0x4;
            static const size_t EntryLength       = 0x40;
            static const size_t EntryNameLength   = 0x38;
            static const String HeaderMagic       = "PACK";
        }

        IdPakFileSystem::IdPakFileSystem(const Path& path, MappedFile::Ptr file) :
        ImageFileSystem(path, file) {
            initialize();
        }

        void IdPakFileSystem::doReadDirectory() {
            char magic[PakLayout::HeaderMagicLength];
            char entryNameBuffer[PakLayout::EntryNameLength + 1];
            entryNameBuffer[PakLayout::EntryNameLength] = 0;
            
            const char* cursor = m_file->begin() + PakLayout::HeaderAddress;
            readBytes(cursor, magic, PakLayout::HeaderMagicLength);
            
            const size_t directoryAddress = readSize<int32_t>(cursor);
            const size_t directorySize = readSize<int32_t>(cursor);
            const size_t entryCount = directorySize / PakLayout::EntryLength;
            
            assert(m_file->begin() + directoryAddress + directorySize <= m_file->end());
            cursor = m_file->begin() + directoryAddress;
            
            for (size_t i = 0; i < entryCount; ++i) {
                readBytes(cursor, entryNameBuffer, PakLayout::EntryNameLength);
                const String entryName(entryNameBuffer);
                const size_t entryAddress = readSize<int32_t>(cursor);
                const size_t entryLength = readSize<int32_t>(cursor);
                assert(m_file->begin() + entryAddress + entryLength <= m_file->end());
                
                const char* entryBegin = m_file->begin() + entryAddress;
                const char* entryEnd = entryBegin + entryLength;
                const Path filePath(StringUtils::toLower(entryName));
                const auto entryFile = std::make_shared<MappedFileView>(m_file, filePath, entryBegin, entryEnd);

                m_root.addFile(filePath, std::make_unique<SimpleFile>(entryFile));
            }
        }
    }
}
