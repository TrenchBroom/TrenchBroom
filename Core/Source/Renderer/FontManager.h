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

#ifndef TrenchBroom_FontManager_h
#define TrenchBroom_FontManager_h

#include <string>
#include <map>
#include "FTGL/ftgl.h"
#include "Utilities/SharedPointer.h"

namespace TrenchBroom {
    namespace Renderer {
        class FontDescriptor {
        public:
            std::string name;
            int size;
            FontDescriptor() {}
            FontDescriptor(const std::string& name, int size) : name(name), size(size) {}
        };

        bool operator<(FontDescriptor const& left, FontDescriptor const& right);

        class FontManager {
        private:
            typedef std::map<FontDescriptor, FTGL::FTGLfont*> FontCache;
            FontCache m_fontCache;
        protected:
            virtual std::string resolveFont(const std::string& fontName) = 0;
        public:
            FontManager();
            virtual ~FontManager();
            FTGL::FTGLfont* font(const FontDescriptor& fontDescriptor);
            void clear();
        };
    }
}

#endif
