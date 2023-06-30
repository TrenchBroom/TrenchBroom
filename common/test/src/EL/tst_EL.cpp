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

namespace TrenchBroom
{
namespace EL
{
TEST_CASE("ELTest.constructValues")
{
  CHECK(Value(true).type() == ValueType::Boolean);
  CHECK(Value(false).type() == ValueType::Boolean);
  CHECK(Value("test").type() == ValueType::String);
  CHECK(Value(1.0).type() == ValueType::Number);
  CHECK(Value(ArrayType()).type() == ValueType::Array);
  CHECK(Value(MapType()).type() == ValueType::Map);
  CHECK(Value().type() == ValueType::Null);
}

TEST_CASE("ELTest.typeConversions")
{
  CHECK(Value(true).convertTo(ValueType::Boolean) == Value(true));
  CHECK(Value(false).convertTo(ValueType::Boolean) == Value(false));
  CHECK(Value(true).convertTo(ValueType::String) == Value("true"));
  CHECK(Value(false).convertTo(ValueType::String) == Value("false"));
  CHECK(Value(true).convertTo(ValueType::Number) == Value(1));
  CHECK(Value(false).convertTo(ValueType::Number) == Value(0));
  CHECK_THROWS_AS(Value(true).convertTo(ValueType::Array), ConversionError);
  CHECK_THROWS_AS(Value(false).convertTo(ValueType::Array), ConversionError);
  CHECK_THROWS_AS(Value(true).convertTo(ValueType::Map), ConversionError);
  CHECK_THROWS_AS(Value(false).convertTo(ValueType::Map), ConversionError);
  CHECK_THROWS_AS(Value(true).convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value(false).convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value(true).convertTo(ValueType::Null), ConversionError);
  CHECK_THROWS_AS(Value(false).convertTo(ValueType::Null), ConversionError);
  CHECK_THROWS_AS(Value(true).convertTo(ValueType::Undefined), ConversionError);
  CHECK_THROWS_AS(Value(false).convertTo(ValueType::Undefined), ConversionError);

  CHECK(Value("asdf").convertTo(ValueType::Boolean) == Value(true));
  CHECK(Value("false").convertTo(ValueType::Boolean) == Value(false));
  CHECK(Value("").convertTo(ValueType::Boolean) == Value(false));
  CHECK(Value("asdf").convertTo(ValueType::String) == Value("asdf"));
  CHECK(Value("2").convertTo(ValueType::Number) == Value(2));
  CHECK(Value("-2.0").convertTo(ValueType::Number) == Value(-2));
  CHECK_THROWS_AS(Value("asdf").convertTo(ValueType::Number), ConversionError);
  CHECK_THROWS_AS(Value("asdf").convertTo(ValueType::Array), ConversionError);
  CHECK_THROWS_AS(Value("asfd").convertTo(ValueType::Map), ConversionError);
  CHECK_THROWS_AS(Value("asdf").convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value("asdf").convertTo(ValueType::Null), ConversionError);
  CHECK_THROWS_AS(Value("asdf").convertTo(ValueType::Undefined), ConversionError);

  CHECK(Value(1).convertTo(ValueType::Boolean) == Value(true));
  CHECK(Value(2).convertTo(ValueType::Boolean) == Value(true));
  CHECK(Value(-2).convertTo(ValueType::Boolean) == Value(true));
  CHECK(Value(0).convertTo(ValueType::Boolean) == Value(false));
  CHECK(Value(1.0).convertTo(ValueType::String) == Value("1"));
  CHECK(Value(-1.0).convertTo(ValueType::String) == Value("-1"));
  CHECK(Value(1.1).convertTo(ValueType::String) == Value("1.1000000000000001"));
  CHECK(Value(-1.1).convertTo(ValueType::String) == Value("-1.1000000000000001"));
  CHECK(Value(1.0).convertTo(ValueType::Number) == Value(1));
  CHECK(Value(-1.0).convertTo(ValueType::Number) == Value(-1));
  CHECK_THROWS_AS(Value(1).convertTo(ValueType::Array), ConversionError);
  CHECK_THROWS_AS(Value(2).convertTo(ValueType::Map), ConversionError);
  CHECK_THROWS_AS(Value(3).convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value(4).convertTo(ValueType::Null), ConversionError);
  CHECK_THROWS_AS(Value(5).convertTo(ValueType::Undefined), ConversionError);

  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::Boolean), ConversionError);
  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::String), ConversionError);
  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::Number), ConversionError);
  CHECK(Value(ArrayType()).convertTo(ValueType::Array) == Value(ArrayType()));
  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::Map), ConversionError);
  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::Null), ConversionError);
  CHECK_THROWS_AS(Value(ArrayType()).convertTo(ValueType::Undefined), ConversionError);

  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::Boolean), ConversionError);
  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::String), ConversionError);
  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::Number), ConversionError);
  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::Array), ConversionError);
  CHECK(Value(MapType()).convertTo(ValueType::Map) == Value(MapType()));
  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::Null), ConversionError);
  CHECK_THROWS_AS(Value(MapType()).convertTo(ValueType::Undefined), ConversionError);

  CHECK(Value::Null.convertTo(ValueType::Boolean) == Value(false));
  CHECK(Value::Null.convertTo(ValueType::String) == Value(""));
  CHECK(Value::Null.convertTo(ValueType::Number) == Value(0));
  CHECK(Value::Null.convertTo(ValueType::Array) == Value(ArrayType()));
  CHECK(Value::Null.convertTo(ValueType::Map) == Value(MapType()));
  CHECK_THROWS_AS(Value::Null.convertTo(ValueType::Range), ConversionError);
  CHECK(Value::Null.convertTo(ValueType::Null) == Value::Null);
  CHECK_THROWS_AS(Value::Null.convertTo(ValueType::Undefined), ConversionError);

  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Boolean), ConversionError);
  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::String), ConversionError);
  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Number), ConversionError);
  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Array), ConversionError);
  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Map), ConversionError);
  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Range), ConversionError);
  CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Null), ConversionError);
  CHECK(Value::Undefined.convertTo(ValueType::Undefined) == Value::Undefined);
}

