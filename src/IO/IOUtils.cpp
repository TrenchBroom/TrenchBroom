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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "IOUtils.h"

namespace TrenchBroom {
    namespace IO {
        Vec3f readVec3f(const char*& cursor) {
            Vec3f value;
            for (size_t i = 0; i < 3; i++)
                value[i] = readFloat<float>(cursor);
            return value;
        }
        
        Vec3f readVec3f(const char* const& cursor) {
            Vec3f value;
            for (size_t i = 0; i < 3; i++)
                value[i] = readFloat<float>(cursor);
            return value;
        }
        
        void readBytes(const char*& cursor, char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
            cursor += n;
        }
        
        void readBytes(const char* const& cursor, char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
        }
        
        void readBytes(const char*& cursor, unsigned char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
            cursor += n;
        }
        
        void readBytes(const char* const& cursor, unsigned char* buffer, size_t n) {
            memcpy(buffer, cursor, n);
        }
    }
}
