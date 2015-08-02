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

#include "ColorRange.h"

namespace TrenchBroom {
    namespace Assets {
        ColorRange::Type detectColorRange(const StringList& components);
        
        ColorRange::Type detectColorRange(const String& str) {
            return detectColorRange(StringUtils::splitAndTrim(str, " "));
        }
        
        ColorRange::Type detectColorRange(const StringList& components) {
            if (components.size() != 3)
                return ColorRange::Unset;
            
            ColorRange::Type range = ColorRange::Byte;
            for (size_t i = 0; i < 3 && range == ColorRange::Byte; ++i)
                if (components[i].find('.') != String::npos)
                    range = ColorRange::Float;
            
            return range;
        }
    }
}
