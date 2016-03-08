/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Wad
#define TrenchBroom_Wad

#include "StringUtils.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        namespace WadEntryType {
            static const char WEStatus    = 'B';
            static const char WEConsole   = 'C';
            static const char WEMip       = 'D';
            static const char WEPalette   = '@';
        }
        
        class WadEntry {
        private:
            String m_name;
            char m_type;
            size_t m_address;
            size_t m_size;
        public:
            WadEntry(const String& name, const char type, const size_t address, const size_t size);
            
            const String& name() const;
            char type() const;
            size_t address() const;
            size_t size() const;
        };
        
        typedef std::vector<WadEntry> WadEntryList;
        
        struct MipSize {
            size_t width;
            size_t height;
            
            MipSize(const size_t i_width, const size_t i_height);
            bool operator==(const MipSize& rhs) const;
        };
        
        struct MipData {
            const char* begin;
            const char* end;
            
            MipData(const char* i_begin, const char* i_end);
        };
        
        class Wad {
        private:
            MappedFile::Ptr m_file;
            WadEntryList m_entries;
        public:
            Wad(const Path& path);
            
            const WadEntryList& allEntries() const;
            const WadEntryList entriesWithType(const char type) const;
            const MipSize mipSize(const WadEntry& entry) const;
            const MipData mipData(const WadEntry& entry, const size_t mipLevel) const;
        private:
            void loadEntries();
        };
    }
}

#endif /* defined(TrenchBroom_Wad) */
