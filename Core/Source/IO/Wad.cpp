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
        Mip::Mip(const string& name, int width, int height) {
            int size;
            
            this->name = name;
            this->width = width;
            this->height = height;
            
            size = width * height;
            mip0 = new unsigned char[size];
            mip1 = new unsigned char[size / 4];
            mip2 = new unsigned char[size / 16];
            mip3 = new unsigned char[size / 64];
        }
        
        Mip::~Mip() {
            delete[] mip0;
            delete[] mip1;
            delete[] mip2;
            delete[] mip3;
        }
        
        
        Wad::Wad(const string& path) {
            int32_t entryCount;
            int32_t directoryAddr;
            char entryName[WAD_DIR_ENTRY_NAME_LENGTH];
            WadEntry entry;
            
            m_stream.open(path.c_str(), ios::binary);
			m_stream.exceptions(ios::failbit | ios::badbit);
            if (m_stream.is_open()) {
                m_stream.seekg(WAD_NUM_ENTRIES_ADDRESS, ios::beg);
                m_stream.read((char *)&entryCount, sizeof(int32_t));
                
                m_stream.seekg(WAD_DIR_OFFSET_ADDRESS, ios::beg);
                m_stream.read((char *)&directoryAddr, sizeof(int32_t));
                m_stream.seekg(directoryAddr, ios::beg);
                
                for (int i = 0; i < entryCount; i++) {
					assert(!m_stream.eof());

					m_stream.read((char *)&entry.address, sizeof(int32_t));
                    m_stream.read((char *)&entry.length, sizeof(int32_t));
                    m_stream.seekg(WAD_DIR_ENTRY_TYPE_OFFSET, ios::cur);
                    m_stream.read((char *)&entry.type, 1);
                    m_stream.seekg(WAD_DIR_ENTRY_NAME_OFFSET, ios::cur);
                    m_stream.read((char *)entryName, WAD_DIR_ENTRY_NAME_LENGTH);

                    entry.name = entryName;
                    entries.push_back(entry);
                }
            }

			m_stream.clear();
        }
        
        Wad::~Wad() {
            m_stream.close();
        }
        
        Mip* Wad::loadMipAtEntry(const WadEntry& entry) {
			int32_t width = 0;
			int32_t height = 0; 
			int32_t mip0Offset = 0;
			int32_t mip1Offset = 0;
			int32_t mip2Offset = 0;
			int32_t mip3Offset = 0;
			int mip0Size = 0;
			int mip1Size = 0;
			int mip2Size = 0;
			int mip3Size = 0;
            Mip* mip = NULL;
            
			assert(m_stream.is_open());
			assert(!m_stream.eof());
            if (entry.type == WT_MIP) {
                m_stream.seekg(entry.address, ios::beg);
                m_stream.seekg(WAD_TEX_WIDTH_OFFSET, ios::cur);
                m_stream.read((char *)&width, sizeof(int32_t));
                m_stream.read((char *)&height, sizeof(int32_t));
                m_stream.read((char *)&mip0Offset, sizeof(int32_t));
                m_stream.read((char *)&mip1Offset, sizeof(int32_t));
                m_stream.read((char *)&mip2Offset, sizeof(int32_t));
                m_stream.read((char *)&mip3Offset, sizeof(int32_t));
                
                mip = new Mip(entry.name, width, height);
                
                mip0Size = width * height;
                mip1Size = mip0Size / 4;
                mip2Size = mip1Size / 4;
                mip3Size = mip2Size / 4;
                
                m_stream.seekg(entry.address + mip0Offset, ios::beg);
                m_stream.read((char *)mip->mip0, mip0Size);
                m_stream.seekg(entry.address + mip1Offset, ios::beg);
                m_stream.read((char *)mip->mip1, mip1Size);
                m_stream.seekg(entry.address + mip2Offset, ios::beg);
                m_stream.read((char *)mip->mip2, mip2Size);
                m_stream.seekg(entry.address + mip3Offset, ios::beg);
                m_stream.read((char *)mip->mip3, mip3Size);
            }
            
            return mip;
        }
    }
}