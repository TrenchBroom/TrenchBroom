/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Types.h"

namespace TrenchBroom {
    namespace EL {
        String typeName(const ValueType type) {
            switch (type) {
                case Type_Boolean:
                    return "Boolean";
                case Type_String:
                    return "String";
                case Type_Number:
                    return "Number";
                case Type_Array:
                    return "Array";
                case Type_Map:
                    return "Map";
                case Type_Range:
                    return "Range";
                case Type_Null:
                    return "Null";
                case Type_Undefined:
                    return "Undefined";
                    switchDefault()
            }
        }

        ValueType typeForName(const String& type) {
            if (type == "Boolean")
                return Type_Boolean;
            if (type == "String")
                return Type_String;
            if (type == "Number")
                return Type_Number;
            if (type == "Array")
                return Type_Array;
            if (type == "Map")
                return Type_Map;
            if (type == "Range")
                return Type_Range;
            if (type == "Undefined")
                return Type_Undefined;
            assert(false);
            return Type_Null;
        }
    }
}
