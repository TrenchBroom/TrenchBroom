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

#include "EL/ELExceptions.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include <vecmath/scalar.h>

#include <limits>
#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace EL {
        TEST_CASE("ELTest.constructValues", "[ELTest]") {
            ASSERT_EQ(ValueType::Boolean, Value(true).type());
            ASSERT_EQ(ValueType::Boolean, Value(false).type());
            ASSERT_EQ(ValueType::String,  Value("test").type());
            ASSERT_EQ(ValueType::Number,  Value(1.0).type());
            ASSERT_EQ(ValueType::Array,   Value(ArrayType()).type());
            ASSERT_EQ(ValueType::Map,     Value(MapType()).type());
            ASSERT_EQ(ValueType::Null,    Value().type());
        }

        TEST_CASE("ELTest.typeConversions", "[ELTest]") {
            ASSERT_EQ(Value(true), Value(true).convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(false), Value(false).convertTo(ValueType::Boolean));
            ASSERT_EQ(Value("true"), Value(true).convertTo(ValueType::String));
            ASSERT_EQ(Value("false"), Value(false).convertTo(ValueType::String));
            ASSERT_EQ(Value(1), Value(true).convertTo(ValueType::Number));
            ASSERT_EQ(Value(0), Value(false).convertTo(ValueType::Number));
            ASSERT_THROW(Value(true).convertTo(ValueType::Array), ConversionError);
            ASSERT_THROW(Value(false).convertTo(ValueType::Array), ConversionError);
            ASSERT_THROW(Value(true).convertTo(ValueType::Map), ConversionError);
            ASSERT_THROW(Value(false).convertTo(ValueType::Map), ConversionError);
            ASSERT_THROW(Value(true).convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value(false).convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value(true).convertTo(ValueType::Null), ConversionError);
            ASSERT_THROW(Value(false).convertTo(ValueType::Null), ConversionError);
            ASSERT_THROW(Value(true).convertTo(ValueType::Undefined), ConversionError);
            ASSERT_THROW(Value(false).convertTo(ValueType::Undefined), ConversionError);

            ASSERT_EQ(Value(true), Value("asdf").convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(false), Value("false").convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(false), Value("").convertTo(ValueType::Boolean));
            ASSERT_EQ(Value("asdf"), Value("asdf").convertTo(ValueType::String));
            ASSERT_EQ(Value(2), Value("2").convertTo(ValueType::Number));
            ASSERT_EQ(Value(-2), Value("-2.0").convertTo(ValueType::Number));
            ASSERT_THROW(Value("asdf").convertTo(ValueType::Number), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(ValueType::Array), ConversionError);
            ASSERT_THROW(Value("asfd").convertTo(ValueType::Map), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(ValueType::Null), ConversionError);
            ASSERT_THROW(Value("asdf").convertTo(ValueType::Undefined), ConversionError);

            ASSERT_EQ(Value(true), Value(1).convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(true), Value(2).convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(true), Value(-2).convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(false), Value(0).convertTo(ValueType::Boolean));
            ASSERT_EQ(Value("1"), Value(1.0).convertTo(ValueType::String));
            ASSERT_EQ(Value("-1"), Value(-1.0).convertTo(ValueType::String));
            ASSERT_EQ(Value("1.1000000000000001"), Value(1.1).convertTo(ValueType::String));
            ASSERT_EQ(Value("-1.1000000000000001"), Value(-1.1).convertTo(ValueType::String));
            ASSERT_EQ(Value(1), Value(1.0).convertTo(ValueType::Number));
            ASSERT_EQ(Value(-1), Value(-1.0).convertTo(ValueType::Number));
            ASSERT_THROW(Value(1).convertTo(ValueType::Array), ConversionError);
            ASSERT_THROW(Value(2).convertTo(ValueType::Map), ConversionError);
            ASSERT_THROW(Value(3).convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value(4).convertTo(ValueType::Null), ConversionError);
            ASSERT_THROW(Value(5).convertTo(ValueType::Undefined), ConversionError);

            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::Boolean), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::String), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::Number), ConversionError);
            ASSERT_EQ(Value(ArrayType()), Value(ArrayType()).convertTo(ValueType::Array));
            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::Map), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::Null), ConversionError);
            ASSERT_THROW(Value(ArrayType()).convertTo(ValueType::Undefined), ConversionError);

            ASSERT_THROW(Value(MapType()).convertTo(ValueType::Boolean), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(ValueType::String), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(ValueType::Number), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(ValueType::Array), ConversionError);
            ASSERT_EQ(Value(MapType()), Value(MapType()).convertTo(ValueType::Map));
            ASSERT_THROW(Value(MapType()).convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(ValueType::Null), ConversionError);
            ASSERT_THROW(Value(MapType()).convertTo(ValueType::Undefined), ConversionError);

            ASSERT_EQ(Value(false), Value::Null.convertTo(ValueType::Boolean));
            ASSERT_EQ(Value(""), Value::Null.convertTo(ValueType::String));
            ASSERT_EQ(Value(0), Value::Null.convertTo(ValueType::Number));
            ASSERT_EQ(Value(ArrayType()), Value::Null.convertTo(ValueType::Array));
            ASSERT_EQ(Value(MapType()), Value::Null.convertTo(ValueType::Map));
            ASSERT_THROW(Value::Null.convertTo(ValueType::Range), ConversionError);
            ASSERT_EQ(Value::Null, Value::Null.convertTo(ValueType::Null));
            ASSERT_THROW(Value::Null.convertTo(ValueType::Undefined), ConversionError);

            ASSERT_THROW(Value::Undefined.convertTo(ValueType::Boolean), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(ValueType::String), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(ValueType::Number), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(ValueType::Array), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(ValueType::Map), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(ValueType::Range), ConversionError);
            ASSERT_THROW(Value::Undefined.convertTo(ValueType::Null), ConversionError);
            ASSERT_EQ(Value::Undefined, Value::Undefined.convertTo(ValueType::Undefined));
        }

        TEST_CASE("ELTest.serializeValues", "[ELTest]") {
            ASSERT_EQ(std::string("16"), Value(16.0).asString());
        }

        TEST_CASE("ELTest.subscriptOperator", "[ELTest]") {
            ASSERT_THROW(Value(true)[Value(0)], EvaluationError);
            ASSERT_THROW(Value(1.0)[Value(0)], EvaluationError);
            ASSERT_THROW(Value()[Value(0)], EvaluationError);

            ASSERT_EQ(Value("t"), Value("test")[Value(0)]);
            ASSERT_EQ(Value("e"), Value("test")[Value(1)]);
            ASSERT_EQ(Value("s"), Value("test")[Value(2)]);
            ASSERT_EQ(Value("t"), Value("test")[Value(3)]);
            ASSERT_EQ(Value("s"), Value("test")[Value(-2)]);
            ASSERT_EQ(Value(""), Value("test")[Value(4)]);


            ASSERT_EQ(Value("e"), Value("test")[Value(ArrayType({ Value(1) }))]);
            ASSERT_EQ(Value("te"), Value("test")[Value(ArrayType({ Value(0), Value(1) }))]);
            ASSERT_EQ(Value("es"), Value("test")[Value(ArrayType({ Value(1), Value(2) }))]);
            ASSERT_EQ(Value("tt"), Value("test")[Value(ArrayType({ Value(0), Value(3) }))]);
            ASSERT_EQ(Value("test"), Value("test")[Value(ArrayType({ Value(0), Value(1), Value(2), Value(3) }))]);
            ASSERT_EQ(Value(""), Value("test")[Value(ArrayType({ Value(4) }))]);
            ASSERT_EQ(Value("t"), Value("test")[Value(ArrayType({ Value(0), Value(4) }))]);


            const Value arrayValue(ArrayType({ Value(1.0), Value("test") }));

            ASSERT_EQ(Value(1.0), arrayValue[Value(0)]);
            ASSERT_EQ(Value("test"), arrayValue[Value(1)]);
            ASSERT_EQ(Value("test"), arrayValue[Value(-1)]);
            ASSERT_EQ(Value(1.0), arrayValue[Value(-2)]);

            ASSERT_THROW(arrayValue[Value(2)], EvaluationError);
            ASSERT_THROW(arrayValue[Value(-3)], EvaluationError);
            ASSERT_THROW(arrayValue[Value("asdf")], EvaluationError);
            ASSERT_THROW(arrayValue[Value("")], EvaluationError);

            ASSERT_EQ(Value(ArrayType({ Value(1.0)} )), arrayValue[Value(ArrayType({ Value(0) }))]);
            ASSERT_EQ(Value(ArrayType({ Value("test") })), arrayValue[Value(ArrayType({ Value(1) }))]);
            ASSERT_EQ(Value(ArrayType({ Value(1.0), Value("test") })), arrayValue[Value(ArrayType({ Value(0), Value(1) }))]);
            ASSERT_THROW(arrayValue[Value(ArrayType({ Value(2) }))], EvaluationError);
            ASSERT_THROW(arrayValue[Value(ArrayType({ Value(1), Value(2) }))], EvaluationError);
            ASSERT_THROW(arrayValue[Value(ArrayType({ Value("test") }))], ConversionError);
            ASSERT_THROW(arrayValue[Value(ArrayType({ Value(0), Value("test") }))], ConversionError);

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

            ASSERT_EQ(Value(exp1), mapValue[Value(ArrayType({ Value("test") }))]);
            ASSERT_EQ(Value(exp2), mapValue[Value(ArrayType({ Value("huhu") }))]);
            ASSERT_EQ(Value(map), mapValue[Value(ArrayType({ Value("test"), Value("huhu") }))]);
            ASSERT_EQ(Value(map), mapValue[Value(ArrayType({ Value("huhu"), Value("test") }))]);
            ASSERT_EQ(Value(MapType()), mapValue[Value(ArrayType({ Value("asdf") }))]);
            ASSERT_EQ(Value(exp1), mapValue[Value(ArrayType({ Value("test"), Value("asdf") }))]);
            ASSERT_THROW(mapValue[Value(ArrayType({ Value(0) }))], ConversionError);
            ASSERT_THROW(mapValue[Value(ArrayType({ Value("test"), Value(0) }))], ConversionError);
        }

        TEST_CASE("ELTest.unaryPlusOperator", "[ELTest]") {
            ASSERT_THROW(+Value("test"), EvaluationError);
            ASSERT_THROW(+Value(ArrayType()), EvaluationError);
            ASSERT_THROW(+Value(MapType()), EvaluationError);

            ASSERT_EQ(Value(1.0), +Value(1.0));
            ASSERT_EQ(Value(1.0), +Value(true));
            ASSERT_EQ(Value(0.0), +Value(false));
        }

        TEST_CASE("ELTest.unaryMinusOperator", "[ELTest]") {
            ASSERT_THROW(-Value("test"), EvaluationError);
            ASSERT_THROW(-Value(ArrayType()), EvaluationError);
            ASSERT_THROW(-Value(MapType()), EvaluationError);

            ASSERT_EQ(Value(-1.0), -Value(1.0));
            ASSERT_EQ(Value(-1.0), -Value(true));
            ASSERT_EQ(Value( 0.0), -Value(false));
        }

        TEST_CASE("ELTest.binaryPlusOperator", "[ELTest]") {
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

            ASSERT_EQ(Value(ArrayType({ Value(1), Value(2), Value(2), Value(3) })),
                      Value(ArrayType({ Value(1), Value(2) })) +
                      Value(ArrayType({ Value(2), Value(3) })));

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

        TEST_CASE("ELTest.binaryMinusOperator", "[ELTest]") {
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

        TEST_CASE("ELTest.binaryTimesOperator", "[ELTest]") {
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

        TEST_CASE("ELTest.binaryOverOperator", "[ELTest]") {
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

        TEST_CASE("ELTest.binaryModulusOperator", "[ELTest]") {
            ASSERT_TRUE(vm::is_nan((Value(true) % Value(false)).numberValue()));
            ASSERT_EQ(Value(0.0), Value(true) % Value(true));
            ASSERT_EQ(Value(1.0), Value(true) % Value(-2.0));
            ASSERT_THROW(Value(true) % Value("test"), EvaluationError);
            ASSERT_THROW(Value(true) % Value(ArrayType()), EvaluationError);
            ASSERT_THROW(Value(true) % Value(MapType()), EvaluationError);
            ASSERT_THROW(Value(true) % Value::Null, EvaluationError);

            ASSERT_TRUE(vm::is_nan((Value(-2.0) % Value(false)).numberValue()));
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
