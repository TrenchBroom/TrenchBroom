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

#ifndef TrenchBroom_Token
#define TrenchBroom_Token

#include "StringUtils.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace TrenchBroom {
    namespace IO {
        template <typename Type>
        class TokenTemplate {
        private:
            Type m_type;
            const char* m_begin;
            const char* m_end;
            size_t m_position;
            size_t m_line;
            size_t m_column;
        public:
            TokenTemplate() :
            m_type(0),
            m_begin(NULL),
            m_end(NULL),
            m_position(0),
            m_line(0),
            m_column(0) {}
            
            TokenTemplate(const Type type, const char* begin, const char* end, const size_t position, const size_t line, const size_t column) :
            m_type(type),
            m_begin(begin),
            m_end(end),
            m_position(position),
            m_line(line),
            m_column(column) {
                assert(end >= begin);
            }
            
            Type type() const {
                return m_type;
            }
            
            const char* begin() const {
                return m_begin;
            }
            
            const char* end() const {
                return m_end;
            }
            
            const String data() const {
                return String(m_begin, length());
            }
            
            size_t position() const {
                return m_position;
            }
            
            size_t length() const {
                return static_cast<size_t>(m_end - m_begin);
            }
            
            size_t line() const {
                return m_line;
            }
            
            size_t column() const {
                return m_column;
            }
            
            template <typename T>
            T toFloat() const {
                static const size_t BufferSize = 256;
                static char buffer[BufferSize];
                assert(length() < BufferSize);
                
                memcpy(buffer, m_begin, length());
                buffer[length()] = 0;
                const T f = static_cast<T>(std::atof(buffer));
                return f;
            }
            
            template <typename T>
            T toInteger() const {
                static char buffer[64];
                assert(length() < 64);
                
                memcpy(buffer, m_begin, length());
                buffer[length()] = 0;
                const T i = static_cast<T>(std::atoi(buffer));
                return i;
            }
        };
    }
}

#endif /* defined(TrenchBroom_Token) */
