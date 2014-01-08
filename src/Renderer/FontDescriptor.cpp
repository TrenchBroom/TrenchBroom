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

namespace TrenchBroom {
    namespace Renderer {
        FontDescriptor::FontDescriptor(const String& name, const size_t size) :
        m_name(name),
        m_size(size),
        m_nameHash(StringUtils::makeHash(m_name)) {}
        
        int FontDescriptor::compare(const FontDescriptor& other) const {
            if (m_nameHash < other.nameHash())
                return -1;
            if (m_nameHash > other.nameHash())
                return +1;
            
            // hashes are the same, but it might be a collision
            const int strOrder = m_name.compare(other.name());
            if (strOrder < 0)
                return -1;
            if (strOrder > 0)
                return +1;
            
            // names are the same, use the size
            if (m_size < other.size())
                return -1;
            if (m_size > other.size())
                return +1;
            return 0;
        }
        
        bool FontDescriptor::operator< (const FontDescriptor& other) const {
            return compare(other) < 0;
        }
        
        long FontDescriptor::nameHash() const {
            return m_nameHash;
        }
        
        const String& FontDescriptor::name() const {
            return m_name;
        }
        
        size_t FontDescriptor::size() const {
            return m_size;
        }
    }
}
