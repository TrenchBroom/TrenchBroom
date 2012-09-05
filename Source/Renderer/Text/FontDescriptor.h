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

#ifndef __TrenchBroom__FontDescriptor__
#define __TrenchBroom__FontDescriptor__

#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class FontDescriptor {
            private:
                unsigned long m_nameHash;
                String m_name;
                unsigned int m_size;
            public:
                FontDescriptor(const String& name, unsigned int size) :
                m_name(name),
                m_size(size) {
                    assert(size > 0);
                    m_nameHash = Utility::makeHash(m_name);
                }
                
                inline int compare(const FontDescriptor& other) const {
                    if (m_nameHash < other.nameHash())
                        return -1;
                    if (m_nameHash > other.nameHash())
                        return +1;
                    
                    // hashes are the same, but it might be a collision
                    int strOrder = m_name.compare(other.name());
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
                
                inline bool operator< (const FontDescriptor& other) const {
                    return compare(other) < 0;
                }

                inline const unsigned long nameHash() const {
                    return m_nameHash;
                }
                
                inline const String& name() const {
                    return m_name;
                }
                
                inline unsigned int size() const {
                    return m_size;
                }
            };
        }
    }
}

#endif /* defined(__TrenchBroom__FontDescriptor__) */
