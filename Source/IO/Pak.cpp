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

#include "IO/FileManager.h"
#include "IO/IOUtils.h"
#include "IO/substream.h"
#include "Utility/List.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        Pak::Pak(const String& path) :
        m_stream(path.c_str(), std::ios::in | std::ios::binary),
        m_path(path) {
            assert(m_stream.is_open());

            char magic[PakLayout::HeaderMagicLength];
            char entryName[PakLayout::EntryNameLength];
            
            m_stream.seekg(PakLayout::HeaderAddress, std::ios::beg);
            m_stream.read(magic, PakLayout::HeaderMagicLength);
            unsigned int directoryAddress = IO::readUnsignedInt<int32_t>(m_stream);
            unsigned int directorySize = IO::readUnsignedInt<int32_t>(m_stream);
            unsigned int entryCount = directorySize / PakLayout::EntryLength;
            
            m_stream.seekg(directoryAddress, std::ios::beg);
            for (unsigned int i = 0; i < entryCount; i++) {
                m_stream.read(entryName, PakLayout::EntryNameLength);
                unsigned int entryAddress = IO::readUnsignedInt<int32_t>(m_stream);
                unsigned int entryLength = IO::readUnsignedInt<int32_t>(m_stream);
                
                m_directory[Utility::toLower(entryName)] = PakEntry(entryName, entryAddress, entryLength);
            }
        }
        
        IStream Pak::entryStream(const String& name) {
            assert(m_stream.is_open());

            PakDirectory::iterator it = m_directory.find(Utility::toLower(name));
            if (it == m_directory.end())
                return IStream(NULL);
            
            const PakEntry& entry = it->second;
            
            m_stream.clear();
            substreambuf* subStreamBuf = new substreambuf(m_stream.rdbuf(),
                                                          static_cast<int>(entry.address()),
                                                          static_cast<int>(entry.length()));
            std::istream* subStream = new isubstream(subStreamBuf);
            return IStream(subStream);
        }

        PakManager* PakManager::sharedManager = NULL;
        
        bool PakManager::findPaks(const String& path, PakList& result) {
            String lowerPath = Utility::toLower(path);
            PakMap::iterator it = m_paks.find(lowerPath);
            
            if (it != m_paks.end()) {
                result = it->second;
                return true;
            }
            
            FileManager fileManager;
            const StringList pakNames = fileManager.directoryContents(path, "pak");
            if (!pakNames.empty()) {
                PakList newPaks;
                for (unsigned int i = 0; i < pakNames.size(); i++) {
                    String pakPath = fileManager.appendPath(path, pakNames[i]);
                    if (!fileManager.isDirectory(pakPath)) {
                        Pak* pak = new Pak(pakPath);
                        newPaks.push_back(pak);
                    }
                }

                std::sort(newPaks.begin(), newPaks.end(), ComparePaksByPath());
                m_paks[lowerPath] = newPaks;
                result = newPaks;
                return true;
            }
            
            return false;
        }

        PakManager::~PakManager() {
            PakMap::iterator it, end;
            for (it = m_paks.begin(), end = m_paks.end(); it != end; ++it) {
                PakList& paks = it->second;
                Utility::deleteAll(paks);
            }
            m_paks.clear();
        }
        
        IStream PakManager::entryStream(const String& name, const String& searchPath) {
            PakList paks;
            if (findPaks(searchPath, paks)) {
                PakList::reverse_iterator pak, endPak;
                for (pak = paks.rbegin(), endPak = paks.rend(); pak != endPak; ++pak) {
                    IStream stream = (*pak)->entryStream(name);
                    if (stream.get() != NULL)
                        return stream;
                }
            }
            
            return IStream(NULL);
        }
    }
}
