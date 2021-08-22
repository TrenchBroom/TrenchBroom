/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include "Token.h"

#include "Exceptions.h"
#include "Macros.h"

#include <kdl/string_format.h>

#include <cassert>
#include <tuple>
#include <string>
#include <string_view>

namespace TrenchBroom {
    namespace IO {
        struct TokenizerState {
            const char* cur;
            size_t line;
            size_t column;
            bool escaped;
        };

        struct TokenizerStateAndSource {
            TokenizerState state;
            const char* begin;
            const char* end;
        };

        class TokenizerBase {
        protected:
            const char* m_begin;
            const char* m_end;
            std::string m_escapableChars;
            char m_escapeChar;
            TokenizerState m_state;
        public:
            TokenizerBase(const char* begin, const char* end, std::string_view escapableChars, const char escapeChar, const size_t line, const size_t column) :
            m_begin(begin),
            m_end(end),
            m_escapableChars(escapableChars),
            m_escapeChar(escapeChar),
            m_state{begin, line, column, false} {}

            void replaceState(std::string_view str) {
                m_begin = str.data();
                m_end = str.data() + str.length();
                // preserve m_escapableChars and m_escapeChar
                reset();
            }

            TokenizerStateAndSource snapshotStateAndSource() const {
                return { m_state, m_begin, m_end };
            }
            void restoreStateAndSource(const TokenizerStateAndSource& snapshot) {
                m_state = snapshot.state;
                m_begin = snapshot.begin;
                m_end = snapshot.end;
            }
        protected:
            /**
             * Returns current character; caller must ensure eof() is false before calling.
             */
            char curChar() const {
                return *m_state.cur;
            }

            char lookAhead(size_t offset = 1) const {
                if (eof(m_state.cur + offset)) {
                    return 0;
                } else {
                    return *(m_state.cur + offset);
                }
            }

            bool escaped() const {
                return !eof() && m_state.escaped && m_escapableChars.find(curChar()) != std::string::npos;
            }

            std::string unescape(std::string_view str) const {
                return kdl::str_unescape(str, m_escapableChars, m_escapeChar);
            }

            void resetEscaped() {
                m_state.escaped = false;
            }

            bool eof(const char* ptr) const {
                return ptr >= m_end;
            }

            size_t offset(const char* ptr) const {
                assert(ptr >= m_begin);
                return static_cast<size_t>(ptr - m_begin);
            }

            void advance(size_t offset)  {
                for (size_t i = 0; i < offset; ++i) {
                    advance();
                }
            }

            void advance() {
                errorIfEof();

                switch (curChar()) {
                    case '\r':
                        if (lookAhead() == '\n') {
                            ++m_state.column;
                            break;
                        }
                        // handle carriage return without consecutive line feed
                        // by falling through into the line feed case
                        switchFallthrough();
                    case '\n':
                        ++m_state.line;
                        m_state.column = 1;
                        m_state.escaped = false;
                        break;
                    default:
                        ++m_state.column;
                        if (curChar() == m_escapeChar) {
                            m_state.escaped = !m_state.escaped;
                        } else {
                            m_state.escaped = false;
                        }
                        break;
                }
                ++m_state.cur;
            }

            void errorIfEof() const {
                if (eof()) {
                    throw ParserException("Unexpected end of file");
                }
            }

            TokenizerState snapshot() const {
                return m_state;
            }

            void restore(const TokenizerState& snapshot) {
                m_state = snapshot;
            }

        public:
            bool eof() const  {
                return eof(m_state.cur);
            }

            size_t line() const {
                return m_state.line;
            }

            size_t column() const {
                return m_state.column;
            }
        public:
            void reset() {
                m_state.cur = m_begin;
                m_state.line = 1;
                m_state.column = 1;
                m_state.escaped = false;
            }

            void adoptState(const TokenizerState& state) {
                assert(state.cur >= m_begin);
                assert(state.cur <= m_end);

                m_state.cur = state.cur;
                m_state.line = state.line;
                m_state.column = state.column;
                // m_state.escaped is not updated
            }
        };

        template <typename TokenType>
        class Tokenizer : public TokenizerBase {
        public:
            using Token = TokenTemplate<TokenType>;
        private:
            class SaveAndRestoreState {
            private:
                TokenizerState& m_target;
                TokenizerState m_snapshot;
            public:
                explicit SaveAndRestoreState(TokenizerState& target) :
                m_target(target),
                m_snapshot(target) {}

                ~SaveAndRestoreState() {
                    m_target = m_snapshot;
                }
            };

        public:
            static const std::string& Whitespace() {
                static const std::string whitespace(" \t\n\r");
                return whitespace;
            }
        public:
            Tokenizer(std::string_view str, std::string_view escapableChars, const char escapeChar, const size_t line = 1, const size_t column = 1) :
            TokenizerBase{str.data(), str.data() + str.size(), escapableChars, escapeChar, line, column} {}

            virtual ~Tokenizer() = default;

            Token nextToken(const TokenType skipTokens = 0u) {
                auto token = emitToken();
                while (token.hasType(skipTokens)) {
                    token = emitToken();
                }
                return token;
            }

            Token peekToken(const TokenType skipTokens = 0u) {
                SaveAndRestoreState oldState(m_state);
                return nextToken(skipTokens);
            }

            void skipToken(const TokenType skipTokens = ~0u) {
                if (peekToken().hasType(skipTokens)) {
                    nextToken();
                }
            }

            void discardLine() {
                discardUntil("\n");
                discardWhile("\n");
            }

