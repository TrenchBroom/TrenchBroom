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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__GameConfig__
#define __TrenchBroom__GameConfig__

#include "StringUtils.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace Model {
        class GameConfig {
        public:
            struct TextureFormat {
                typedef enum {
                    TWad,
                    TWal,
                    TUnknown
                } Type;
                
                Type type;
                IO::Path palette;
                
                TextureFormat(const Type i_type, const IO::Path& i_palette = IO::Path(""));
            };
        private:
            String m_name;
            TextureFormat m_textureFormat;
            StringSet m_modelFormats;
        public:
            GameConfig(const String& name, const TextureFormat& textureFormat, const StringSet& modelFormats);
            
            const String& name() const;
            const TextureFormat& textureFormat() const;
            const StringSet& modelFormats() const;
            
            static TextureFormat::Type parseType(const String str);
        };
    }
}

#endif /* defined(__TrenchBroom__GameConfig__) */
