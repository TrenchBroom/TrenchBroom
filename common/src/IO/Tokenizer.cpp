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


#include <string>

namespace TrenchBroom {
    namespace IO {
        TokenizerBase::TokenizerBase(const char* begin, const char* end, std::string_view escapableChars, const char escapeChar) :
        m_begin(begin),
        m_end(end),
        m_escapableChars(escapableChars),
        m_escapeChar(escapeChar),
        m_state{begin, 1, 1, false} {}
    }
}
