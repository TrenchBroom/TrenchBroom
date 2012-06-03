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
#include "IO/FileManager.h"
#include "Utilities/Utils.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace IO {
        Pak::Pak(const std::string& path) {
            char magic[PAK_HEADER_MAGIC_LENGTH];
            char entryName[PAK_ENTRY_NAME_LENGTH];
            int32_t directoryAddr, directorySize;
            int entryCount;

            this->path = path;
			m_stream.open(this->path.c_str(), std::ios::binary);
            if (m_stream.is_open()) {
                m_stream.seekg(PAK_HEADER_ADDRESS, std::ios::beg);
                m_stream.read((char *)magic, PAK_HEADER_MAGIC_LENGTH); // todo check and throw exception
                m_stream.read((char *)&directoryAddr, sizeof(int32_t));
                m_stream.read((char *)&directorySize, sizeof(int32_t));
                entryCount = directorySize / PAK_ENTRY_LENGTH;

                m_stream.seekg(directoryAddr, std::ios::beg);
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

        PakStream Pak::streamForEntry(const std::string& name) {
            PakEntry* entry;

            std::map<std::string, PakEntry>::iterator it = entries.find(name);
            if (it == entries.end())
                return std::auto_ptr<std::istream>(NULL);

            entry = &it->second;
            if (!m_stream.is_open())
                m_stream.open(path.c_str());

			m_stream.clear();
            substreambuf* subStreamBuf = new substreambuf(m_stream.rdbuf(), entry->address, entry->length);
            std::istream* subStream = new isubstream(subStreamBuf);
            return PakStream(subStream);
        }

        int comparePaks(const PakPtr pak1, const PakPtr pak2) {
			return pak1->path.compare(pak2->path) < 0;
        }

        PakManager* PakManager::sharedManager = NULL;

        PakStream PakManager::streamForEntry(const std::string& name, const std::vector<std::string>& paths) {
            std::vector<std::string>::const_reverse_iterator path;
            for (path = paths.rbegin(); path < paths.rend(); ++path) {
                std::vector<PakPtr> paks;
                if (paksAtPath(*path, paks)) {
                    std::vector<PakPtr>::reverse_iterator pak;
                    for (pak = paks.rbegin(); pak < paks.rend(); ++pak) {
                        PakStream stream = (*pak)->streamForEntry(name);
                        if (stream.get() != NULL)
                            return stream;
                    }
                }
            }

            std::string nicePaths = accumulate(paths.begin(), paths.end(), std::string(", "));
            log(TB_LL_WARN, "Could not find pak entry %s at pak paths %s\n", name.c_str(), nicePaths.c_str());
            return PakStream(NULL);
        }

        bool PakManager::paksAtPath(const std::string& path, std::vector<PakPtr>& result) {
            std::map<std::string, std::vector<PakPtr> >::iterator it = paks.find(path);
            if (it != paks.end()) {
                result = it->second;
                return true;
            }

            FileManager& fileManager = *FileManager::sharedFileManager;
            std::vector<std::string> pakNames = fileManager.directoryContents(path, "pak");
            if (!pakNames.empty()) {
                std::vector<PakPtr> newPaks;
                for (unsigned int i = 0; i < pakNames.size(); i++) {
                    std::string pakPath = fileManager.appendPath(path, pakNames[i]);
                    if (!fileManager.isDirectory(pakPath)) {
                        Pak* pak = new Pak(pakPath);
                        PakPtr pakPtr(pak);
                        newPaks.push_back(pakPtr);
                    }
                }

                sort(newPaks.begin(), newPaks.end(), comparePaks);
                paks[path] = newPaks;
                
                result = paks[path];
                return true;
            } else {
                log(TB_LL_WARN, "Could not open pak path %s\n", path.c_str());
                return false;
            }
        }
    }
}
