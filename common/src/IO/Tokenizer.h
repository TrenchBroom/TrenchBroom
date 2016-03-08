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

#ifndef TrenchBroom_Tokenizer_h
#define TrenchBroom_Tokenizer_h

#include "Exceptions.h"
#include "Token.h"

#include <cassert>
#include <stack>

namespace TrenchBroom {
    namespace IO {
        template <typename TokenType>
        class Tokenizer {
        public:
            typedef TokenTemplate<TokenType> Token;
        private:
            typedef std::stack<Token> TokenStack;
            
            struct State {
                const char* cur;
                size_t line;
                size_t column;
                size_t lastColumn;
                
                State(const char* i_cur) :
                cur(i_cur),
                line(1),
                column(1),
                lastColumn(0) {}
            };
            
            const char* m_begin;
            const char* m_end;
            State m_state;
            
            TokenStack m_tokenStack;
        public:
            static const String Whitespace;
        public:
            Tokenizer(const char* begin, const char* end) :
            m_begin(begin),
            m_end(end),
            m_state(State(m_begin)) {}
            
            Tokenizer(const String& str) :
            m_begin(str.c_str()),
            m_end(str.c_str() + str.size()),
            m_state(State(m_begin)) {}
            
            virtual ~Tokenizer() {}
            
            Token nextToken() {
                return !m_tokenStack.empty() ? popToken() : emitToken();
            }
            
            Token peekToken() {
                Token token = nextToken();
                pushToken(token);
                return token;
            }
            
            void pushToken(const Token& token) {
                m_tokenStack.push(token);
            }
            
            Token popToken() {
                assert(!m_tokenStack.empty());
                Token token = m_tokenStack.top();
                m_tokenStack.pop();
                return token;
            }
            
            String readRemainder(const TokenType delimiterType) {
                if (eof())
                    return "";
                
                Token token = peekToken();
                const char* startPos = token.begin();
                const char* endPos = startPos;
                token = nextToken();
                while ((token.type() & delimiterType) == 0 && !eof()) {
                    endPos = token.end();
                    token = nextToken();
                }
                
                pushToken(token);
                return String(startPos, static_cast<size_t>(endPos - startPos));
            }
            
            String readAnyString(const String& delims) {
                while (isWhitespace(curChar()))
                    advance();
                const char* startPos = curPos();
                const char* endPos = (curChar() == '"' ? readQuotedString() : readString(delims));
                return String(startPos, static_cast<size_t>(endPos - startPos));
            }
            
            void reset() {
                m_state = State(m_begin);
            }

            double progress() const {
                if (length() == 0)
                    return 0.0;
                const double cur = static_cast<double>(offset(curPos()));
                const double len = static_cast<double>(length());
                return cur / len;
            }
        protected:
            size_t line() const {
                return m_state.line;
            }
            
            size_t column() const {
                return m_state.column;
            }
            
            bool eof() const {
                return m_state.cur >= m_end;
            }
            
            size_t length() const {
                return static_cast<size_t>(m_end - m_begin);
            }
            
            size_t offset(const char* ptr) const {
                assert(ptr >= m_begin);
                return static_cast<size_t>(ptr - m_begin);
            }
            
            const char* curPos() const {
                return m_state.cur;
            }
            
            char curChar() const {
                if (eof())
                    return 0;
                
                return *curPos();
            }
            
            char lookAhead(const size_t offset = 1) {
                if (m_state.cur + offset >= m_end)
                    return 0;
                return *(m_state.cur + offset);
            }
            
            void advance() {
                errorIfEof();
                
                if (curChar() == '\n') {
                    ++m_state.line;
                    m_state.lastColumn = m_state.column;
                    m_state.column = 1;
                } else {
                    ++m_state.column;
                }
                
                ++m_state.cur;
            }
            
            void retreat() {
                if (curPos() == m_begin)
                    throw ParserException("Cannot retreat beyond beginning of file");
                if (curChar() == '\n') {
                    --m_state.line;
                    if (m_state.lastColumn > 0) {
                        m_state.column = m_state.lastColumn;
                        m_state.lastColumn = 0;
                    } else {
                        m_state.column = 1;
                        const char* c = m_state.cur - 1;
                        while (c > m_begin && *c != '\n') {
                            --c;
                            ++m_state.column;
                        }
                    }
                } else {
                    --m_state.column;
                }
                --m_state.cur;
            }
            
            bool isDigit(const char c) const {
                return c >= '0' && c <= '9';
            }
            
            bool isWhitespace(const char c) const {
                return isAnyOf(c, Whitespace);
            }

            const char* readInteger(const String& delims) {
                if (curChar() != '+' && curChar() != '-' && !isDigit(curChar()))
                    return NULL;
                
                const State previous = m_state;
                if (curChar() == '+' || curChar() == '-')
                    advance();
                while (!eof() && isDigit(curChar()))
                    advance();
                if (eof() || isAnyOf(curChar(), delims))
                    return curPos();
                
                m_state = previous;
                return NULL;
            }
            
            const char* readDecimal(const String& delims) {
                if (curChar() != '+' && curChar() != '-' && curChar() != '.' && !isDigit(curChar()))
                    return NULL;
                
                const State previous = m_state;
                advance();
                while (!eof() && isDigit(curChar()))
                    advance();
                if (curChar() == '.') {
                    advance();
                    while (!eof() && isDigit(curChar()))
                        advance();
                }
                if (curChar() == 'e') {
                    advance();
                    if (curChar() == '+' || curChar() == '-' || isDigit(curChar())) {
                        advance();
                        while (!eof() && isDigit(curChar()))
                            advance();
                    }
                }
                if (eof() || isAnyOf(curChar(), delims))
                    return curPos();
                
                m_state = previous;
                return NULL;
            }
            
            const char* readString(const String& delims) {
                while (!eof() && !isAnyOf(curChar(), delims))
                    advance();
                return curPos();
            }
            
            const char* readQuotedString() {
                while (!eof() && curChar() != '"')
                    advance();
                errorIfEof();
                const char* end = curPos();
                advance();
                return end;
            }
            
            void discardWhile(const String& allow) {
                while (!eof() && isAnyOf(curChar(), allow))
                    advance();
            }
            
            void discardUntil(const String& delims) {
                while (!eof() && !isAnyOf(curChar(), delims))
                    advance();
            }
            
            void error(const char c) const {
                ParserException e;
                e << "Unexpected character '" << c << "'";
                throw e;
            }
            
            void errorIfEof() const {
                if (eof())
                    throw ParserException("Unexpected end of file");
            }
        private:
            bool isAnyOf(const char c, const String& allow) const {
                for (size_t i = 0; i < allow.size(); i++)
                    if (c == allow[i])
                        return true;
                return false;
            }
            
            virtual Token emitToken() = 0;
        };
        
        template <typename TokenType>
        const String Tokenizer<TokenType>::Whitespace = " \t\n\r";
    }
}
#endif
