/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "VecMath.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>

#ifdef _Win32
#include <cstdint>
#endif

namespace TrenchBroom {
    namespace IO {
        template <typename T>
        inline T read(const char*& cursor) {
            T value;
            memcpy(&value, cursor, sizeof(T));
            cursor += sizeof(T);
            return value;
        }
        
        template <typename T>
        inline int readInt(const char*& cursor) {
            return static_cast<int>(read<T>(cursor));
        }
        
        template <typename T>
        inline unsigned int readUnsignedInt(const char*& cursor) {
            return static_cast<unsigned int>(read<T>(cursor));
        }
        
        template <typename T>
        inline size_t readSize(const char*& cursor) {
            return static_cast<size_t>(read<T>(cursor));
        }
        
        template <typename T>
        inline bool readBool(const char*& cursor) {
            return read<T>(cursor) != 0;
        }
        
        template <typename T>
        inline float readFloat(const char*& cursor) {
            return static_cast<float>(read<T>(cursor));
        }
        
        inline Vec3f readVec3f(const char*& cursor) {
            Vec3f value;
            for (size_t i = 0; i < 3; i++)
                value[i] = readFloat<float>(cursor);
            return value;
        }
        
        inline void readBytes(const char*& cursor, char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
            cursor += n;
        }
        
        inline void readBytes(const char*& cursor, unsigned char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
            cursor += n;
        }
    }
}

#endif
