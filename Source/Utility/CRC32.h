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

#ifndef __TrenchBroom__CRC32__
#define __TrenchBroom__CRC32__

#include <iostream>

#if defined _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace Utility {
        uint32_t updC32(uint32_t octet, uint32_t crc);
        
        inline uint32_t crc32(const char* buf, size_t len) {
            register uint32_t old = 0xFFFFFFFF;
            for (; len; --len, ++buf)
                old = updC32(static_cast<uint32_t>(*buf), old);
            
            return ~old;
        }
        
        template <typename T>
        inline uint32_t updateCRC32(T s, uint32_t crc) {
            const char* buf = reinterpret_cast<const char*>(&s);
            size_t len = sizeof(T);
            
            for (; len; --len, ++buf)
                crc = updC32(static_cast<uint32_t>(*buf), crc);
            return crc;
        }
    }
}

#endif /* defined(__TrenchBroom__CRC32__) */
