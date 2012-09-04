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
                String m_name;
                unsigned int m_size;
            public:
                FontDescriptor(const String& name, unsigned int size) :
                m_name(name),
                m_size(size) {
                    assert(size > 0);
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