            std::string_view readRemainder(const TokenType delimiterType) {
                if (eof()) {
                    return std::string_view();
                }

                Token token = peekToken();
                const char* startPos = std::begin(token);
                const char* endPos = nullptr;
                do {
                    token = nextToken();
                    endPos = std::end(token);
                } while (peekToken().hasType(delimiterType) == 0 && !eof());

                return std::string_view(startPos, static_cast<size_t>(endPos - startPos));
            }

            std::tuple<std::string_view, bool> readAnyString(std::string_view delims) {
                while (isWhitespace(curChar())) {
                    advance();
                }

                if (curChar() == '"') {
                    advance();
                    const char* startPos = curPos();
                    const char* endPos = readQuotedString();
                    return {std::string_view(startPos, static_cast<size_t>(endPos - startPos)), true};
                }

                const char* startPos = curPos();
                const char* endPos = readUntil(delims);
                return {std::string_view(startPos, static_cast<size_t>(endPos - startPos)), false};
            }

            std::string unescapeString(std::string_view str) const {
                return unescape(str);
            }

            double progress() const {
                if (length() == 0) {
                    return 0.0;
                }
                const auto cur = static_cast<double>(offset(curPos()));
                const auto len = static_cast<double>(length());
                return cur / len;
            }

            size_t length() const {
                return static_cast<size_t>(m_end - m_begin);
            }

            std::string_view remainder() const {
                return std::string_view(curPos(), length());
            }

        public:
            TokenizerState snapshot() const {
                return m_state;
            }

            void restore(const TokenizerState& snapshot) {
                m_state = snapshot;
            }
        protected:
            const char* curPos() const {
                return m_state.cur;
            }

            char curChar() const {
                if (eof()) {
                    return 0;
                }

                return *curPos();
            }

            bool isDigit(const char c) const {
                return c >= '0' && c <= '9';
            }

            bool isLetter(const char c) const {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            }

            bool isWhitespace(const char c) const {
                return isAnyOf(c, Whitespace());
            }

            bool isEscaped() const {
                return escaped();
            }

            const char* readInteger(std::string_view delims) {
                if (curChar() != '+' && curChar() != '-' && !isDigit(curChar())) {
                    return nullptr;
                }

                const TokenizerState previousState = m_state;
                if (curChar() == '+' || curChar() == '-') {
                    advance();
                }
                while (!eof() && isDigit(curChar())) {
                    advance();
                }
                if (eof() || isAnyOf(curChar(), delims)) {
                    return curPos();
                }

                m_state = previousState;
                return nullptr;
            }

            const char* readDecimal(std::string_view delims) {
                if (curChar() != '+' && curChar() != '-' && curChar() != '.' && !isDigit(curChar())) {
                    return nullptr;
                }

                const TokenizerState previousState = m_state;
                if (curChar() != '.') {
                    advance();
                    readDigits();
                }

                if (curChar() == '.') {
                    advance();
                    readDigits();
                }

                if (curChar() == 'e') {
                    advance();
                    if (curChar() == '+' || curChar() == '-' || isDigit(curChar())) {
                        advance();
                        readDigits();
                    }
                }

                if (eof() || isAnyOf(curChar(), delims)) {
                    return curPos();
                }

                m_state = previousState;
                return nullptr;
            }

        private:
            void readDigits() {
                while (!eof() && isDigit(curChar())) {
                    advance();
                }
            }
        protected:
            const char* readUntil(std::string_view delims) {
                if (!eof()) {
                    do {
                        advance();
                    } while (!eof() && !isAnyOf(curChar(), delims));
                }
                return curPos();
            }

            const char* readWhile(std::string_view allow) {
                while (!eof() && isAnyOf(curChar(), allow)) {
                    advance();
                }
                return curPos();
            }

            const char* readQuotedString(const char delim = '"', std::string_view hackDelims = std::string_view()) {
                while (!eof() && (curChar() != delim || isEscaped())) {
                    // This is a hack to handle paths with trailing backslashes that get misinterpreted as escaped double quotation marks.
                    if (!hackDelims.empty() && curChar() == '"' && isEscaped() && hackDelims.find(lookAhead()) != std::string_view::npos) {
                        resetEscaped();
                        break;
                    }
                    advance();
                }
                errorIfEof();
                const char* end = curPos();
                advance();
                return end;
            }

            const char* discardWhile(std::string_view allow) {
                while (!eof() && isAnyOf(curChar(), allow)) {
                    advance();
                }
                return curPos();
            }

            const char* discardUntil(std::string_view delims) {
                while (!eof() && !isAnyOf(curChar(), delims)) {
                    advance();
                }
                return curPos();
            }

            bool matchesPattern(std::string_view pattern) const {
                if (pattern.empty() || isEscaped() || curChar() != pattern[0]) {
                    return false;
                }
                for (size_t i = 1; i < pattern.size(); ++i) {
                    if (lookAhead(i) != pattern[i]) {
                        return false;
                    }
                }
                return true;
            }

            const char* discardUntilPattern(std::string_view pattern) {
                if (pattern.empty()) {
                    return curPos();
                }

                while (!eof() && !matchesPattern(pattern)) {
                    advance();
                }

                if (eof()) {
                    return m_end;
                }

                return curPos();
            }

            const char* discard(std::string_view str) {
                for (size_t i = 0; i < str.size(); ++i) {
                    const char c = lookAhead(i);
                    if (c == 0 || c != str[i]) {
                        return nullptr;
                    }

                }

                advance(str.size());
                return curPos();
            }
        protected:
            bool isAnyOf(const char c, std::string_view allow) const {
                for (const auto& a : allow) {
                    if (c == a) {
                        return true;
                    }
                }
                return false;
            }

            virtual Token emitToken() = 0;
        };
    }
}
