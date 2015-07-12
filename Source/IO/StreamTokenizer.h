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
#include <cstring>
#include <istream>
#include <memory>
#include <stack>

namespace TrenchBroom {
    namespace IO {
        class Token : public Utility::Allocator<Token> {
        protected:
            unsigned int m_type;
            size_t m_position;
            size_t m_line;
            size_t m_column;
        public:
            const char* m_begin;
            const char* m_end;

            Token() :
            m_type(0),
            m_begin(NULL),
            m_end(NULL),
            m_position(0),
            m_line(0),
            m_column(0) {}

            Token(unsigned int type, const char* begin, const char* end, size_t position, size_t line, size_t column) :
            m_type(type),
            m_begin(begin),
            m_end(end),
            m_position(position),
            m_line(line),
            m_column(column) {
                assert(end >= begin);
            }

            inline unsigned int type() const {
                return m_type;
            }

            inline const String data() const {
                return String(m_begin, length());
            }

            inline size_t position() const {
                return m_position;
            }

            inline size_t length() const {
                return static_cast<size_t>(m_end - m_begin);
            }

            inline size_t line() const {
                return m_line;
            }

            inline size_t column() const {
                return m_column;
            }

            inline float toFloat() const {
                static char buffer[64];
                memcpy(buffer, m_begin, length());
                buffer[length()] = 0;
                float f = static_cast<float>(std::atof(buffer));
                return f;
            }

            inline int toInteger() const {
                static char buffer[64];
                memcpy(buffer, m_begin, length());
                buffer[length()] = 0;
                int i = static_cast<int>(std::atoi(buffer));
                return i;
            }
        };

        template <typename Emitter>
        class StreamTokenizer {
        private:
            typedef std::stack<Token> TokenStack;

            const char* m_begin;
            const char* m_end;
            const char* m_cur;
            size_t m_line;
            size_t m_column;
            size_t m_lastColumn;

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
            StreamTokenizer(const char* begin, const char* end) :
            m_begin(begin),
            m_end(end),
            m_cur(begin),
            m_line(1),
            m_column(1),
            m_lastColumn(0) {}

            inline size_t line() const {
                return m_line;
            }

            inline size_t column() const {
                return m_column;
            }

            inline size_t offset(const char* ptr) const {
                assert(ptr >= m_begin);
                return static_cast<size_t>(ptr - m_begin);
            }

            inline const char* nextChar() {
                if (eof())
                    return 0;

                if (*m_cur == '\n') {
                    m_line++;
                    m_lastColumn = m_column;
                    m_column = 1;
                } else {
                    m_column++;
                }

                return m_cur++;
            }

            inline void pushChar() {
                assert(m_cur > m_begin);
                if (*--m_cur == '\n') {
                    m_line--;
                    m_column = m_lastColumn;
                } else {
                    m_column--;
                }
            }

            inline char peekChar(size_t offset = 0) {
                if (eof())
                    return 0;

                assert(m_cur + offset < m_end);
                return *(m_cur + offset);
            }

            inline bool eof() const {
                return m_cur >= m_end;
            }

            inline Token nextToken() {
                return !m_tokenStack.empty() ? popToken() : m_emitter.emit(*this);
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
                const char* startPos = token.m_begin;
                const char* endPos = token.m_begin;
                
                while (token.type() != delimiterType && !eof()) {
                    endPos = token.m_end;
                    token = nextToken();
                }

                pushToken(token);
                return String(startPos, static_cast<size_t>(endPos - startPos));
            }

            inline void quotedString(const char*& begin, const char*& end) {
                assert(*begin == '"');
                begin = nextChar();
                end = begin;
                while (!eof() && *end != '"')
                    end = nextChar();
            }

            inline void reset() {
                m_line = 1;
                m_column = 1;
                m_cur = m_begin;
            }
        };

        template <typename Subclass>
        class TokenEmitter {
        protected:
            typedef StreamTokenizer<Subclass> Tokenizer;
            virtual Token doEmit(Tokenizer& tokenizer) = 0;

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

            Token emit(Tokenizer& tokenizer) {
                return doEmit(tokenizer);
            }
        };
    }
}


#endif /* defined(__TrenchBroom__StreamTokenizer__) */
