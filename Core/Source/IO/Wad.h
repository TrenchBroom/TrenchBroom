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
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace IO {
		static const int WAD_NUM_ENTRIES_ADDRESS	= 4;
		static const int WAD_DIR_OFFSET_ADDRESS		= 8;
		static const int WAD_DIR_ENTRY_TYPE_OFFSET	= 4;
		static const int WAD_DIR_ENTRY_NAME_OFFSET	= 3;
		static const int WAD_DIR_ENTRY_NAME_LENGTH	= 16;
		static const int WAD_PAL_LENGTH				= 256;
		static const int WAD_TEX_WIDTH_OFFSET		= 16;

		static const char WT_STATUS		= 'B';
		static const char WT_CONSOLE	= 'C';
		static const char WT_MIP		= 'D';
		static const char WT_PALETTE	= '@';


        class WadEntry {
        public:
            int32_t address;
            int32_t length;
            int32_t size;
            char type;
            std::string name;
        };

        class Mip {
        public:
            std::string name;
            int width;
            int height;
            unsigned char* mip0;
            unsigned char* mip1;
            unsigned char* mip2;
            unsigned char* mip3;

            Mip(const std::string& name, int width, int height);
            ~Mip();
        };

        class Wad {
            std::ifstream m_stream;
        public:
            std::vector<WadEntry> entries;
            Wad(const std::string& path);
            ~Wad();
            Mip* loadMipAtEntry(const WadEntry& entry);
        };
    }
}
#endif
