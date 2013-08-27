/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "PakFS.h"

#include "Exceptions.h"
#include "IO/IOUtils.h"
#include "IO/MappedFile.h"

namespace TrenchBroom {
    namespace IO {
        namespace PakLayout {
            static const size_t HeaderAddress     = 0x0;
            static const size_t HeaderMagicLength = 0x4;
            static const size_t EntryLength       = 0x40;
            static const size_t EntryNameLength   = 0x38;
            static const String HeaderMagic       = "PACK";
        }

        PakFS::PakFS(const Path& path) :
        m_path(path) {
            FileSystem fs;
            m_file = fs.mapFile(m_path, std::ios::in);
            if (m_file == NULL)
                throw FileSystemException("Cannot open file " + m_path.asString());
            readDirectory();
        }

        const MappedFile::Ptr PakFS::doFindFile(const Path& path) const {
            const IO::Path lcPath(StringUtils::toLower(path.asString()));
            PakDirectory::const_iterator it = m_directory.find(lcPath);
            if (it == m_directory.end())
                return MappedFile::Ptr();
            return it->second;
        }

        String PakFS::doGetLocation() const {
            return m_path.asString();
        }

        void PakFS::readDirectory() {
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
                const IO::Path path(StringUtils::toLower(entryName));
                
                m_directory[path] = MappedFile::Ptr(new MappedFileView(entryBegin, entryEnd));
            }
        }
    }
}
