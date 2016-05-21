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

#include <gtest/gtest.h>

#include "EL.h"

namespace TrenchBroom {
    namespace EL {
        TEST(ELTest, constructValues) {
            ASSERT_EQ(Type_Boolean, Value(true).type());
            ASSERT_EQ(Type_Boolean, Value(false).type());
            ASSERT_EQ(Type_String,  Value("test").type());
            ASSERT_EQ(Type_Number,  Value(1.0).type());
            ASSERT_EQ(Type_Array,   Value(ArrayType()).type());
            ASSERT_EQ(Type_Map,     Value(MapType()).type());
            ASSERT_EQ(Type_Null,    Value().type());
        }
        
        TEST(ELTest, subscriptOperator) {
            ASSERT_THROW(Value(true)[Value(0)], EvaluationError);
            ASSERT_THROW(Value("test")[Value(0)], EvaluationError);
            ASSERT_THROW(Value(1.0)[Value(0)], EvaluationError);
            ASSERT_THROW(Value()[Value(0)], EvaluationError);
            
            ArrayType array;
            array.push_back(Value(1.0));
            array.push_back(Value("test"));
            
            const Value arrayValue(array);
            
            ASSERT_EQ(Value(1.0), arrayValue[Value(0)]);
            ASSERT_EQ(Value("test"), arrayValue[Value(1)]);
            ASSERT_EQ(Value("test"), arrayValue[Value(-1)]);
            ASSERT_EQ(Value(1.0), arrayValue[Value(-2)]);
            
            ASSERT_THROW(arrayValue[Value(2)], EvaluationError);
            ASSERT_THROW(arrayValue[Value(-3)], EvaluationError);
            ASSERT_THROW(arrayValue[Value("asdf")], EvaluationError);
            ASSERT_THROW(arrayValue[Value("")], EvaluationError);
            
            
            MapType map;
            map["test"] = Value(1.0);
            map["huhu"] = Value("yeah");
            
            const Value mapValue(map);
            
            ASSERT_EQ(Value(1.0), mapValue[Value("test")]);
            ASSERT_EQ(Value("yeah"), mapValue[Value("huhu")]);
            ASSERT_EQ(Value::Null, mapValue[Value("huu")]);
            ASSERT_EQ(Value::Null, mapValue[Value("")]);
            ASSERT_EQ(Value::Null, mapValue[Value("huu")]);
        }
    }
}
