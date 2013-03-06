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

#ifndef TrenchBroom_ByteBuffer_h
#define TrenchBroom_ByteBuffer_h

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class ByteBuffer {
        public:
            static const ByteBuffer EmptyBuffer;
        private:
            typedef std::vector<char> Buffer;
            Buffer m_buffer;
            size_t m_index;
        public:
            ByteBuffer(size_t size = 0) :
            m_buffer(size),
            m_index(0) {}
            
            template <typename T>
            inline void operator<<(const T& value) {
                union coercion { T value; char data[sizeof(T)]; };
                coercion c;
                c.value = value;
                for (size_t i = 0; i < sizeof(T); i++)
                    m_buffer.push_back(c.data[i]);
            }
            
            template <typename T>
            inline void operator>>(T& value) {
                assert(m_index + sizeof(T) <= size());
                
                union coercion { T value; char data[sizeof(T)]; };
                coercion c;
                c.value = value;
                for (size_t i = 0; i < sizeof(T); i++)
                    c.data[i] = m_buffer[m_index + i];
                value = c.value;
                m_index += sizeof(T);
            }
            
            inline void reset() {
                m_index = 0;
            }
            
            inline bool empty() const {
                return m_buffer.empty();
            }
            
            inline size_t size() const {
                return m_buffer.size();
            }
            
            inline char* get() {
                assert(!empty());
                return &m_buffer[0];
            }
            
            inline const char* get() const {
                assert(!empty());
                return &m_buffer[0];
            }
        };
    }
}

#endif
