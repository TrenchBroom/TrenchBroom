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

#ifndef TrenchBroom_Wad_h
#define TrenchBroom_Wad_h

#include <fstream>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <cstdint>
#endif

#define WAD_NUM_ENTRIES_ADDRESS 4
#define WAD_DIR_OFFSET_ADDRESS 8
#define WAD_DIR_ENTRY_TYPE_OFFSET 4
#define WAD_DIR_ENTRY_NAME_OFFSET 3
#define WAD_DIR_ENTRY_NAME_LENGTH 16
#define WAD_PAL_LENGTH 256
#define WAD_TEX_WIDTH_OFFSET 16

#define WT_STATUS 'B'
#define WT_CONSOLE 'C'
#define WT_MIP 'D'
#define WT_PALETTE '@'

using namespace std;

namespace TrenchBroom {
    namespace IO {
        class WadEntry {
        public:
            int32_t address;
            int32_t length;
            int32_t size;
            char type;
            string name;
        };
        
        class Mip {
        public:
            string name;
            int width;
            int height;
            unsigned char* mip0;
            unsigned char* mip1;
            unsigned char* mip2;
            unsigned char* mip3;
            
            Mip(string name, int width, int height);
            ~Mip();
        };
        
        class Wad {
            ifstream mStream;
        public:
            vector<WadEntry> entries;
            Wad(string path);
            ~Wad();
            Mip* loadMipAtEntry(WadEntry& entry);
        };
    }
}
#endif
