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

#include "Pak.h"
#include <cstdio>
#include <cstring>
#include <numeric>
#include <algorithm>
#include "substream.h"
#include "Utilities/Utils.h"
#include "Utilities/Console.h"

#ifdef _MSC_VER
#include "dirent.h"
#else
#include <dirent.h>
#endif

namespace TrenchBroom {
    namespace IO {
        Pak::Pak(string path) {
            char magic[PAK_HEADER_MAGIC_LENGTH];
            char entryName[PAK_ENTRY_NAME_LENGTH];
            int32_t directoryAddr, directorySize;
            int entryCount;

            this->path = path;
			m_stream.open(this->path.c_str(), ios::binary);
            if (m_stream.is_open()) {
                m_stream.seekg(PAK_HEADER_ADDRESS, ios::beg);
                m_stream.read((char *)magic, PAK_HEADER_MAGIC_LENGTH); // todo check and throw exception
                m_stream.read((char *)&directoryAddr, sizeof(int32_t));
                m_stream.read((char *)&directorySize, sizeof(int32_t));
                entryCount = directorySize / PAK_ENTRY_LENGTH;

                m_stream.seekg(directoryAddr, ios::beg);
                for (int i = 0; i < entryCount; i++) {
                    PakEntry entry;

                    m_stream.read(entryName, PAK_ENTRY_NAME_LENGTH);
                    entry.name = entryName;
                    m_stream.read((char *)&entry.address, sizeof(int32_t));
                    m_stream.read((char *)&entry.length, sizeof(int32_t));

                    entries[entry.name] = entry;
                }
            }
        }

        PakStream Pak::streamForEntry(string name) {
            PakEntry* entry;

            map<string, PakEntry>::iterator it = entries.find(name);
            if (it == entries.end())
                return auto_ptr<istream>(NULL);

            entry = &it->second;
            if (!m_stream.is_open())
                m_stream.open(path.c_str());

			m_stream.clear();
            substreambuf* subStreamBuf = new substreambuf(m_stream.rdbuf(), entry->address, entry->length);
            istream* subStream = new isubstream(subStreamBuf);
            return PakStream(subStream);
        }

        int comparePaks(const PakPtr pak1, const PakPtr pak2) {
			return pak1->path.compare(pak2->path) < 0;
        }
        
        PakManager* PakManager::sharedManager = NULL;
        
        PakStream PakManager::streamForEntry(string& name, const vector<string>& paths) {
            vector<string>::const_reverse_iterator path;
            for (path = paths.rbegin(); path < paths.rend(); ++path) {
                vector<PakPtr> paks;
                if (paksAtPath(*path, paks)) {
                    vector<PakPtr>::reverse_iterator pak;
                    for (pak = paks.rbegin(); pak < paks.rend(); ++pak) {
                        PakStream stream = (*pak)->streamForEntry(name);
                        if (stream.get() != NULL)
                            return stream;
                    }
                }
            }

            string nicePaths = accumulate(paths.begin(), paths.end(), string(", "));
            log(TB_LL_WARN, "Could not find pak entry %s at pak paths %s\n", name.c_str(), nicePaths.c_str());
            return PakStream(NULL);
        }

        bool PakManager::paksAtPath(const string& path, vector<PakPtr>& result) {
            map<string, vector<PakPtr> >::iterator it = paks.find(path);
            if (it != paks.end()) {
                result = it->second;
                return true;
            }

            DIR* dir = opendir(path.c_str());
            if (!dir) {
                log(TB_LL_WARN, "Could not open pak path %s\n", path.c_str());
                return false;
            }

            struct dirent* entry = readdir(dir);
            if (!entry) {
                log(TB_LL_WARN, "%s does not contain any pak files\n", path.c_str());
                closedir(dir);
                return false;
            }

            vector<PakPtr> newPaks;
            do {
                if (strncmp(entry->d_name + entry->d_namlen - 4, ".pak", 4) == 0) {
					string pakPath = appendPath(path, entry->d_name);
                    Pak* pak = new Pak(pakPath);
                    PakPtr pakPtr(pak);
                    newPaks.push_back(pakPtr);
                }
                entry = readdir(dir);
            } while (entry);
            closedir(dir);

            sort(newPaks.begin(), newPaks.end(), comparePaks);
            paks[path] = newPaks;

            result = paks[path];
            return true;
        }
    }
}
