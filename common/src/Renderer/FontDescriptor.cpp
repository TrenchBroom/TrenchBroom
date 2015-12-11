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

#include "FontDescriptor.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        FontDescriptor::FontDescriptor(const IO::Path& path, const size_t size, const unsigned char minChar, const unsigned char maxChar) :
        m_path(path),
        m_size(size),
        m_minChar(minChar),
        m_maxChar(maxChar) {
            assert(m_minChar <= m_maxChar);
        }
        
        int FontDescriptor::compare(const FontDescriptor& other) const {
            if (m_size < other.m_size)
                return -1;
            if (m_size > other.m_size)
                return +1;
            if (m_minChar < other.m_minChar)
                return -1;
            if (m_minChar > other.m_minChar)
                return +1;
            if (m_maxChar < other.m_maxChar)
                return -1;
            if (m_maxChar > other.m_maxChar)
                return +1;
            return m_path.compare(other.m_path);
        }
        
        bool FontDescriptor::operator<(const FontDescriptor& other) const {
            return compare(other) < 0;
        }
        
        const IO::Path& FontDescriptor::path() const {
            return m_path;
        }
        
        String FontDescriptor::name() const {
            return m_path.lastComponent().deleteExtension().asString();
        }
        
        size_t FontDescriptor::size() const {
            return m_size;
        }

        unsigned char FontDescriptor::minChar() const {
            return m_minChar;
        }
        
        unsigned char FontDescriptor::maxChar() const {
            return m_maxChar;
        }

        unsigned char FontDescriptor::charCount() const {
            return m_maxChar - m_minChar + 1;
        }
    }
}