TEST_CASE("ELTest.serializeValues")
{
  CHECK(Value(16.0).asString() == std::string("16"));
}

TEST_CASE("ELTest.subscriptOperator")
{
  CHECK_THROWS_AS(Value(true)[Value(0)], EvaluationError);
  CHECK_THROWS_AS(Value(1.0)[Value(0)], EvaluationError);
  CHECK_THROWS_AS(Value()[Value(0)], EvaluationError);

  CHECK(Value("test")[Value(0)] == Value("t"));
  CHECK(Value("test")[Value(1)] == Value("e"));
  CHECK(Value("test")[Value(2)] == Value("s"));
  CHECK(Value("test")[Value(3)] == Value("t"));
  CHECK(Value("test")[Value(-2)] == Value("s"));
  CHECK(Value("test")[Value(4)] == Value(""));

  CHECK(Value("test")[Value(ArrayType({Value(1)}))] == Value("e"));
  CHECK(Value("test")[Value(ArrayType({Value(0), Value(1)}))] == Value("te"));
  CHECK(Value("test")[Value(ArrayType({Value(1), Value(2)}))] == Value("es"));
  CHECK(Value("test")[Value(ArrayType({Value(0), Value(3)}))] == Value("tt"));
  CHECK(
    Value("test")[Value(ArrayType({Value(0), Value(1), Value(2), Value(3)}))]
    == Value("test"));
  CHECK(Value("test")[Value(ArrayType({Value(4)}))] == Value(""));
  CHECK(Value("test")[Value(ArrayType({Value(0), Value(4)}))] == Value("t"));

  const Value arrayValue(ArrayType({Value(1.0), Value("test")}));

  CHECK(arrayValue[Value(0)] == Value(1.0));
  CHECK(arrayValue[Value(1)] == Value("test"));
  CHECK(arrayValue[Value(-1)] == Value("test"));
  CHECK(arrayValue[Value(-2)] == Value(1.0));

  CHECK_THROWS_AS(arrayValue[Value(2)], EvaluationError);
  CHECK_THROWS_AS(arrayValue[Value(-3)], EvaluationError);
  CHECK_THROWS_AS(arrayValue[Value("asdf")], EvaluationError);
  CHECK_THROWS_AS(arrayValue[Value("")], EvaluationError);

  CHECK(arrayValue[Value(ArrayType({Value(0)}))] == Value(ArrayType({Value(1.0)})));
  CHECK(arrayValue[Value(ArrayType({Value(1)}))] == Value(ArrayType({Value("test")})));
  CHECK(
    arrayValue[Value(ArrayType({Value(0), Value(1)}))]
    == Value(ArrayType({Value(1.0), Value("test")})));
  CHECK_THROWS_AS(arrayValue[Value(ArrayType({Value(2)}))], EvaluationError);
  CHECK_THROWS_AS(arrayValue[Value(ArrayType({Value(1), Value(2)}))], EvaluationError);
  CHECK_THROWS_AS(arrayValue[Value(ArrayType({Value("test")}))], ConversionError);
  CHECK_THROWS_AS(
    arrayValue[Value(ArrayType({Value(0), Value("test")}))], ConversionError);

  MapType map;
  map["test"] = Value(1.0);
  map["huhu"] = Value("yeah");

  const Value mapValue(map);

  CHECK(mapValue[Value("test")] == Value(1.0));
  CHECK(mapValue[Value("huhu")] == Value("yeah"));
  CHECK(mapValue[Value("huu")] == Value::Undefined);
  CHECK(mapValue[Value("")] == Value::Undefined);

  MapType exp1;
  exp1["test"] = map["test"];

  MapType exp2;
  exp2["huhu"] = map["huhu"];

  CHECK(mapValue[Value(ArrayType({Value("test")}))] == Value(exp1));
  CHECK(mapValue[Value(ArrayType({Value("huhu")}))] == Value(exp2));
  CHECK(mapValue[Value(ArrayType({Value("test"), Value("huhu")}))] == Value(map));
  CHECK(mapValue[Value(ArrayType({Value("huhu"), Value("test")}))] == Value(map));
  CHECK(mapValue[Value(ArrayType({Value("asdf")}))] == Value(MapType()));
  CHECK(mapValue[Value(ArrayType({Value("test"), Value("asdf")}))] == Value(exp1));
  CHECK_THROWS_AS(mapValue[Value(ArrayType({Value(0)}))], ConversionError);
  CHECK_THROWS_AS(mapValue[Value(ArrayType({Value("test"), Value(0)}))], ConversionError);
}

} // namespace EL
} // namespace TrenchBroom
