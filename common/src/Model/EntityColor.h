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

#ifndef __TrenchBroom__EntityColor__
#define __TrenchBroom__EntityColor__

#include "StringUtils.h"
#include "Model/ModelTypes.h"

#include <wx/colour.h>

namespace TrenchBroom {
    namespace Model {
        namespace ColorRange {
            typedef int Type;
            static const Type Unset = 0;
            static const Type Float = 1;
            static const Type Byte  = 2;
            static const Type Mixed = Float | Byte;
        }
        
        ColorRange::Type detectColorRange(const AttributeName& name, const AttributableNodeList& attributables);
        ColorRange::Type detectColorRange(const String& str);
        
        const String convertEntityColor(const String& str, ColorRange::Type colorRange);
        wxColor parseEntityColor(const String& str);
        String entityColorAsString(const wxColor& color, ColorRange::Type colorRange);
    }
}

#endif /* defined(__TrenchBroom__EntityColor__) */
