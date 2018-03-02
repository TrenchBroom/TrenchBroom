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

#include "DkPakFileSystem.h"

#include "CollectionUtils.h"
#include "IO/CharArrayReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <cassert>
#include <cstring>

namespace TrenchBroom {
    namespace IO {
        namespace PakLayout {
            static const size_t HeaderMagicLength = 0x4;
            static const size_t EntryLength       = 0x48;
            static const size_t EntryNameLength   = 0x38;
            static const String HeaderMagic       = "PACK";
        }
        
        DkPakFileSystem::CompressedFile::CompressedFile(MappedFile::Ptr file, const size_t uncompressedSize) :
        m_file(file),
        m_uncompressedSize(uncompressedSize) {}

        MappedFile::Ptr DkPakFileSystem::CompressedFile::doOpen() {
            const char* data = decompress();
            return MappedFile::Ptr(new MappedFileBuffer(m_file->path(), data, m_uncompressedSize));
        }

        char* DkPakFileSystem::CompressedFile::decompress() const {
            CharArrayReader reader(m_file->begin(), m_file->end());
            
            char* result = new char[m_uncompressedSize];
            char* curTarget = result;
            
            unsigned char x = reader.readUnsignedChar<unsigned char>();
            while (!reader.eof() && x < 0xFF) {
                if (x < 0x40) {
                    // x+1 bytes of uncompressed data follow (just read+write them as they are)
                    const size_t len = static_cast<size_t>(x) + 1;
                    reader.read(curTarget, len);
                    curTarget += len;
                } else if (x < 0x80) {
                    // run-length encoded zeros, write (x - 62) zero-bytes to output
                    const size_t len = static_cast<size_t>(x) - 62;
                    std::memset(curTarget, 0, len);
                    curTarget += len;
                } else if (x < 0xC0) {
                    // run-length encoded data, read one byte, write it (x-126) times to output
                    const size_t len = static_cast<size_t>(x) - 126;
                    const int data = reader.readInt<unsigned char>();
                    std::memset(curTarget, data, len);
                    curTarget += len;
                } else if (x < 0xFE) {
                    // this references previously uncompressed data
                    // read one byte to get _offset_
                    // read (x-190) bytes from the already uncompressed and written output data,
                    // starting at (offset+2) bytes before the current write position (and add them to output, of course)
                    const size_t len = static_cast<size_t>(x) - 190;
                    const size_t offset = reader.readSize<unsigned char>();
                    char* from = curTarget - (offset + 2);
                    
                    assert(from >= result);
                    assert(from <=  curTarget - len);

                    std::memcpy(curTarget, from, len);
                    curTarget += len;
                }

                x = reader.readUnsignedChar<unsigned char>();
            }
            
            return result;
        }
        
        DkPakFileSystem::DkPakFileSystem(const Path& path, MappedFile::Ptr file) :
        ImageFileSystem(path, file) {
            initialize();
        }
        
        void DkPakFileSystem::doReadDirectory() {
            CharArrayReader reader(m_file->begin(), m_file->end());
            reader.seekFromBegin(PakLayout::HeaderMagicLength);

            const size_t directoryAddress = reader.readSize<int32_t>();
            const size_t directorySize = reader.readSize<int32_t>();
            const size_t entryCount = directorySize / PakLayout::EntryLength;
            
            reader.seekFromBegin(directoryAddress);
            
            for (size_t i = 0; i < entryCount; ++i) {
                const String entryName = reader.readString(PakLayout::EntryNameLength);
                const size_t entryAddress = reader.readSize<int32_t>();
                const size_t uncompressedSize = reader.readSize<int32_t>();
                const size_t compressedSize = reader.readSize<int32_t>();
                const bool compressed = reader.readBool<int32_t>();
                const size_t entrySize = compressed ? compressedSize : uncompressedSize;

                const char* entryBegin = m_file->begin() + entryAddress;
                const char* entryEnd = entryBegin + entrySize;
                const Path filePath(StringUtils::toLower(entryName));
                MappedFile::Ptr entryFile(new MappedFileView(m_file, filePath, entryBegin, entryEnd));
                
                if (compressed)
                    m_root.addFile(filePath, new CompressedFile(entryFile, uncompressedSize));
                else
                    m_root.addFile(filePath, new SimpleFile(entryFile));
            }
        }
    }
}
