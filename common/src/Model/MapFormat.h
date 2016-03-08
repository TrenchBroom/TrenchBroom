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

#ifndef TrenchBroom_MapFormat
#define TrenchBroom_MapFormat

#include "StringUtils.h"

namespace TrenchBroom {
    namespace Model {
        namespace MapFormat {
            typedef size_t Type;
            static const Type Unknown  = 1 << 0;
            static const Type Standard = 1 << 1;
            static const Type Quake2   = 1 << 2;
            static const Type Valve    = 1 << 3;
            static const Type Hexen2   = 1 << 4;
        }
        
        MapFormat::Type mapFormat(const String& formatName);
        String formatName(MapFormat::Type format);
    }
}

#endif /* defined(TrenchBroom_MapFormat) */
