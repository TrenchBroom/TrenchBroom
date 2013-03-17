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
#include "Utility/List.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        Pak::Pak(const String& path, MappedFile::Ptr file) :
        m_path(path),
        m_file(file) {
            char magic[PakLayout::HeaderMagicLength];
            char entryName[PakLayout::EntryNameLength];

            char* cursor = m_file->begin() + PakLayout::HeaderAddress;
            readBytes(cursor, magic, PakLayout::HeaderMagicLength);
            
            unsigned int directoryAddress = readUnsignedInt<int32_t>(cursor);
            unsigned int directorySize = readUnsignedInt<int32_t>(cursor);
            unsigned int entryCount = directorySize / PakLayout::EntryLength;

            assert(m_file->begin() + directoryAddress + directorySize <= m_file->end());
            cursor = m_file->begin() + directoryAddress;
            
            for (unsigned int i = 0; i < entryCount; i++) {
                readBytes(cursor, entryName, PakLayout::EntryNameLength);
                int entryAddress = readInt<int32_t>(cursor);
                int entryLength = readInt<int32_t>(cursor);
                assert(m_file->begin() + entryAddress + entryLength <= m_file->end());

                char* entryBegin = m_file->begin() + entryAddress;
                char* entryEnd = entryBegin + entryLength;
                m_directory[Utility::toLower(entryName)] = PakEntry(entryName, entryBegin, entryEnd);
            }
        }
        
        MappedFile::Ptr Pak::entry(const String& name) {
            PakDirectory::iterator it = m_directory.find(Utility::toLower(name));
            if (it == m_directory.end())
                return MappedFile::Ptr();
            
            const PakEntry& entry = it->second;
            return entry.data();
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
                        MappedFile::Ptr file = fileManager.mapFile(pakPath);
                        assert(file.get() != NULL);
                        newPaks.push_back(Pak(pakPath, file));
                    }
                }

                std::sort(newPaks.begin(), newPaks.end(), ComparePaksByPath());
                m_paks[lowerPath] = newPaks;
                result = newPaks;
                return true;
            }
            
            return false;
        }

        MappedFile::Ptr PakManager::entry(const String& name, const String& searchPath) {
            PakList paks;
            if (findPaks(searchPath, paks)) {
                PakList::reverse_iterator pak, endPak;
                for (pak = paks.rbegin(), endPak = paks.rend(); pak != endPak; ++pak) {
                    MappedFile::Ptr data = pak->entry(name);
                    if (data.get() != NULL)
                        return data;
                }
            }
            
            return MappedFile::Ptr();
        }
    }
}
