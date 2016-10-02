/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef Types_h
#define Types_h

#include "StringUtils.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace EL {
        class Value;
        
        typedef bool BooleanType;
        typedef String StringType;
        typedef double NumberType;
        typedef long IntegerType;
        typedef std::vector<Value> ArrayType;
        typedef std::map<String, Value> MapType;
        typedef std::vector<long> RangeType;
        
        typedef enum {
            Type_Boolean,
            Type_String,
            Type_Number,
            Type_Array,
            Type_Map,
            Type_Range,
            Type_Null,
            Type_Undefined
        } ValueType;
        
        String typeName(ValueType type);
        ValueType typeForName(const String& type);
    }
}

#endif /* Types_h */
