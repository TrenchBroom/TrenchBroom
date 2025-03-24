/*
 Copyright (C) 2010 Kristian Duske

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

#include "el/ELExceptions.h"
#include "el/Types.h"
#include "el/Value.h"

#include <string>

#include "Catch2.h"

namespace tb::el
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

} // namespace tb::el
