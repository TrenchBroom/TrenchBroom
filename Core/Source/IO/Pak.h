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

#ifndef TrenchBroom_Pak_h
#define TrenchBroom_Pak_h

#include <istream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "Utilities/SharedPointer.h"

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

#define PAK_HEADER_ADDRESS 0x0
#define PAK_HEADER_MAGIC_LENGTH 0x4
#define PAK_HEADER_MAGIC_VALUE "PACK";
#define PAK_ENTRY_LENGTH 0x40
#define PAK_ENTRY_NAME_LENGTH 0x38

namespace TrenchBroom {
    namespace IO {
        class Pak;

        typedef std::auto_ptr<std::istream> PakStream;

        class PakEntry {
        public:
            std::string name;
            int32_t address;
            int32_t length;
        };

        class Pak {
            std::ifstream m_stream;
        public:
            std::string path;
            std::map<std::string, PakEntry> entries;
            Pak(const std::string& path);
            PakStream streamForEntry(const std::string& name);
        };

        typedef std::tr1::shared_ptr<Pak> PakPtr;
        static int comparePaks(const PakPtr pak1, const PakPtr pak2);

        class PakManager {
        private:
            std::map<std::string, std::vector<PakPtr> > paks;
            bool paksAtPath(const std::string& path, std::vector<PakPtr>& result);
        public:
            static PakManager* sharedManager;
            PakManager() {};
            PakStream streamForEntry(const std::string& name, const std::vector<std::string>& paths);
        };
    }
}

#endif
