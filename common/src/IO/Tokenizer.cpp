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

#include "Tokenizer.h"

#include "Exceptions.h"
#include "Macros.h"

#include <kdl/string_format.h>

#include <string>

namespace TrenchBroom {
    namespace IO {
        TokenizerState::TokenizerState(const char* begin, const char* end, const std::string& escapableChars, const char escapeChar) :
        m_begin(begin),
        m_cur(m_begin),
        m_end(end),
        m_escapableChars(escapableChars),
        m_escapeChar(escapeChar),
        m_line(1),
        m_column(1),
        m_escaped(false) {}

        TokenizerState TokenizerState::clone(const char* begin, const char* end) const {
            return TokenizerState(begin, end, m_escapableChars, m_escapeChar);
        }

        char TokenizerState::curChar() const {
            return *m_cur;
        }

        char TokenizerState::lookAhead(const size_t offset) const {
            if (eof(m_cur + offset)) {
                return 0;
            } else {
                return *(m_cur + offset);
            }
        }

        size_t TokenizerState::line() const {
            return m_line;
        }

        size_t TokenizerState::column() const {
            return m_column;
        }

        bool TokenizerState::escaped() const {
            return !eof() && m_escaped && m_escapableChars.find(curChar()) != std::string::npos;
        }

        std::string TokenizerState::unescape(const std::string& str) const {
            return kdl::str_unescape(str, m_escapableChars, m_escapeChar);
        }

        void TokenizerState::resetEscaped() {
            m_escaped = false;
        }

        bool TokenizerState::eof() const {
            return eof(m_cur);
        }

        bool TokenizerState::eof(const char* ptr) const {
            return ptr >= m_end;
        }

        size_t TokenizerState::offset(const char* ptr) const {
            assert(ptr >= m_begin);
            return static_cast<size_t>(ptr - m_begin);
        }

        void TokenizerState::advance(const size_t offset) {
            for (size_t i = 0; i < offset; ++i) {
                advance();
            }
        }

        void TokenizerState::advance() {
            errorIfEof();

            switch (curChar()) {
                case '\r':
                    if (lookAhead() == '\n') {
                        ++m_column;
                        break;
                    }
                    // handle carriage return without consecutive line feed
                    // by falling through into the line feed case
                    switchFallthrough();
                case '\n':
                    ++m_line;
                    m_column = 1;
                    m_escaped = false;
                    break;
                default:
                    ++m_column;
                    if (curChar() == m_escapeChar) {
                        m_escaped = !m_escaped;
                    } else {
                        m_escaped = false;
                    }
                    break;
            }
            ++m_cur;
        }

        void TokenizerState::reset() {
            m_cur = m_begin;
            m_line = 1;
            m_column = 1;
            m_escaped = false;
        }

        void TokenizerState::errorIfEof() const {
            if (eof()) {
                throw ParserException("Unexpected end of file");
            }
        }

        TokenizerState TokenizerState::snapshot() const {
            return TokenizerState(*this);
        }

        void TokenizerState::restore(const TokenizerState& snapshot) {
            *this = snapshot;
        }
    }
}
