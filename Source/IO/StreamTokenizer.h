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

#ifndef __TrenchBroom__StreamTokenizer__
#define __TrenchBroom__StreamTokenizer__

#include "IO/ParserException.h"
#include "Utility/Allocator.h"

#include <cassert>
#include <cstdlib>
#include <istream>
#include <memory>
#include <stack>

namespace TrenchBroom {
    namespace IO {
        class Token : public Utility::Allocator<Token> {
        protected:
            unsigned int m_type;
            String m_data;
            size_t m_position;
            size_t m_length;
            size_t m_line;
            size_t m_column;
        public:
            Token() :
            m_type(0),
            m_data(""),
            m_position(0),
            m_length(0),
            m_line(0),
            m_column(0) {}

            Token(unsigned int type, const String& data, size_t position, size_t length, size_t line, size_t column) :
            m_type(type),
            m_data(data),
            m_position(position),
            m_length(length),
            m_line(line),
            m_column(column) {}
            
            inline unsigned int type() const {
                return m_type;
            }
            
            inline const String& data() const {
                return m_data;
            }
            
            inline size_t position() const {
                return m_position;
            }
            
            inline size_t length() const {
                return m_length;
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
        
        template <typename Emitter>
        class StreamTokenizer {
        private:
            typedef std::stack<Token> TokenStack;
            
            std::istream& m_stream;
            size_t m_line;
            size_t m_column;
            size_t m_position;
            size_t m_length;
            
            Emitter m_emitter;
            TokenStack m_tokenStack;
        protected:
            inline Token popToken() {
                assert(!m_tokenStack.empty());
                Token token = m_tokenStack.top();
                m_tokenStack.pop();
                return token;
            }
        public:
            StreamTokenizer(std::istream& stream) :
            m_stream(stream),
            m_line(1),
            m_column(1),
            m_position(0) {
                m_stream.seekg(0, std::ios::end);
                m_length = static_cast<size_t>(m_stream.tellg());
                m_stream.seekg(0, std::ios::beg);
            }

            inline size_t position() const {
                return m_position;
            }
            
            inline char nextChar() {
                if (eof())
                    return 0;
                
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
                int c = m_stream.peek();
                m_position--;
                if (c == '\n') {
                    m_line--;
                    m_column = 0;
                    int d;
                    do {
                        m_stream.seekg(-1, std::ios::cur);
                        d = m_stream.peek();
                        m_column++;
                    } while (d != '\n' && m_stream.tellg() > 0);
                    m_stream.seekg(static_cast<std::streamoff>(m_column), std::ios::cur);
                } else {
                    m_column--;
                }
            }
            
            inline char peekChar(unsigned int offset = 0) {
                if (eof())
                    return 0;
                
                int c;
                if (offset == 0) {
                    c = m_stream.peek();
                } else {
                    m_stream.seekg(offset, std::ios::cur);
                    c = m_stream.peek();
                    m_stream.seekg(-static_cast<long>(offset), std::ios::cur);
                }
                return static_cast<char>(c);
            }
            
            inline bool eof() const {
                return m_stream.eof() || m_position >= m_length;
            }
            
            inline Token nextToken() {
                return !m_tokenStack.empty() ? popToken() : m_emitter.emit(*this, m_line, m_column);
            }
            
            inline Token peekToken() {
                Token token = nextToken();
                pushToken(token);
                return token;
            }
            
            inline void pushToken(Token& token) {
                m_tokenStack.push(token);
            }
            
            inline String remainder(unsigned int delimiterType) {
                if (eof())
                    return "";
                
                Token token = nextToken();
                const size_t oldPosition = token.position();
                while (token.type() != delimiterType && !eof()) {
                    token = nextToken();
                }
                
                const std::streampos newPosition = m_stream.tellg();
                const size_t numChars = token.position() - oldPosition;
                const std::streamoff offset = static_cast<std::streamoff>(oldPosition - token.position() - token.length());
                m_stream.seekg(offset, std::ios::cur);
                
                char* buffer = new char[numChars];
                m_stream.readsome(buffer, static_cast<std::streamsize>(numChars));
                
                String result(buffer, numChars);
                delete [] buffer;
                buffer = NULL;
                
                m_stream.seekg(static_cast<std::streamoff>(newPosition), std::ios::beg);
                pushToken(token);
                
                return result;
            }
            
            inline void reset() {
                m_line = 1;
                m_column = 1;
                m_position = 0;
                m_stream.seekg(0, std::ios::beg);
            }
        };

        template <typename Subclass>
        class TokenEmitter {
        protected:
            typedef StreamTokenizer<Subclass> Tokenizer;
            virtual Token doEmit(Tokenizer& tokenizer, size_t line, size_t column) = 0;
            
            inline bool isDigit(char c) const {
                return c >= '0' && c <= '9';
            }
            
            inline bool isWhitespace(char c) const {
                return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == 0;
            }
            
            inline void error(size_t line, size_t column, char c) const {
                StringStream msg;
                msg << "Unexpected character '" << c << "'";
                throw ParserException(line, column, msg.str());
            }
        public:
            virtual ~TokenEmitter() {}
            
            Token emit(Tokenizer& tokenizer, size_t line, size_t column) {
                return doEmit(tokenizer, line, column);
            }
        };
    }
}


#endif /* defined(__TrenchBroom__StreamTokenizer__) */
