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

#ifndef __TrenchBroom__AbstractTokenizer__
#define __TrenchBroom__AbstractTokenizer__

#include "IO/ParserException.h"
#include "Utility/Pool.h"

#include <cstdlib>
#include <istream>

namespace TrenchBroom {
    namespace IO {
        template <typename TokenType, typename SubClass>
        class AbstractToken {
        private:
			static Utility::Pool<SubClass> pool;
        protected:
            TokenType m_type;
            const String m_data;
            size_t m_position;
            size_t m_line;
            size_t m_column;
        public:
            inline void* operator new(size_t size) {
                if (!pool.empty())
                    return pool.pop();
                    
                    return malloc(size);
            }
            
            inline void operator delete(void* pointer) {
                if (!pool.push(static_cast<SubClass*>(pointer)))
                    free(pointer);
            }
            
            AbstractToken(TokenType type, const String& data, size_t position, size_t line, size_t column) : m_type(type), m_data(data), m_position(position), m_line(line), m_column(column) {}
            
            inline TokenType type() const {
                return m_type;
            }
            
            inline const String& data() const {
                return m_data;
            }
            
            inline size_t position() const {
                return m_position;
            }
            
            inline size_t line() const {
                return m_line;
            }
            
            inline size_t column() const {
                return m_column;
            }
            
            inline float toFloat() const {
                return static_cast<float>(atof(m_data.c_str()));
            }
            
            inline int toInteger() const {
                return static_cast<int>(atoi(m_data.c_str()));
            }
        };
        
        template <typename TokenType, typename SubClass>
        Utility::Pool<SubClass> AbstractToken<TokenType, SubClass>::pool(5);
        
        class AbstractTokenizer {
        protected:
            std::istream& m_stream;
            size_t m_line;
            size_t m_column;
            size_t m_position;

            inline char nextChar() {
                if (eof())
                    throw ParserException(m_line, m_column, "unexpected end of file");
                
                char c;
                m_stream.get(c);
                m_position++;
                
                if (c == '\n') {
                    m_line++;
                    m_column = 1;
                } else {
                    m_column++;
                }
                
                return c;
            }
            
            inline void pushChar() {
                m_stream.seekg(-1, std::ios::cur);
                char c = m_stream.peek();
                m_position--;
                if (c == '\n') {
                    m_line--;
                    m_column = 0;
                    char d;
                    do {
                        m_stream.seekg(-1, std::ios::cur);
                        d = m_stream.peek();
                        m_column++;
                    } while (d != '\n' && m_stream.tellg() > 0);
                    m_stream.seekg(m_column, std::ios::cur);
                } else {
                    m_column--;
                }
            }
            
            inline char peekChar(unsigned int offset = 0) {
                if (eof())
                    throw ParserException(m_line, m_column, "unexpected end of file");

                char c;
                if (offset == 0) {
                    c = m_stream.peek();
                } else {
                    m_stream.seekg(offset, std::ios::cur);
                    c = m_stream.peek();
                    m_stream.seekg(-static_cast<long>(offset), std::ios::cur);
                }
                return c;
            }
            
            inline bool eof() const {
                return m_stream.eof();
            }
        public:
            AbstractTokenizer(std::istream& stream) : m_stream(stream), m_line(1), m_column(1), m_position(0) {}

            inline size_t line() const {
                return m_line;
            }
            
            inline size_t column() const {
                return m_column;
            }
            
            inline size_t position() const {
                return m_position;
            }
        };
    }
}


#endif /* defined(__TrenchBroom__AbstractTokenizer__) */
