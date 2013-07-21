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

#ifndef __TrenchBroom__FontDescriptor__
#define __TrenchBroom__FontDescriptor__

#include "StringUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        class FontDescriptor {
        private:
            String m_name;
            size_t m_size;
            long m_nameHash;
        public:
            FontDescriptor(const String& name, const size_t size);
            
            int compare(const FontDescriptor& other) const;
            bool operator< (const FontDescriptor& other) const;
            
            long nameHash() const;
            const String& name() const;
            size_t size() const;
        };
    }
}

#endif /* defined(__TrenchBroom__FontDescriptor__) */
