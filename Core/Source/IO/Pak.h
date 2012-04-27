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

#ifdef _MSC_VER
#include <cstdint>
#endif

#define PAK_HEADER_ADDRESS 0x0
#define PAK_HEADER_MAGIC_LENGTH 0x4
#define PAK_HEADER_MAGIC_VALUE "PACK";
#define PAK_ENTRY_LENGTH 0x40
#define PAK_ENTRY_NAME_LENGTH 0x38

using namespace std;

namespace TrenchBroom {
    namespace IO {
        class Pak;
        static int comparePaks(const Pak* pak1, const Pak* pak2);

        typedef auto_ptr<istream> PakStream;
        
        class PakEntry {
        public:
            string name;
            int32_t address;
            int32_t length;
        };
        
        class Pak {
            ifstream mStream;
        public:
            string path;
            map<string, PakEntry> entries;
            Pak(string path);
            Pak(const Pak&);
            PakStream streamForEntry(string name);
        };
        
        class PakManager {
        private:
            map<string, vector<Pak*> > paks;
            vector<Pak*>* paksAtPath(string path);
        public:
            static PakManager* sharedManager;
            PakManager();
            ~PakManager();
           PakStream streamForEntry(string& name, vector<string>& paths);
        };
    }
}

#endif
