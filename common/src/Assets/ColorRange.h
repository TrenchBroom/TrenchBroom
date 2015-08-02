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

#ifndef TrenchBroom_ColorRange_h
#define TrenchBroom_ColorRange_h

#include "StringUtils.h"

namespace TrenchBroom {
    namespace Assets {
        namespace ColorRange {
            typedef int Type;
            static const Type Unset = 0;
            static const Type Float = 1;
            static const Type Byte  = 2;
            static const Type Mixed = Float | Byte;
        }

        ColorRange::Type detectColorRange(const String& str);
        ColorRange::Type detectColorRange(const StringList& components);
    }
}

#endif
