/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Wad.h"

#include "IO/IOUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        namespace WadLayout {
            static const unsigned int NumEntriesAddress     = 4;
            static const unsigned int DirOffsetAddress      = 8;
            static const unsigned int DirEntryTypeOffset    = 4;
            static const unsigned int DirEntryNameOffset    = 3;
            static const unsigned int DirEntryNameLength    = 16;
            static const unsigned int PalLength             = 256;
            static const unsigned int TexWidthOffset        = 16;
        }

        Mip* Wad::loadMip(const WadEntry& entry, unsigned int mipCount) const throw (IOException) {
            if (entry.type() != WadEntryType::WEMip)
                throw IOException("Entry %s is not a mip", entry.name().c_str());
            
            char* cursor = m_file->begin() + entry.address() + WadLayout::TexWidthOffset;
            unsigned int width = readUnsignedInt<int32_t>(cursor);
            unsigned int height = readUnsignedInt<int32_t>(cursor);
            unsigned int mip0Size = width * height;
            unsigned int mip0Offset = readUnsignedInt<int32_t>(cursor);
            
            
            if (width == 0 || height == 0)
                throw IOException("Invalid mip dimensions (%ix%i)", width, height);
            if (mip0Offset + mip0Size > entry.length())
                throw IOException("Mip data beyond wad entry");
            
            unsigned char* mip0 = NULL;
            if (mipCount > 0) {
                mip0 = new unsigned char[mip0Size];
                cursor = m_file->begin() + entry.address() + mip0Offset;
                readBytes(cursor, mip0, mip0Size);
            }
            
            return new Mip(entry.name(), static_cast<unsigned int>(width), static_cast<unsigned int>(height), mip0);
        }

        Wad::Wad(const String& path) throw (IOException) {
            FileManager fileManager;
            m_file = fileManager.mapFile(path);
            
            if (m_file.get() == NULL)
                throw IOException::openError();
            
            if (WadLayout::NumEntriesAddress + sizeof(int32_t) >= m_file->size() ||
                WadLayout::DirOffsetAddress + sizeof(int32_t) >= m_file->size())
                throw IOException("Invalid wad layout");
            
            char* cursor = m_file->begin() + WadLayout::NumEntriesAddress;
            unsigned int entryCount = readUnsignedInt<int32_t>(cursor);
            cursor = m_file->begin() + WadLayout::DirOffsetAddress;
            unsigned int directoryAddr = readUnsignedInt<int32_t>(cursor);

            if (directoryAddr  >= m_file->size())
                throw IOException("Wad directory beyond end of file");

            char entryType;
            char entryName[WadLayout::DirEntryNameLength];
            
            cursor = m_file->begin() + directoryAddr;
            
            for (unsigned int i = 0; i < entryCount; i++) {
                unsigned int entryAddress = readUnsignedInt<int32_t>(cursor);
                unsigned int entryLength = readUnsignedInt<int32_t>(cursor);
                
                if (entryAddress + entryLength >= m_file->size())
                    throw IOException("Wad entry beyond end of file");
                
                cursor += WadLayout::DirEntryTypeOffset;
                readBytes(cursor, &entryType, 1);
                cursor += WadLayout::DirEntryNameOffset;
                readBytes(cursor, entryName, WadLayout::DirEntryNameLength);
                
                // might leak if there are duplicate entries
                m_entries[entryName] = WadEntry(entryAddress, entryLength, entryType, entryName);
            }
        }
        
        Mip* Wad::loadMip(const String& name, unsigned int mipCount) const throw (IOException) {
            EntryMap::const_iterator it = m_entries.find(name);
            if (it == m_entries.end())
                throw IOException("Wad entry %s not found", name.c_str());
            return loadMip(it->second, mipCount);
        }

        Mip::List Wad::loadMips(unsigned int mipCount) const throw (IOException) {
            Mip::List mips;
            EntryMap::const_iterator it, end;
            for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                const WadEntry& entry = it->second;
                if (entry.type() == WadEntryType::WEMip) {
                    Mip* mip = loadMip(entry, mipCount);
                    if (mip != NULL)
                        mips.push_back(mip);
                }
            }
            
            return mips;
        }
    }
}
