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

#include <gtest/gtest.h>

#include "EL.h"
#include "CollectionUtils.h"
#include "MathUtils.h"

#include <limits>

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
        
        TEST(ELTest, typeConversions) {
            ASSERT_EQ(Value(true), Value(true).convertTo(Type_Boolean));
            ASSERT_EQ(Value(false), Value(false).convertTo(Type_Boolean));
            ASSERT_EQ(Value("true"), Value(true).convertTo(Type_String));
            ASSERT_EQ(Value("false"), Value(false).convertTo(Type_String));
            ASSERT_EQ(Value(1), Value(true).convertTo(Type_Number));
            ASSERT_EQ(Value(0), Value(false).convertTo(Type_Number));
            ASSERT_THROW(Value(true).convertTo(Type_Array), ConversionError);
            ASSERT_THROW(Value(false).convertTo(Type_Array), ConversionError);
            ASSERT_THROW(Value(true).convertTo(Type_Map), ConversionError);
            ASSERT_THROW(Value(false).convertTo(Type_Map), ConversionError);
            ASSERT_THROW(Value(true).convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value(false).convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value(true).convertTo(Type_Null), ConversionError);
            ASSERT_THROW(Value(false).convertTo(Type_Null), ConversionError);
            ASSERT_THROW(Value(true).convertTo(Type_Undefined), ConversionError);
            ASSERT_THROW(Value(false).convertTo(Type_Undefined), ConversionError);
            
            ASSERT_EQ(Value(true), Value("asdf").convertTo(Type_Boolean));
            ASSERT_EQ(Value(false), Value("false").convertTo(Type_Boolean));
            ASSERT_EQ(Value(false), Value("").convertTo(Type_Boolean));
            ASSERT_EQ(Value("asdf"), Value("asdf").convertTo(Type_String));
            ASSERT_EQ(Value(2), Value("2").convertTo(Type_Number));
            ASSERT_EQ(Value(-2), Value("-2.0").convertTo(Type_Number));
            ASSERT_THROW(Value("asdf").convertTo(Type_Number), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(Type_Array), ConversionError);
            ASSERT_THROW(Value("asfd").convertTo(Type_Map), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(Type_Null), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(Type_Undefined), ConversionError);
            
            ASSERT_EQ(Value(true), Value(1).convertTo(Type_Boolean));
            ASSERT_EQ(Value(true), Value(2).convertTo(Type_Boolean));
            ASSERT_EQ(Value(true), Value(-2).convertTo(Type_Boolean));
            ASSERT_EQ(Value(false), Value(0).convertTo(Type_Boolean));
            ASSERT_EQ(Value("1"), Value(1.0).convertTo(Type_String));
            ASSERT_EQ(Value("-1"), Value(-1.0).convertTo(Type_String));
            ASSERT_EQ(Value("1.1000000000000001"), Value(1.1).convertTo(Type_String));
            ASSERT_EQ(Value("-1.1000000000000001"), Value(-1.1).convertTo(Type_String));
            ASSERT_EQ(Value(1), Value(1.0).convertTo(Type_Number));
            ASSERT_EQ(Value(-1), Value(-1.0).convertTo(Type_Number));
            ASSERT_THROW(Value(1).convertTo(Type_Array), ConversionError);
            ASSERT_THROW(Value(2).convertTo(Type_Map), ConversionError);
            ASSERT_THROW(Value(3).convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value(4).convertTo(Type_Null), ConversionError);
            ASSERT_THROW(Value(5).convertTo(Type_Undefined), ConversionError);
            
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_Boolean), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_String), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_Number), ConversionError);
            ASSERT_EQ(Value(ArrayType()), Value(ArrayType()).convertTo(Type_Array));
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_Map), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_Null), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(Type_Undefined), ConversionError);
            
            ASSERT_THROW(Value(MapType()).convertTo(Type_Boolean), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(Type_String), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(Type_Number), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(Type_Array), ConversionError);
            ASSERT_EQ(Value(MapType()), Value(MapType()).convertTo(Type_Map));
            ASSERT_THROW(Value(MapType()).convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(Type_Null), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(Type_Undefined), ConversionError);
            
            ASSERT_EQ(Value(false), Value::Null.convertTo(Type_Boolean));
            ASSERT_EQ(Value(""), Value::Null.convertTo(Type_String));
            ASSERT_EQ(Value(0), Value::Null.convertTo(Type_Number));
            ASSERT_EQ(Value(ArrayType()), Value::Null.convertTo(Type_Array));
            ASSERT_EQ(Value(MapType()), Value::Null.convertTo(Type_Map));
            ASSERT_THROW(Value::Null.convertTo(Type_Range), ConversionError);
            ASSERT_EQ(Value::Null, Value::Null.convertTo(Type_Null));
            ASSERT_THROW(Value::Null.convertTo(Type_Undefined), ConversionError);
            
            ASSERT_THROW(Value::Undefined.convertTo(Type_Boolean), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(Type_String), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(Type_Number), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(Type_Array), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(Type_Map), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(Type_Range), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(Type_Null), ConversionError);
            ASSERT_EQ(Value::Undefined, Value::Undefined.convertTo(Type_Undefined));
        }
        
        TEST(ELTest, serializeValues) {
            ASSERT_EQ(String("16"), Value(16.0).asString());
        }
        
        TEST(ELTest, subscriptOperator) {
            ASSERT_THROW(Value(true)[Value(0)], EvaluationError);
            ASSERT_THROW(Value(1.0)[Value(0)], EvaluationError);
            ASSERT_THROW(Value()[Value(0)], EvaluationError);
            
            ASSERT_EQ(Value("t"), Value("test")[Value(0)]);
            ASSERT_EQ(Value("e"), Value("test")[Value(1)]);
            ASSERT_EQ(Value("s"), Value("test")[Value(2)]);
            ASSERT_EQ(Value("t"), Value("test")[Value(3)]);
            ASSERT_EQ(Value("s"), Value("test")[Value(-2)]);
            ASSERT_EQ(Value(""), Value("test")[Value(4)]);
            

            ASSERT_EQ(Value("e"), Value("test")[Value(VectorUtils::create<Value>(Value(1)))]);
            ASSERT_EQ(Value("te"), Value("test")[Value(VectorUtils::create<Value>(Value(0), Value(1)))]);
            ASSERT_EQ(Value("es"), Value("test")[Value(VectorUtils::create<Value>(Value(1), Value(2)))]);
            ASSERT_EQ(Value("tt"), Value("test")[Value(VectorUtils::create<Value>(Value(0), Value(3)))]);
            ASSERT_EQ(Value("test"), Value("test")[Value(VectorUtils::create<Value>(Value(0), Value(1), Value(2), Value(3)))]);
            ASSERT_EQ(Value(""), Value("test")[Value(VectorUtils::create<Value>(Value(4)))]);
            ASSERT_EQ(Value("t"), Value("test")[Value(VectorUtils::create<Value>(Value(0), Value(4)))]);

            
            const Value arrayValue(VectorUtils::create<Value>(Value(1.0), Value("test")));
            
            ASSERT_EQ(Value(1.0), arrayValue[Value(0)]);
            ASSERT_EQ(Value("test"), arrayValue[Value(1)]);
            ASSERT_EQ(Value("test"), arrayValue[Value(-1)]);
            ASSERT_EQ(Value(1.0), arrayValue[Value(-2)]);
            
            ASSERT_THROW(arrayValue[Value(2)], EvaluationError);
            ASSERT_THROW(arrayValue[Value(-3)], EvaluationError);
            ASSERT_THROW(arrayValue[Value("asdf")], EvaluationError);
            ASSERT_THROW(arrayValue[Value("")], EvaluationError);
            
            ASSERT_EQ(Value(VectorUtils::create<Value>(Value(1.0))), arrayValue[Value(VectorUtils::create<Value>(Value(0)))]);
            ASSERT_EQ(Value(VectorUtils::create<Value>(Value("test"))), arrayValue[Value(VectorUtils::create<Value>(Value(1)))]);
            ASSERT_EQ(Value(VectorUtils::create<Value>(Value(1.0), Value("test"))), arrayValue[Value(VectorUtils::create<Value>(Value(0), Value(1)))]);
            ASSERT_THROW(arrayValue[Value(VectorUtils::create<Value>(Value(2)))], EvaluationError);
            ASSERT_THROW(arrayValue[Value(VectorUtils::create<Value>(Value(1), Value(2)))], EvaluationError);
            ASSERT_THROW(arrayValue[Value(VectorUtils::create<Value>(Value("test")))], ConversionError);
            ASSERT_THROW(arrayValue[Value(VectorUtils::create<Value>(Value(0), Value("test")))], ConversionError);
            
            MapType map;
            map["test"] = Value(1.0);
            map["huhu"] = Value("yeah");
            
            const Value mapValue(map);
            
            ASSERT_EQ(Value(1.0), mapValue[Value("test")]);
            ASSERT_EQ(Value("yeah"), mapValue[Value("huhu")]);
            ASSERT_EQ(Value::Undefined, mapValue[Value("huu")]);
            ASSERT_EQ(Value::Undefined, mapValue[Value("")]);
            
            MapType exp1;
            exp1["test"] = map["test"];

            MapType exp2;
            exp2["huhu"] = map["huhu"];
            
            ASSERT_EQ(Value(exp1), mapValue[Value(VectorUtils::create<Value>(Value("test")))]);
            ASSERT_EQ(Value(exp2), mapValue[Value(VectorUtils::create<Value>(Value("huhu")))]);
            ASSERT_EQ(Value(map), mapValue[Value(VectorUtils::create<Value>(Value("test"), Value("huhu")))]);
            ASSERT_EQ(Value(map), mapValue[Value(VectorUtils::create<Value>(Value("huhu"), Value("test")))]);
            ASSERT_EQ(Value(MapType()), mapValue[Value(VectorUtils::create<Value>(Value("asdf")))]);
            ASSERT_EQ(Value(exp1), mapValue[Value(VectorUtils::create<Value>(Value("test"), Value("asdf")))]);
            ASSERT_THROW(mapValue[Value(VectorUtils::create<Value>(Value(0)))], ConversionError);
            ASSERT_THROW(mapValue[Value(VectorUtils::create<Value>(Value("test"), Value(0)))], ConversionError);
        }
        
        TEST(ELTest, unaryPlusOperator) {
            ASSERT_THROW(+Value("test"), EvaluationError);
            ASSERT_THROW(+Value(ArrayType()), EvaluationError);
            ASSERT_THROW(+Value(MapType()), EvaluationError);
            
            ASSERT_EQ(Value(1.0), +Value(1.0));
            ASSERT_EQ(Value(1.0), +Value(true));
            ASSERT_EQ(Value(0.0), +Value(false));
        }
        
        TEST(ELTest, unaryMinusOperator) {
            ASSERT_THROW(-Value("test"), EvaluationError);
            ASSERT_THROW(-Value(ArrayType()), EvaluationError);
            ASSERT_THROW(-Value(MapType()), EvaluationError);
            
            ASSERT_EQ(Value(-1.0), -Value(1.0));
            ASSERT_EQ(Value(-1.0), -Value(true));
            ASSERT_EQ(Value( 0.0), -Value(false));
        }
        
        TEST(ELTest, binaryPlusOperator) {
            ASSERT_EQ(Value(2.0),           Value(true)     + Value(true));
            ASSERT_EQ(Value(3.0),           Value(false)    + Value(3.0));
            ASSERT_THROW(Value(true) + Value("test"),       EvaluationError);
            ASSERT_THROW(Value(true) + Value::Null,         EvaluationError);
            ASSERT_THROW(Value(true) + Value(ArrayType()),  EvaluationError);
            ASSERT_THROW(Value(true) + Value(MapType()),    EvaluationError);

            ASSERT_EQ(Value(2.0),           Value(1.0)      + Value(true));
            ASSERT_EQ(Value(2.0),           Value(3.0)      + Value(-1.0));
            ASSERT_THROW(Value(1.0) + Value("test"),        EvaluationError);
            ASSERT_THROW(Value(1.0) + Value::Null,          EvaluationError);
            ASSERT_THROW(Value(1.0) + Value(ArrayType()),   EvaluationError);
            ASSERT_THROW(Value(1.0) + Value(MapType()),     EvaluationError);
            
            ASSERT_THROW(Value("tst") + Value(true),        EvaluationError);
            ASSERT_THROW(Value("tst") + Value(2.0),         EvaluationError);
            ASSERT_EQ(Value("tsttest"),     Value("tst")    + Value("test"));
            ASSERT_THROW(Value("tst") + Value::Null,        EvaluationError);
            ASSERT_THROW(Value("tst") + Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value("tst") + Value(MapType()),   EvaluationError);

            ASSERT_EQ(Value(VectorUtils::create<Value>(Value(1), Value(2), Value(2), Value(3))),
                      Value(VectorUtils::create<Value>(Value(1), Value(2))) +
                      Value(VectorUtils::create<Value>(Value(2), Value(3))));
            
            ASSERT_THROW(Value(ArrayType()) + Value(true),          EvaluationError);
            ASSERT_THROW(Value(ArrayType()) + Value(1.0),           EvaluationError);
            ASSERT_THROW(Value(ArrayType()) + Value("test"),        EvaluationError);
            ASSERT_THROW(Value(ArrayType()) + Value::Null,          EvaluationError);
            ASSERT_THROW(Value(ArrayType()) + Value(MapType()),     EvaluationError);
            
            MapType map1;
            map1["k1"] = Value(1);
            map1["k2"] = Value(2);
            map1["k3"] = Value(3);
            
            MapType map2;
            map2["k3"] = Value(4);
            map2["k4"] = Value(5);
            
            MapType map3;
            map3["k1"] = Value(1);
            map3["k2"] = Value(2);
            map3["k3"] = Value(4);
            map3["k4"] = Value(5);
            
            ASSERT_EQ(Value(map3), Value(map1) + Value(map2));
            
            ASSERT_THROW(Value(MapType()) + Value(true),          EvaluationError);
            ASSERT_THROW(Value(MapType()) + Value(1.0),           EvaluationError);
            ASSERT_THROW(Value(MapType()) + Value("test"),        EvaluationError);
            ASSERT_THROW(Value(MapType()) + Value::Null,          EvaluationError);
            ASSERT_THROW(Value(MapType()) + Value(ArrayType()),   EvaluationError);
        }
        
        TEST(ELTest, binaryMinusOperator) {
            ASSERT_EQ(Value(0.0),           Value(true)     - Value(true));
            ASSERT_EQ(Value(-3.0),          Value(false)    - Value(3.0));
            ASSERT_THROW(Value(true) - Value("test"),       EvaluationError);
            ASSERT_THROW(Value(true) - Value::Null,         EvaluationError);
            ASSERT_THROW(Value(true) - Value(ArrayType()),  EvaluationError);
            ASSERT_THROW(Value(true) - Value(MapType()),    EvaluationError);
            
            ASSERT_EQ(Value(1.0),           Value(2.0)      - Value(true));
            ASSERT_EQ(Value(-1.0),          Value(2.0)      - Value(3.0));
            ASSERT_THROW(Value(1.0) - Value("test"),        EvaluationError);
            ASSERT_THROW(Value(1.0) - Value::Null,          EvaluationError);
            ASSERT_THROW(Value(1.0) - Value(ArrayType()),   EvaluationError);
            ASSERT_THROW(Value(1.0) - Value(MapType()),     EvaluationError);
            
            ASSERT_THROW(Value("test") - Value(true),           EvaluationError);
            ASSERT_THROW(Value("test") - Value(1.0),            EvaluationError);
            ASSERT_THROW(Value("test") - Value("test"),         EvaluationError);
            ASSERT_THROW(Value("test") - Value::Null,           EvaluationError);
            ASSERT_THROW(Value("test") - Value(ArrayType()),    EvaluationError);
            ASSERT_THROW(Value("test") - Value(MapType()),      EvaluationError);
            
            ASSERT_THROW(Value(ArrayType()) - Value(true),           EvaluationError);
            ASSERT_THROW(Value(ArrayType()) - Value(1.0),            EvaluationError);
            ASSERT_THROW(Value(ArrayType()) - Value("test"),         EvaluationError);
            ASSERT_THROW(Value(ArrayType()) - Value::Null,           EvaluationError);
            ASSERT_THROW(Value(ArrayType()) - Value(ArrayType()),    EvaluationError);
            ASSERT_THROW(Value(ArrayType()) - Value(MapType()),      EvaluationError);
            
            ASSERT_THROW(Value(MapType()) - Value(true),           EvaluationError);
            ASSERT_THROW(Value(MapType()) - Value(1.0),            EvaluationError);
            ASSERT_THROW(Value(MapType()) - Value("test"),         EvaluationError);
            ASSERT_THROW(Value(MapType()) - Value::Null,           EvaluationError);
            ASSERT_THROW(Value(MapType()) - Value(ArrayType()),    EvaluationError);
            ASSERT_THROW(Value(MapType()) - Value(MapType()),      EvaluationError);
        }
        
        TEST(ELTest, binaryTimesOperator) {
            ASSERT_EQ(Value(0.0), Value(true) * Value(false));
            ASSERT_EQ(Value(1.0), Value(true) * Value(true));
            ASSERT_EQ(Value(-2.0), Value(true) * Value(-2.0));
            ASSERT_THROW(Value(true) * Value("test"), EvaluationError);
            ASSERT_THROW(Value(true) * Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(true) * Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(true) * Value::Null, EvaluationError);
            
            ASSERT_EQ(Value(0.0), Value(2.0) * Value(false));
            ASSERT_EQ(Value(2.0), Value(2.0) * Value(true));
            ASSERT_EQ(Value(-6.0), Value(3.0) * Value(-2.0));
            ASSERT_THROW(Value(1.0) * Value("test"), EvaluationError);
            ASSERT_THROW(Value(1.0) * Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(1.0) * Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(1.0) * Value::Null, EvaluationError);

            ASSERT_THROW(Value("test") * Value(true), EvaluationError);
            ASSERT_THROW(Value("test") * Value(1.0), EvaluationError);
            ASSERT_THROW(Value("test") * Value("test"), EvaluationError);
            ASSERT_THROW(Value("test") * Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value("test") * Value(MapType()), EvaluationError);
            ASSERT_THROW(Value("test") * Value::Null, EvaluationError);
            
            ASSERT_THROW(Value(ArrayType()) * Value(true), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) * Value(1.0), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) * Value("test"), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) * Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) * Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) * Value::Null, EvaluationError);
            
            ASSERT_THROW(Value(MapType()) * Value(true), EvaluationError);
            ASSERT_THROW(Value(MapType()) * Value(1.0), EvaluationError);
            ASSERT_THROW(Value(MapType()) * Value("test"), EvaluationError);
            ASSERT_THROW(Value(MapType()) * Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(MapType()) * Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(MapType()) * Value::Null, EvaluationError);
        }
        
        TEST(ELTest, binaryOverOperator) {
            ASSERT_EQ(Value(std::numeric_limits<NumberType>::infinity()), Value(true) / Value(false));
            ASSERT_EQ(Value(1.0), Value(true) / Value(true));
            ASSERT_EQ(Value(-0.5), Value(true) / Value(-2.0));
            ASSERT_THROW(Value(true) / Value("test"), EvaluationError);
            ASSERT_THROW(Value(true) / Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(true) / Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(true) / Value::Null, EvaluationError);
            
            ASSERT_EQ(Value(-std::numeric_limits<NumberType>::infinity()), Value(-2.0) / Value(false));
            ASSERT_EQ(Value(2.0), Value(2.0) / Value(true));
            ASSERT_EQ(Value(-1.5), Value(3.0) / Value(-2.0));
            ASSERT_THROW(Value(1.0) / Value("test"), EvaluationError);
            ASSERT_THROW(Value(1.0) / Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(1.0) / Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(1.0) / Value::Null, EvaluationError);
            
            ASSERT_THROW(Value("test") / Value(true), EvaluationError);
            ASSERT_THROW(Value("test") / Value(1.0), EvaluationError);
            ASSERT_THROW(Value("test") / Value("test"), EvaluationError);
            ASSERT_THROW(Value("test") / Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value("test") / Value(MapType()), EvaluationError);
            ASSERT_THROW(Value("test") / Value::Null, EvaluationError);
            
            ASSERT_THROW(Value(ArrayType()) / Value(true), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) / Value(1.0), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) / Value("test"), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) / Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) / Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) / Value::Null, EvaluationError);
            
            ASSERT_THROW(Value(MapType()) / Value(true), EvaluationError);
            ASSERT_THROW(Value(MapType()) / Value(1.0), EvaluationError);
            ASSERT_THROW(Value(MapType()) / Value("test"), EvaluationError);
            ASSERT_THROW(Value(MapType()) / Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(MapType()) / Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(MapType()) / Value::Null, EvaluationError);
        }
        
        TEST(ELTest, binaryModulusOperator) {
            ASSERT_TRUE(Math::isnan((Value(true) % Value(false)).numberValue()));
            ASSERT_EQ(Value(0.0), Value(true) % Value(true));
            ASSERT_EQ(Value(1.0), Value(true) % Value(-2.0));
            ASSERT_THROW(Value(true) % Value("test"), EvaluationError);
            ASSERT_THROW(Value(true) % Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(true) % Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(true) % Value::Null, EvaluationError);
            
            ASSERT_TRUE(Math::isnan((Value(-2.0) % Value(false)).numberValue()));
            ASSERT_EQ(Value(0.0), Value(2.0) % Value(true));
            ASSERT_EQ(Value(1.0), Value(3.0) % Value(-2.0));
            ASSERT_THROW(Value(1.0) % Value("test"), EvaluationError);
            ASSERT_THROW(Value(1.0) % Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(1.0) % Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(1.0) % Value::Null, EvaluationError);
            
            ASSERT_THROW(Value("test") % Value(true), EvaluationError);
            ASSERT_THROW(Value("test") % Value(1.0), EvaluationError);
            ASSERT_THROW(Value("test") % Value("test"), EvaluationError);
            ASSERT_THROW(Value("test") % Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value("test") % Value(MapType()), EvaluationError);
            ASSERT_THROW(Value("test") % Value::Null, EvaluationError);
            
            ASSERT_THROW(Value(ArrayType()) % Value(true), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) % Value(1.0), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) % Value("test"), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) % Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) % Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(ArrayType()) % Value::Null, EvaluationError);
            
            ASSERT_THROW(Value(MapType()) % Value(true), EvaluationError);
            ASSERT_THROW(Value(MapType()) % Value(1.0), EvaluationError);
            ASSERT_THROW(Value(MapType()) % Value("test"), EvaluationError);
            ASSERT_THROW(Value(MapType()) % Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(MapType()) % Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(MapType()) % Value::Null, EvaluationError);
        }
    }
}
