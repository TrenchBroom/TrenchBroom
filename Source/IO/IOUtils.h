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

#ifndef TrenchBroom_IOUtils_h
#define TrenchBroom_IOUtils_h

#include "IO/FileManager.h"
#include "IO/IOTypes.h"
#include "IO/Pak.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>

#ifdef _MSC_VER
#include <cstdint>
#endif

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace IO {
        inline MappedFile::Ptr findGameFile(const String& filePath, const StringList& searchPaths) {
            MappedFile::Ptr mappedFile;
            FileManager fileManager;

            StringList::const_reverse_iterator pathIt, pathEnd;
            for (pathIt = searchPaths.rbegin(), pathEnd = searchPaths.rend(); pathIt != pathEnd; ++pathIt) {
                const String& searchPath = *pathIt;
                const String path = fileManager.appendPath(searchPath, filePath);
                MappedFile::Ptr file;
                if (fileManager.exists(path) && !fileManager.isDirectory(path))
                    file = fileManager.mapFile(path);
                else
                    file = PakManager::sharedManager->entry(filePath, searchPath);
                if (file.get() != NULL)
                    return file;
            }

            return mappedFile;
        }

        template <typename T>
        inline T read(char*& cursor) {
            T value;
            memcpy(&value, cursor, sizeof(T));
            cursor += sizeof(T);
            return value;
        }

        template <typename T>
        inline int readInt(char*& cursor) {
            return static_cast<int>(read<T>(cursor));
        }

        template <typename T>
        inline unsigned int readUnsignedInt(char*& cursor) {
            return static_cast<unsigned int>(read<T>(cursor));
        }

        template <typename T>
        inline size_t readSize(char*& cursor) {
            return static_cast<size_t>(read<T>(cursor));
        }

        template <typename T>
        inline bool readBool(char*& cursor) {
            return read<T>(cursor) != 0;
        }

        template <typename T>
        inline float readFloat(char*& cursor) {
            return static_cast<float>(read<T>(cursor));
        }

        inline Vec3f readVec3f(char*& cursor) {
            Vec3f value;
            for (size_t i = 0; i < 3; i++)
                value[i] = readFloat<float>(cursor);
            return value;
        }

        inline void readBytes(char*& cursor, char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
            cursor += n;
        }

        inline void readBytes(char*& cursor, unsigned char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
            cursor += n;
        }
    }
}

#endif
