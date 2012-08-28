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

#ifndef __TrenchBroom__Wad__
#define __TrenchBroom__Wad__

#include "Utility/String.h"

#include "IO/mmapped_fstream.h"
#include <vector>

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
        
        namespace WadEntryType {
            static const char Status    = 'B';
            static const char Console   = 'C';
            static const char Mip       = 'D';
            static const char Palette   = '@';
        }
        
        class WadEntry {
        public:
            typedef std::vector<WadEntry> List;
        private:
            int32_t m_address;
            int32_t m_length;
            char m_type;
            String m_name;
        public:
            WadEntry(int32_t address, int32_t length, char type, const String& name) : m_address(address), m_length(length), m_type(type), m_name(name) {}
            
            inline int32_t address() const {
                return m_address;
            }
            
            inline int32_t length() const {
                return m_length;
            }
    
            inline char type() const {
                return m_type;
            }
    
            inline const String& name() const {
                return m_name;
            }
        };
        
        class Mip {
        public:
            typedef std::vector<Mip*> List;
        private:
            String m_name;
            unsigned int m_width;
            unsigned int m_height;
            unsigned char* m_mip0;
        public:
            Mip(const String& name, unsigned int width, unsigned int height, unsigned char* mip0) : m_name(name), m_width(width), m_height(height), m_mip0(mip0) {}
            ~Mip() {
                delete [] m_mip0;
            }
            
            inline const String& name() const {
                return m_name;
            }
            
            inline unsigned int width() const {
                return m_width;
            }
            
            inline unsigned int height() const {
                return m_height;
            }
            
            inline const unsigned char const* mip0() const {
                return m_mip0;
            }
        };

        class Wad {
        private:
            mutable mmapped_fstream m_stream;
            WadEntry::List m_entries;

            Mip* loadMip(const WadEntry& entry) const;
        public:
            Wad(const String& path);
            
            Mip::List loadMips() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Wad__) */
