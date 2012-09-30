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

#include <map>
#include <vector>

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace IO {
        namespace WadEntryType {
            static const char WEStatus    = 'B';
            static const char WEConsole   = 'C';
            static const char WEMip       = 'D';
            static const char WEPalette   = '@';
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
            WadEntry(int32_t address, int32_t length, char type, const String& name) :
            m_address(address),
            m_length(length),
            m_type(type),
            m_name(name) {}
            
            WadEntry() :
            m_address(0),
            m_length(0),
            m_type(WadEntryType::WEStatus),
            m_name("") {}
            
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
            
            inline const unsigned char* const mip0() const {
                return m_mip0;
            }
        };

        class Wad {
        private:
            typedef std::map<String, WadEntry> EntryMap;
            
            mutable mmapped_fstream m_stream;
            EntryMap m_entries;

            Mip* loadMip(const WadEntry& entry, unsigned int mipCount) const;
        public:
            Wad(const String& path);
            
            Mip* loadMip(const String& name, unsigned int mipCount) const;
            Mip::List loadMips(unsigned int mipCount) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Wad__) */
