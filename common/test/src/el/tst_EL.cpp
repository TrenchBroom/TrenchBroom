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

#include "el/EvaluationContext.h"
#include "el/Exceptions.h"
#include "el/Types.h"
#include "el/Value.h"

#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

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
  withEvaluationContext([](auto& ctx) {
    CHECK(Value(true).convertTo(ctx, ValueType::Boolean) == Value(true));
    CHECK(Value(false).convertTo(ctx, ValueType::Boolean) == Value(false));
    CHECK(Value(true).convertTo(ctx, ValueType::String) == Value("true"));
    CHECK(Value(false).convertTo(ctx, ValueType::String) == Value("false"));
    CHECK(Value(true).convertTo(ctx, ValueType::Number) == Value(1));
    CHECK(Value(false).convertTo(ctx, ValueType::Number) == Value(0));
    CHECK_THROWS_AS(Value(true).convertTo(ctx, ValueType::Array), ConversionError);
    CHECK_THROWS_AS(Value(false).convertTo(ctx, ValueType::Array), ConversionError);
    CHECK_THROWS_AS(Value(true).convertTo(ctx, ValueType::Map), ConversionError);
    CHECK_THROWS_AS(Value(false).convertTo(ctx, ValueType::Map), ConversionError);
    CHECK_THROWS_AS(Value(true).convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value(false).convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value(true).convertTo(ctx, ValueType::Null), ConversionError);
    CHECK_THROWS_AS(Value(false).convertTo(ctx, ValueType::Null), ConversionError);
    CHECK_THROWS_AS(Value(true).convertTo(ctx, ValueType::Undefined), ConversionError);
    CHECK_THROWS_AS(Value(false).convertTo(ctx, ValueType::Undefined), ConversionError);

    CHECK(Value("asdf").convertTo(ctx, ValueType::Boolean) == Value(true));
    CHECK(Value("false").convertTo(ctx, ValueType::Boolean) == Value(false));
    CHECK(Value("").convertTo(ctx, ValueType::Boolean) == Value(false));
    CHECK(Value("asdf").convertTo(ctx, ValueType::String) == Value("asdf"));
    CHECK(Value("2").convertTo(ctx, ValueType::Number) == Value(2));
    CHECK(Value("-2.0").convertTo(ctx, ValueType::Number) == Value(-2));
    CHECK_THROWS_AS(Value("asdf").convertTo(ctx, ValueType::Number), ConversionError);
    CHECK_THROWS_AS(Value("asdf").convertTo(ctx, ValueType::Array), ConversionError);
    CHECK_THROWS_AS(Value("asfd").convertTo(ctx, ValueType::Map), ConversionError);
    CHECK_THROWS_AS(Value("asdf").convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value("asdf").convertTo(ctx, ValueType::Null), ConversionError);
    CHECK_THROWS_AS(Value("asdf").convertTo(ctx, ValueType::Undefined), ConversionError);

    CHECK(Value(1).convertTo(ctx, ValueType::Boolean) == Value(true));
    CHECK(Value(2).convertTo(ctx, ValueType::Boolean) == Value(true));
    CHECK(Value(-2).convertTo(ctx, ValueType::Boolean) == Value(true));
    CHECK(Value(0).convertTo(ctx, ValueType::Boolean) == Value(false));
    CHECK(Value(1.0).convertTo(ctx, ValueType::String) == Value("1"));
    CHECK(Value(-1.0).convertTo(ctx, ValueType::String) == Value("-1"));
    CHECK(Value(1.1).convertTo(ctx, ValueType::String) == Value("1.1000000000000001"));
    CHECK(Value(-1.1).convertTo(ctx, ValueType::String) == Value("-1.1000000000000001"));
    CHECK(Value(1.0).convertTo(ctx, ValueType::Number) == Value(1));
    CHECK(Value(-1.0).convertTo(ctx, ValueType::Number) == Value(-1));
    CHECK_THROWS_AS(Value(1).convertTo(ctx, ValueType::Array), ConversionError);
    CHECK_THROWS_AS(Value(2).convertTo(ctx, ValueType::Map), ConversionError);
    CHECK_THROWS_AS(Value(3).convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value(4).convertTo(ctx, ValueType::Null), ConversionError);
    CHECK_THROWS_AS(Value(5).convertTo(ctx, ValueType::Undefined), ConversionError);

    CHECK_THROWS_AS(
      Value(ArrayType()).convertTo(ctx, ValueType::Boolean), ConversionError);
    CHECK_THROWS_AS(
      Value(ArrayType()).convertTo(ctx, ValueType::String), ConversionError);
    CHECK_THROWS_AS(
      Value(ArrayType()).convertTo(ctx, ValueType::Number), ConversionError);
    CHECK(Value(ArrayType()).convertTo(ctx, ValueType::Array) == Value(ArrayType()));
    CHECK_THROWS_AS(Value(ArrayType()).convertTo(ctx, ValueType::Map), ConversionError);
    CHECK_THROWS_AS(Value(ArrayType()).convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value(ArrayType()).convertTo(ctx, ValueType::Null), ConversionError);
    CHECK_THROWS_AS(
      Value(ArrayType()).convertTo(ctx, ValueType::Undefined), ConversionError);

    CHECK_THROWS_AS(Value(MapType()).convertTo(ctx, ValueType::Boolean), ConversionError);
    CHECK_THROWS_AS(Value(MapType()).convertTo(ctx, ValueType::String), ConversionError);
    CHECK_THROWS_AS(Value(MapType()).convertTo(ctx, ValueType::Number), ConversionError);
    CHECK_THROWS_AS(Value(MapType()).convertTo(ctx, ValueType::Array), ConversionError);
    CHECK(Value(MapType()).convertTo(ctx, ValueType::Map) == Value(MapType()));
    CHECK_THROWS_AS(Value(MapType()).convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value(MapType()).convertTo(ctx, ValueType::Null), ConversionError);
    CHECK_THROWS_AS(
      Value(MapType()).convertTo(ctx, ValueType::Undefined), ConversionError);

    CHECK(Value::Null.convertTo(ctx, ValueType::Boolean) == Value(false));
    CHECK(Value::Null.convertTo(ctx, ValueType::String) == Value(""));
    CHECK(Value::Null.convertTo(ctx, ValueType::Number) == Value(0));
    CHECK(Value::Null.convertTo(ctx, ValueType::Array) == Value(ArrayType()));
    CHECK(Value::Null.convertTo(ctx, ValueType::Map) == Value(MapType()));
    CHECK_THROWS_AS(Value::Null.convertTo(ctx, ValueType::Range), ConversionError);
    CHECK(Value::Null.convertTo(ctx, ValueType::Null) == Value::Null);
    CHECK_THROWS_AS(Value::Null.convertTo(ctx, ValueType::Undefined), ConversionError);

    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::Boolean), ConversionError);
    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::String), ConversionError);
    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::Number), ConversionError);
    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::Array), ConversionError);
    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::Map), ConversionError);
    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::Range), ConversionError);
    CHECK_THROWS_AS(Value::Undefined.convertTo(ctx, ValueType::Null), ConversionError);
    CHECK(Value::Undefined.convertTo(ctx, ValueType::Undefined) == Value::Undefined);
  }).ignore();
}

TEST_CASE("ELTest.serializeValues")
{
  CHECK(Value(16.0).asString() == std::string("16"));
}

} // namespace tb::el
