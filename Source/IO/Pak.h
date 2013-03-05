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

#ifndef __TrenchBroom__Pak__
#define __TrenchBroom__Pak__

#include "IO/IOTypes.h"
#include "IO/mmapped_fstream.h"
#include "Utility/String.h"

#include <map>
#include <vector>

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace IO {
        namespace PakLayout {
            static const unsigned int HeaderAddress     = 0x0;
            static const unsigned int HeaderMagicLength = 0x4;
            static const unsigned int EntryLength       = 0x40;
            static const unsigned int EntryNameLength   = 0x38;
            static const String HeaderMagic             = "PACK";
        }

        class PakEntry {
            String m_name;
            unsigned int m_address;
            unsigned int m_length;
        public:
            PakEntry() {}

            PakEntry(const String& name, unsigned int address, unsigned int length) :
            m_name(name),
            m_address(address),
            m_length(length) {}

            inline const String& name() const {
                return m_name;
            }

            inline unsigned int address() const {
                return m_address;
            }

            inline unsigned int length() const {
                return m_length;
            }
        };

        class Pak {
        private:
            typedef std::map<String, PakEntry> PakDirectory;

            mmapped_fstream m_stream;
            String m_path;
            PakDirectory m_directory;
        public:
            Pak(const String& path);

            inline const String& path() const {
                return m_path;
            }

            IStream entryStream(const String& name);
        };

        class ComparePaksByPath {
        public:
            inline bool operator() (const Pak* left, const Pak* right) const {
                return left->path() < right->path();
            }
        };

        class PakManager {
        private:
            typedef std::vector<Pak*> PakList;
            typedef std::map<String, PakList> PakMap;

            PakMap m_paks;
            bool findPaks(const String& path, PakList& result);
        public:
            static PakManager* sharedManager;
            ~PakManager();
            
            IStream entryStream(const String& name, const String& searchPath);
        };
    }
}

#endif /* defined(__TrenchBroom__Pak__) */
