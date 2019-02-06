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
#include "IO/CharArrayReader.h"
#include "IO/DiskFileSystem.h"

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
        IdPakFileSystem(nullptr, path, file) {}

        IdPakFileSystem::IdPakFileSystem(std::shared_ptr<FileSystem> next, const Path& path, MappedFile::Ptr file) :
        ImageFileSystem(std::move(next), path, file) {
            initialize();
        }

        void IdPakFileSystem::doReadDirectory() {
            char magic[PakLayout::HeaderMagicLength];
            char entryNameBuffer[PakLayout::EntryNameLength + 1];
            entryNameBuffer[PakLayout::EntryNameLength] = 0;

            CharArrayReader reader(m_file->begin(), m_file->end());
            reader.seekForward(PakLayout::HeaderAddress);
            reader.read(magic, PakLayout::HeaderMagicLength);

            const auto directoryAddress = reader.readSize<int32_t>();
            const auto directorySize = reader.readSize<int32_t>();
            const auto entryCount = directorySize / PakLayout::EntryLength;

            reader.seekFromBegin(directoryAddress);

            for (size_t i = 0; i < entryCount; ++i) {
                reader.read(entryNameBuffer, PakLayout::EntryNameLength);

                const auto entryName = String(entryNameBuffer);
                const auto entryAddress = reader.readSize<int32_t>();
                const auto entryLength = reader.readSize<int32_t>();

                const char* entryBegin = m_file->begin() + entryAddress;
                const char* entryEnd = entryBegin + entryLength;
                const Path filePath(StringUtils::toLower(entryName));
                const auto entryFile = std::make_shared<MappedFileView>(m_file, filePath, entryBegin, entryEnd);

                m_root.addFile(filePath, std::make_unique<SimpleFile>(entryFile));
            }
        }
    }
}
