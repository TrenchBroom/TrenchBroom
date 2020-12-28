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
        TokenizerBase::TokenizerBase(const char* begin, const char* end, const std::string& escapableChars, const char escapeChar) :
        m_begin(begin),
        m_end(end),
        m_escapableChars(escapableChars),
        m_escapeChar(escapeChar),
        m_state{begin, 1, 1, false} {}

        void TokenizerBase::replaceState(std::string_view str) {
            m_begin = str.data();
            m_end = str.data() + str.length();
            // preserve m_escapableChars and m_escapeChar
            reset();
        }

        TokenizerStateAndSource TokenizerBase::snapshotStateAndSource() const {
            return { m_state, m_begin, m_end };
        }

        void TokenizerBase::restoreStateAndSource(const TokenizerStateAndSource& snapshot) {
            m_state = snapshot.state;
            m_begin = snapshot.begin;
            m_end = snapshot.end;
        }

        char TokenizerBase::curChar() const {
            return *m_state.cur;
        }

        char TokenizerBase::lookAhead(const size_t offset) const {
            if (eof(m_state.cur + offset)) {
                return 0;
            } else {
                return *(m_state.cur + offset);
            }
        }

        size_t TokenizerBase::line() const {
            return m_state.line;
        }

        size_t TokenizerBase::column() const {
            return m_state.column;
        }

        bool TokenizerBase::escaped() const {
            return !eof() && m_state.escaped && m_escapableChars.find(curChar()) != std::string::npos;
        }

        std::string TokenizerBase::unescape(const std::string& str) const {
            return kdl::str_unescape(str, m_escapableChars, m_escapeChar);
        }

        void TokenizerBase::resetEscaped() {
            m_state.escaped = false;
        }

        bool TokenizerBase::eof() const {
            return eof(m_state.cur);
        }

        bool TokenizerBase::eof(const char* ptr) const {
            return ptr >= m_end;
        }

        size_t TokenizerBase::offset(const char* ptr) const {
            assert(ptr >= m_begin);
            return static_cast<size_t>(ptr - m_begin);
        }

        void TokenizerBase::advance(const size_t offset) {
            for (size_t i = 0; i < offset; ++i) {
                advance();
            }
        }

        void TokenizerBase::advance() {
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

        void TokenizerBase::reset() {
            m_state.cur = m_begin;
            m_state.line = 1;
            m_state.column = 1;
            m_state.escaped = false;
        }

        void TokenizerBase::errorIfEof() const {
            if (eof()) {
                throw ParserException("Unexpected end of file");
            }
        }

        TokenizerState TokenizerBase::snapshot() const {
            return m_state;
        }

        void TokenizerBase::restore(const TokenizerState& snapshot) {
            m_state = snapshot;
        }
    }
}
