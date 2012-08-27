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

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        Mip* Wad::loadMip(const WadEntry& entry) const {
			assert(m_stream.is_open());
			assert(!m_stream.eof());
            if (entry.type() != WadEntryType::Mip)
                return NULL;
            
            int32_t width = 0;
			int32_t height = 0;
			int32_t mip0Offset = 0;
			int mip0Size = 0;

            m_stream.seekg(entry.address(), std::ios::beg);
            m_stream.seekg(WadLayout::TexWidthOffset, std::ios::cur);
            m_stream.read(reinterpret_cast<char *>(&width), sizeof(int32_t));
            m_stream.read(reinterpret_cast<char *>(&height), sizeof(int32_t));
            m_stream.read(reinterpret_cast<char *>(&mip0Offset), sizeof(int32_t));
            mip0Size = width * height;
            
            unsigned char* mip0 = new unsigned char[mip0Size];
            
            m_stream.seekg(entry.address() + mip0Offset, std::ios::beg);
            m_stream.read((char *)mip0, mip0Size);
            
            return new Mip(entry.name(), width, height, mip0);
        }

        Wad::Wad(const String& path) : m_stream(path.c_str(), std::ios::binary | std::ios::in) {
			m_stream.exceptions(std::ios::failbit | std::ios::badbit);
            
            int32_t entryCount, directoryAddr;
            int32_t entryAddress, entryLength;
            char entryType;
            char entryName[WadLayout::DirEntryNameLength];
            if (m_stream.is_open()) {
                m_stream.seekg(WadLayout::NumEntriesAddress, std::ios::beg);
                m_stream.read(reinterpret_cast<char *>(&entryCount), sizeof(int32_t));
                
                m_stream.seekg(WadLayout::DirOffsetAddress, std::ios::beg);
                m_stream.read(reinterpret_cast<char *>(&directoryAddr), sizeof(int32_t));
                m_stream.seekg(directoryAddr, std::ios::beg);
                
                for (int i = 0; i < entryCount; i++) {
					assert(!m_stream.eof());
                    
					m_stream.read(reinterpret_cast<char *>(&entryAddress), sizeof(int32_t));
                    m_stream.read(reinterpret_cast<char *>(&entryLength), sizeof(int32_t));
                    m_stream.seekg(WadLayout::DirEntryTypeOffset, std::ios::cur);
                    m_stream.read(&entryType, 1);
                    m_stream.seekg(WadLayout::DirEntryNameOffset, std::ios::cur);
                    m_stream.read((char *)entryName, WadLayout::DirEntryNameLength);
                    
                    m_entries.push_back(WadEntry(entryAddress, entryLength, entryType, entryName));
                }
            }
            
			m_stream.clear();
        }
        
        Mip::List Wad::loadMips() const {
            Mip::List mips;
            for (unsigned int i = 0; i < m_entries.size(); i++) {
                const WadEntry& entry = m_entries[i];
                if (entry.type() == WadEntryType::Mip) {
                    Mip* mip = loadMip(entry);
                    if (mip != NULL)
                        mips.push_back(mip);
                }
            }
            
            return mips;
        }
    }
}