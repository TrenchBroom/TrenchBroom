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

#ifndef TrenchBroom_FontDescriptor
#define TrenchBroom_FontDescriptor

#include "IO/Path.h"

namespace TrenchBroom {
    namespace Renderer {
        class FontDescriptor {
        private:
            IO::Path m_path;
            size_t m_size;
            unsigned char m_minChar;
            unsigned char m_maxChar;
        public:
            FontDescriptor(const IO::Path& path, const size_t size, unsigned char minChar = ' ', unsigned char maxChar = '~');
            
            int compare(const FontDescriptor& other) const;
            bool operator<(const FontDescriptor& other) const;
            
            const IO::Path& path() const;
            String name() const;
            size_t size() const;
            unsigned char minChar() const;
            unsigned char maxChar() const;
            unsigned char charCount() const;
        };
    }
}

#endif /* defined(TrenchBroom_FontDescriptor) */
