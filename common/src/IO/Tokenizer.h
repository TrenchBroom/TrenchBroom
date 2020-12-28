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
            TokenizerBase(const char* begin, const char* end, std::string_view escapableChars, char escapeChar);

            void replaceState(std::string_view str);

            TokenizerStateAndSource snapshotStateAndSource() const;
            void restoreStateAndSource(const TokenizerStateAndSource& snapshot);
        protected:
            char curChar() const;

            char lookAhead(size_t offset = 1) const;

            size_t line() const;
            size_t column() const;

            bool escaped() const;
            std::string unescape(std::string_view str) const;
            void resetEscaped();

            bool eof() const;
            bool eof(const char* ptr) const;

            size_t offset(const char* ptr) const;

            void advance(size_t offset);
            void advance();
            void reset();

            void errorIfEof() const;

            TokenizerState snapshot() const;
            void restore(const TokenizerState& snapshot);
        };

        template <typename TokenType>
        class Tokenizer : public TokenizerBase {
        public:
            using Token = TokenTemplate<TokenType>;
        private:
            class SaveState {
            private:
                TokenizerState* m_target;
                TokenizerState m_snapshot;
            public:
                explicit SaveState(TokenizerState* target) :
                m_target(target),
                m_snapshot(*target) {}

                ~SaveState() {
                    *m_target = m_snapshot;
                }
            };

            template <typename T> friend class Tokenizer;
        public:
            static const std::string& Whitespace() {
                static const std::string whitespace(" \t\n\r");
                return whitespace;
            }
        public:
            Tokenizer(std::string_view str, std::string_view escapableChars, const char escapeChar) :
            TokenizerBase(str.data(), str.data() + str.size(), escapableChars, escapeChar) {}

            virtual ~Tokenizer() = default;

            Token nextToken(const TokenType skipTokens = 0u) {
                auto token = emitToken();
                while (token.hasType(skipTokens)) {
                    token = emitToken();
                }
                return token;
            }

            Token peekToken(const TokenType skipTokens = 0u) {
                SaveState oldState(&m_state);
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

            std::string_view readAnyString(std::string_view delims) {
                while (isWhitespace(curChar())) {
                    advance();
                }
                const char* startPos = curPos();
                const char* endPos = (curChar() == '"' ? readQuotedString() : readUntil(delims));
                return std::string_view(startPos, static_cast<size_t>(endPos - startPos));
            }

            std::string unescapeString(std::string_view str) const {
                return unescape(str);
            }

            void reset() {
                TokenizerBase::reset();
            }

            double progress() const {
                if (length() == 0) {
                    return 0.0;
                }
                const auto cur = static_cast<double>(offset(curPos()));
                const auto len = static_cast<double>(length());
                return cur / len;
            }

            bool eof() const {
                return TokenizerBase::eof();
            }
        public:
            size_t line() const {
                return TokenizerBase::line();
            }

            size_t column() const {
                return TokenizerBase::column();
            }

            size_t length() const {
                return static_cast<size_t>(m_end - m_begin);
            }

            std::string_view remainder() const {
                return std::string_view(curPos(), length());
            }

            size_t curOffset() const {
                return offset(curPos());
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
        public:
            void advance(const size_t offset) {
                TokenizerBase::advance(offset);
            }
        protected:
            void advance() {
                advance(1);
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
