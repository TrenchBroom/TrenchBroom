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

#ifndef TrenchBroom_Tokenizer_h
#define TrenchBroom_Tokenizer_h

#include "Exceptions.h"
#include "IO/Token.h"

#include <cassert>
#include <stack>

namespace TrenchBroom {
    namespace IO {
        template <typename TokenType>
        class Tokenizer {
        protected:
            typedef TokenTemplate<TokenType> Token;
        private:
            typedef std::stack<Token> TokenStack;

            const char* m_begin;
            const char* m_end;
            const char* m_cur;
            size_t m_line;
            size_t m_column;
            size_t m_lastColumn;
            
            TokenStack m_tokenStack;
        public:
            Tokenizer(const char* begin, const char* end) :
            m_begin(begin),
            m_end(end),
            m_cur(m_begin),
            m_line(1),
            m_column(1),
            m_lastColumn(0) {}
            
            Tokenizer(const String& str) :
            m_begin(str.c_str()),
            m_end(str.c_str() + str.size()),
            m_cur(m_begin),
            m_line(1),
            m_column(1),
            m_lastColumn(0) {}

            virtual ~Tokenizer() {}
            
            inline Token nextToken() {
                return !m_tokenStack.empty() ? popToken() : emitToken();
            }
            
            inline Token peekToken() {
                Token token = nextToken();
                pushToken(token);
                return token;
            }
            
            inline void pushToken(Token& token) {
                m_tokenStack.push(token);
            }
            
            inline Token popToken() {
                assert(!m_tokenStack.empty());
                Token token = m_tokenStack.top();
                m_tokenStack.pop();
                return token;
            }
            
            inline String readRemainder(unsigned int delimiterType) {
                if (eof())
                    return "";
                
                const char* startPos = m_cur;
                const char* endPos = m_cur;
                TokenType token = nextToken();
                while (token.type() != delimiterType && !eof()) {
                    endPos = m_cur;
                    token = nextToken();
                }
                
                pushToken(token);
                return String(startPos, static_cast<size_t>(endPos - startPos));
            }
        protected:
            inline size_t line() const {
                return m_line;
            }
            
            inline size_t column() const {
                return m_column;
            }
            
            inline bool eof() const {
                return m_cur >= m_end;
            }
            
            inline size_t length() const {
                return static_cast<size_t>(m_end - m_begin);
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
            
            inline bool isDigit(const char c) const {
                return c >= '0' && c <= '9';
            }
            
            inline bool isWhitespace(const char c) const {
                return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == 0;
            }
            
            inline const char* readInteger(const char* begin, const String& delims) {
                const char* c = begin;
                if ((*c == '-' && !eof()) || isDigit(*c)) {
                    while (!eof() && isDigit(*(c = nextChar())));
                    if (eof() || isDelimiter(*c, delims)) {
                        if (!eof())
                            pushChar();
                        return c;
                    }
                }
                return begin;
            }
            
            inline const char* readDecimal(const char* begin, const String& delims) {
                const char* c = begin;
                if (((*c == '-' || *c == '.') && !eof()) || isDigit(*c)) {
                    while (!eof() && isDigit(*(c = nextChar())));
                    if (*c == '.' && eof())
                        return begin;
                    if (*c == '.')
                        while (!eof() && isDigit(*(c = nextChar())));
                    if (*c == 'e' && eof())
                        return begin;
                    if (*c == 'e') {
                        c = nextChar();
                        if ((*c == '+' || *c == '-') && eof())
                            return begin;
                        if (*c == '+' || *c == '-' || isDigit(*c))
                            while (!eof() && isDigit(*(c = nextChar())));
                    }
                    if (eof() || isDelimiter(*c, delims)) {
                        if (!eof())
                            pushChar();
                        return c;
                    }
                }
                return begin;
            }
            
            inline const char* readString(const char* begin, const String& delims) {
                const char* c = begin;
                while (!eof() && !isDelimiter(*(c = nextChar()), delims));
                if (!eof())
                    pushChar();
                return c;
            }
            
            inline const char* readQuotedString(const char* begin) {
                const char* c = begin;
                while (!eof() && *(c = nextChar()) != '"');
                errorIfEof();
                return c;
            }
              
            inline void reset() {
                m_line = 1;
                m_column = 1;
                m_cur = m_begin;
            }
            
            inline void error(const char c) const {
                ParserException e;
                e << "Unexpected character '" << c << "'";
                throw e;
            }
            
            inline void errorIfEof() const {
                if (eof()) {
                    ParserException e;
                    e << "Unexpected end of file";
                    throw e;
                }
            }
        private:
            inline bool isDelimiter(const char c, const String& delims) const {
                for (size_t i = 0; i < delims.size(); i++)
                    if (c == delims[i])
                        return true;
                return false;
            }

            virtual Token emitToken() = 0;
        };
    }
}

#endif
