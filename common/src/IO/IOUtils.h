/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_IOUtils_h
#define TrenchBroom_IOUtils_h

#include "VecMath.h"
#include "Exceptions.h"
#include "ByteBuffer.h"
#include "StringUtils.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class OpenFile {
        private:
            FILE* m_file;
        public:
            OpenFile(const Path& path, bool write);
            ~OpenFile();
            
            FILE* file() const;
        };
        
        String readGameComment(FILE* stream);
        String readFormatComment(FILE* stream);
        String readInfoComment(FILE* stream, const String& name);
        
        void writeGameComment(FILE* stream, const String& gameName, const String& mapFormat);
        
        template <typename T>
        void advance(const char*& cursor, const size_t i = 1) {
            cursor += (i * sizeof(T));
        }
        
        template <typename T>
        T read(const char*& cursor) {
            T value;
            memcpy(&value, cursor, sizeof(T));
            cursor += sizeof(T);
            return value;
        }
        
        template <typename T>
        T read(const char* const& cursor) {
            T value;
            memcpy(&value, cursor, sizeof(T));
            return value;
        }
        
        template <typename T>
        int readInt(const char*& cursor) {
            return static_cast<int>(read<T>(cursor));
        }
        
        template <typename T>
        int readInt(const char* const& cursor) {
            return static_cast<int>(read<T>(cursor));
        }
        
        template <typename T>
        unsigned int readUnsignedInt(const char*& cursor) {
            return static_cast<unsigned int>(read<T>(cursor));
        }
        
        template <typename T>
        unsigned int readUnsignedInt(const char* const& cursor) {
            return static_cast<unsigned int>(read<T>(cursor));
        }
        
        template <typename T>
        size_t readSize(const char*& cursor) {
            return static_cast<size_t>(read<T>(cursor));
        }
        
        template <typename T>
        size_t readSize(const char* const& cursor) {
            return static_cast<size_t>(read<T>(cursor));
        }

        template <typename T>
        bool readBool(const char*& cursor) {
            return read<T>(cursor) != 0;
        }
        
        template <typename T>
        bool readBool(const char* const& cursor) {
            return read<T>(cursor) != 0;
        }
        
        template <typename T>
        float readFloat(const char*& cursor) {
            return static_cast<float>(read<T>(cursor));
        }
        
        template <typename T>
        float readFloat(const char* const& cursor) {
            return static_cast<float>(read<T>(cursor));
        }
        
        Vec3f readVec3f(const char*& cursor);
        Vec3f readVec3f(const char* const& cursor);
        void readBytes(const char*& cursor, char* buffer, size_t n);
        void readBytes(const char* const& cursor, char* buffer, size_t n);
        void readBytes(const char*& cursor, unsigned char* buffer, size_t n);
        void readBytes(const char* const& cursor, unsigned char* buffer, size_t n);
        
        template <typename T>
        void readVector(const char*& cursor, std::vector<T>& vec, const size_t size = sizeof(T)) {
            readBytes(cursor, reinterpret_cast<char*>(&vec.front()), vec.size() * size);
        }
        
        template <typename T>
        void readVector(const char* const& cursor, std::vector<T>& vec, const size_t size = sizeof(T)) {
            readBytes(cursor, reinterpret_cast<char*>(&vec.front()), vec.size() * size);
        }
    }
}

#endif
