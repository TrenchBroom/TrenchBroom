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

#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{
namespace
{

const auto allTypes = std::vector{
  ValueType::Boolean,
  ValueType::String,
  ValueType::Number,
  ValueType::Array,
  ValueType::Map,
  ValueType::Range,
  ValueType::Null,
  ValueType::Undefined,
};

std::vector<ValueType> convertibleTypes(const Value& value)
{
  auto result = std::vector<ValueType>{};
  for (const auto type : allTypes)
  {
    if (value.convertibleTo(type))
    {
      result.push_back(type);
    }
  }
  return result;
}

const auto boundedRange = Value{RangeType{BoundedRange{1, 3}}};
const auto leftBoundedRange = Value{RangeType{LeftBoundedRange{2}}};
const auto rightBoundedRange = Value{RangeType{RightBoundedRange{5}}};

} // namespace

TEST_CASE("Value")
{
  SECTION("constructor")
  {
    CHECK(Value{}.type() == ValueType::Null);
    CHECK(Value{true}.type() == ValueType::Boolean);
    CHECK(Value{false}.type() == ValueType::Boolean);
    CHECK(Value{StringType{"test"}}.type() == ValueType::String);
    CHECK(Value{"test"}.type() == ValueType::String);
    CHECK(Value{1.0}.type() == ValueType::Number);
    CHECK(Value{1}.type() == ValueType::Number);
    CHECK(Value{1l}.type() == ValueType::Number);
    CHECK(Value{size_t(1)}.type() == ValueType::Number);
    CHECK(Value{ArrayType{}}.type() == ValueType::Array);
    CHECK(Value{MapType{}}.type() == ValueType::Map);
    CHECK(Value{RangeType{BoundedRange{1, 3}}}.type() == ValueType::Range);
    CHECK(Value{NullType::Value}.type() == ValueType::Null);
    CHECK(Value{UndefinedType::Value}.type() == ValueType::Undefined);

    CHECK(Value::Null.type() == ValueType::Null);
    CHECK(Value::Undefined.type() == ValueType::Undefined);

    // the integral constructors all widen to a number
    CHECK(Value{1} == Value{1.0});
    CHECK(Value{1l} == Value{1.0});
    CHECK(Value{size_t(1)} == Value{1.0});
  }

  SECTION("hasType")
  {
    CHECK(Value{1.0}.hasType(ValueType::Number));
    CHECK(!Value{1.0}.hasType(ValueType::String));

    CHECK(Value{1.0}.hasType(ValueType::String, ValueType::Number));
    CHECK(!Value{1.0}.hasType(ValueType::String, ValueType::Boolean));
  }

  SECTION("typeName")
  {
    CHECK(Value{true}.typeName() == "Boolean");
    CHECK(Value{"test"}.typeName() == "String");
    CHECK(Value{1.0}.typeName() == "Number");
    CHECK(Value{ArrayType{}}.typeName() == "Array");
    CHECK(Value{MapType{}}.typeName() == "Map");
    CHECK(boundedRange.typeName() == "Range");
    CHECK(Value::Null.typeName() == "Null");
    CHECK(Value::Undefined.typeName() == "Undefined");
  }

  SECTION("describe")
  {
    CHECK(Value{true}.describe() == "true");
    CHECK(Value{"test"}.describe() == R"("test")");
    CHECK(Value{1.0}.describe() == "1");
    CHECK(Value{ArrayType{Value{1.0}, Value{2.0}}}.describe() == "[1, 2]");
    CHECK(Value{MapType{{"key", Value{1.0}}}}.describe() == R"({ "key": 1 })");
  }

  SECTION("booleanValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{true}.booleanValue(context) == true);
      CHECK(Value{false}.booleanValue(context) == false);
      CHECK(Value::Null.booleanValue(context) == false);

      CHECK_THROWS_AS(Value{"test"}.booleanValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.booleanValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.booleanValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.booleanValue(context), DereferenceError);
      CHECK_THROWS_AS(boundedRange.booleanValue(context), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.booleanValue(context), DereferenceError);
    }).ignore();

    // the error names the type the caller asked for
    CHECK(
      withEvaluationContext([](auto& context) { Value{1.0}.booleanValue(context); })
      == Result<void>{Error{
        "At unknown location: Cannot dereference value '1' of type 'Number' as type "
        "'Boolean'"}});
  }

  SECTION("stringValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{"test"}.stringValue(context) == "test");
      CHECK(Value::Null.stringValue(context) == "");

      CHECK_THROWS_AS(Value{true}.stringValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.stringValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.stringValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.stringValue(context), DereferenceError);
      CHECK_THROWS_AS(boundedRange.stringValue(context), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.stringValue(context), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto& context) { Value{1.0}.stringValue(context); })
      == Result<void>{Error{
        "At unknown location: Cannot dereference value '1' of type 'Number' as type "
        "'String'"}});
  }

  SECTION("numberValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{1.5}.numberValue(context) == 1.5);
      CHECK(Value::Null.numberValue(context) == 0.0);

      CHECK_THROWS_AS(Value{true}.numberValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.numberValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.numberValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.numberValue(context), DereferenceError);
      CHECK_THROWS_AS(boundedRange.numberValue(context), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.numberValue(context), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto& context) { Value{"test"}.numberValue(context); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Number')"}});
  }

  SECTION("integerValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{1.0}.integerValue(context) == 1l);
      CHECK(Value{1.7}.integerValue(context) == 1l);
      CHECK(Value{-1.7}.integerValue(context) == -1l);
      CHECK(Value::Null.integerValue(context) == 0l);

      CHECK_THROWS_AS(Value{"test"}.integerValue(context), DereferenceError);
    }).ignore();
  }

  SECTION("arrayValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{ArrayType{Value{1.0}}}.arrayValue(context) == ArrayType{Value{1.0}});
      CHECK(Value::Null.arrayValue(context) == ArrayType{});

      CHECK_THROWS_AS(Value{true}.arrayValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.arrayValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.arrayValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.arrayValue(context), DereferenceError);
      CHECK_THROWS_AS(boundedRange.arrayValue(context), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.arrayValue(context), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto& context) { Value{"test"}.arrayValue(context); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Array')"}});
  }

  SECTION("mapValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(
        Value{MapType{{"key", Value{1.0}}}}.mapValue(context)
        == MapType{{"key", Value{1.0}}});
      CHECK(Value::Null.mapValue(context) == MapType{});

      CHECK_THROWS_AS(Value{true}.mapValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.mapValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.mapValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.mapValue(context), DereferenceError);
      CHECK_THROWS_AS(boundedRange.mapValue(context), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.mapValue(context), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto& context) { Value{"test"}.mapValue(context); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Map')"}});
  }

  SECTION("rangeValue")
  {
    withEvaluationContext([](auto& context) {
      CHECK(boundedRange.rangeValue(context) == RangeType{BoundedRange{1, 3}});
      CHECK(leftBoundedRange.rangeValue(context) == RangeType{LeftBoundedRange{2}});
      CHECK(rightBoundedRange.rangeValue(context) == RangeType{RightBoundedRange{5}});

      CHECK_THROWS_AS(Value{true}.rangeValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.rangeValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.rangeValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.rangeValue(context), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.rangeValue(context), DereferenceError);

      // unlike the other accessors, a range cannot be dereferenced from null
      CHECK_THROWS_AS(Value::Null.rangeValue(context), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.rangeValue(context), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto& context) { Value{"test"}.rangeValue(context); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Range')"}});
  }

  SECTION("asStringList")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{ArrayType{}}.asStringList(context) == std::vector<std::string>{});
      CHECK(Value::Null.asStringList(context) == std::vector<std::string>{});
      CHECK(
        Value{ArrayType{Value{"b"}, Value{"a"}, Value{"b"}, Value::Null}}.asStringList(
          context)
        == std::vector<std::string>{"b", "a", "b", ""});

      CHECK_THROWS_AS(
        Value{ArrayType{Value{1.0}}}.asStringList(context), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.asStringList(context), DereferenceError);
    }).ignore();
  }

  SECTION("asStringSet")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{ArrayType{}}.asStringSet(context) == std::vector<std::string>{});
      CHECK(
        Value{ArrayType{Value{"b"}, Value{"a"}, Value{"b"}}}.asStringSet(context)
        == std::vector<std::string>{"a", "b"});

      CHECK_THROWS_AS(Value{"test"}.asStringSet(context), DereferenceError);
    }).ignore();
  }

  SECTION("length")
  {
    CHECK(Value{true}.length() == 1u);
    CHECK(Value{""}.length() == 0u);
    CHECK(Value{"test"}.length() == 4u);
    CHECK(Value{1.0}.length() == 1u);
    CHECK(Value{ArrayType{}}.length() == 0u);
    CHECK(Value{ArrayType{Value{1.0}, Value{2.0}}}.length() == 2u);
    CHECK(Value{MapType{}}.length() == 0u);
    CHECK(Value{MapType{{"key", Value{1.0}}}}.length() == 1u);
    CHECK(boundedRange.length() == 2u);
    CHECK(Value::Null.length() == 0u);
    CHECK(Value::Undefined.length() == 0u);
  }

  SECTION("convertibleTo")
  {
    using enum ValueType;

    CHECK(convertibleTypes(Value{true}) == std::vector{Boolean, String, Number});
    CHECK(convertibleTypes(Value{"test"}) == std::vector{Boolean, String});
    CHECK(convertibleTypes(Value{"2.0"}) == std::vector{Boolean, String, Number});
    CHECK(convertibleTypes(Value{" "}) == std::vector{Boolean, String, Number});
    CHECK(convertibleTypes(Value{1.0}) == std::vector{Boolean, String, Number});
    CHECK(convertibleTypes(Value{ArrayType{}}) == std::vector{Array});
    CHECK(convertibleTypes(Value{MapType{}}) == std::vector{Map});
    CHECK(convertibleTypes(boundedRange) == std::vector{Range});
    CHECK(
      convertibleTypes(Value::Null)
      == std::vector{Boolean, String, Number, Array, Map, Null});
    CHECK(convertibleTypes(Value::Undefined) == std::vector{Undefined});
  }

  SECTION("convertTo")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{true}.convertTo(context, ValueType::Boolean) == Value{true});
      CHECK(Value{false}.convertTo(context, ValueType::Boolean) == Value{false});
      CHECK(Value{true}.convertTo(context, ValueType::String) == Value{"true"});
      CHECK(Value{false}.convertTo(context, ValueType::String) == Value{"false"});
      CHECK(Value{true}.convertTo(context, ValueType::Number) == Value{1});
      CHECK(Value{false}.convertTo(context, ValueType::Number) == Value{0});
      CHECK_THROWS_AS(Value{true}.convertTo(context, ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(context, ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(context, ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(context, ValueType::Null), ConversionError);
      CHECK_THROWS_AS(
        Value{true}.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK(Value{"asdf"}.convertTo(context, ValueType::Boolean) == Value{true});
      CHECK(Value{"false"}.convertTo(context, ValueType::Boolean) == Value{false});
      CHECK(Value{""}.convertTo(context, ValueType::Boolean) == Value{false});
      CHECK(Value{"asdf"}.convertTo(context, ValueType::String) == Value{"asdf"});
      CHECK(Value{"2"}.convertTo(context, ValueType::Number) == Value{2});
      CHECK(Value{"-2.0"}.convertTo(context, ValueType::Number) == Value{-2});
      CHECK(Value{" "}.convertTo(context, ValueType::Number) == Value{0});
      CHECK_THROWS_AS(
        Value{"asdf"}.convertTo(context, ValueType::Number), ConversionError);
      CHECK_THROWS_AS(
        Value{"asdf"}.convertTo(context, ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value{"asfd"}.convertTo(context, ValueType::Map), ConversionError);
      CHECK_THROWS_AS(
        Value{"asdf"}.convertTo(context, ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(context, ValueType::Null), ConversionError);
      CHECK_THROWS_AS(
        Value{"asdf"}.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK(Value{1}.convertTo(context, ValueType::Boolean) == Value{true});
      CHECK(Value{2}.convertTo(context, ValueType::Boolean) == Value{true});
      CHECK(Value{-2}.convertTo(context, ValueType::Boolean) == Value{true});
      CHECK(Value{0}.convertTo(context, ValueType::Boolean) == Value{false});
      CHECK(Value{1.0}.convertTo(context, ValueType::String) == Value{"1"});
      CHECK(Value{-1.0}.convertTo(context, ValueType::String) == Value{"-1"});
      CHECK(
        Value{1.1}.convertTo(context, ValueType::String) == Value{"1.1000000000000001"});
      CHECK(
        Value{-1.1}.convertTo(context, ValueType::String)
        == Value{"-1.1000000000000001"});
      CHECK(Value{1.0}.convertTo(context, ValueType::Number) == Value{1});
      CHECK(Value{-1.0}.convertTo(context, ValueType::Number) == Value{-1});
      CHECK_THROWS_AS(Value{1}.convertTo(context, ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value{2}.convertTo(context, ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value{3}.convertTo(context, ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{4}.convertTo(context, ValueType::Null), ConversionError);
      CHECK_THROWS_AS(Value{5}.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::String), ConversionError);
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::Number), ConversionError);
      CHECK(
        Value{ArrayType{}}.convertTo(context, ValueType::Array) == Value{ArrayType{}});
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::Map), ConversionError);
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::Range), ConversionError);
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::Null), ConversionError);
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::String), ConversionError);
      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::Number), ConversionError);
      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::Array), ConversionError);
      CHECK(Value{MapType{}}.convertTo(context, ValueType::Map) == Value{MapType{}});
      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::Range), ConversionError);
      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::Null), ConversionError);
      CHECK_THROWS_AS(
        Value{MapType{}}.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(
        boundedRange.convertTo(context, ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(
        boundedRange.convertTo(context, ValueType::String), ConversionError);
      CHECK_THROWS_AS(
        boundedRange.convertTo(context, ValueType::Number), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(context, ValueType::Array), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(context, ValueType::Map), ConversionError);
      CHECK(boundedRange.convertTo(context, ValueType::Range) == boundedRange);
      CHECK_THROWS_AS(boundedRange.convertTo(context, ValueType::Null), ConversionError);
      CHECK_THROWS_AS(
        boundedRange.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK(Value::Null.convertTo(context, ValueType::Boolean) == Value{false});
      CHECK(Value::Null.convertTo(context, ValueType::String) == Value{""});
      CHECK(Value::Null.convertTo(context, ValueType::Number) == Value{0});
      CHECK(Value::Null.convertTo(context, ValueType::Array) == Value{ArrayType{}});
      CHECK(Value::Null.convertTo(context, ValueType::Map) == Value{MapType{}});
      CHECK_THROWS_AS(Value::Null.convertTo(context, ValueType::Range), ConversionError);
      CHECK(Value::Null.convertTo(context, ValueType::Null) == Value::Null);
      CHECK_THROWS_AS(
        Value::Null.convertTo(context, ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::String), ConversionError);
      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::Number), ConversionError);
      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::Array), ConversionError);
      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::Map), ConversionError);
      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::Range), ConversionError);
      CHECK_THROWS_AS(
        Value::Undefined.convertTo(context, ValueType::Null), ConversionError);
      CHECK(
        Value::Undefined.convertTo(context, ValueType::Undefined) == Value::Undefined);
    }).ignore();
  }

  SECTION("tryConvertTo")
  {
    withEvaluationContext([](auto& context) {
      CHECK(Value{"2"}.tryConvertTo(context, ValueType::Number) == Value{2});
      CHECK(Value{"asdf"}.tryConvertTo(context, ValueType::Number) == std::nullopt);
      CHECK(Value{ArrayType{}}.tryConvertTo(context, ValueType::Map) == std::nullopt);
    }).ignore();
  }

  SECTION("asString")
  {
    SECTION("single line")
    {
      CHECK(Value{true}.asString() == "true");
      CHECK(Value{false}.asString() == "false");
      CHECK(Value{"test"}.asString() == R"("test")");
      // MSVC reports C4129 for escape sequences inside a raw string literal, so these
      // are regular literals: the value is a"b\c and it serializes to "a\"b\\c"
      CHECK(Value{"a\"b\\c"}.asString() == "\"a\\\"b\\\\c\"");
      CHECK(Value{16.0}.asString() == "16");
      CHECK(Value{-16.0}.asString() == "-16");
      CHECK(Value{1.1}.asString() == "1.1000000000000001");
      CHECK(Value{ArrayType{}}.asString() == "[]");
      CHECK(Value{ArrayType{Value{1.0}}}.asString() == "[1]");
      CHECK(Value{ArrayType{Value{1.0}, Value{"a"}}}.asString() == R"([1, "a"])");
      CHECK(Value{ArrayType{Value{ArrayType{Value{1.0}}}}}.asString() == "[[1]]");
      CHECK(Value{MapType{}}.asString() == "{}");
      CHECK(Value{MapType{{"a", Value{1.0}}}}.asString() == R"({ "a": 1 })");
      CHECK(
        Value{MapType{{"a", Value{1.0}}, {"b", Value{2.0}}}}.asString()
        == R"({ "a": 1, "b": 2 })");
      CHECK(
        Value{MapType{{"a", Value{MapType{{"b", Value{1.0}}}}}}}.asString()
        == R"({ "a": { "b": 1 } })");
      CHECK(boundedRange.asString() == "[1..3]");
      CHECK(leftBoundedRange.asString() == "[2..]");
      CHECK(rightBoundedRange.asString() == "[..5]");
      CHECK(Value::Null.asString() == "null");
      CHECK(Value::Undefined.asString() == "undefined");
    }

    SECTION("multiline")
    {
      CHECK(Value{true}.asString(true) == "true");
      CHECK(Value{ArrayType{}}.asString(true) == "[]");
      CHECK(Value{ArrayType{Value{1.0}, Value{2.0}}}.asString(true) == "[\n\t1,\n\t2\n]");
      CHECK(
        Value{ArrayType{Value{ArrayType{Value{1.0}}}}}.asString(true)
        == "[\n\t[\n\t\t1\n\t]\n]");
      CHECK(Value{MapType{}}.asString(true) == "{}");
      CHECK(
        Value{MapType{{"a", Value{1.0}}, {"b", Value{2.0}}}}.asString(true)
        == "{\n\t\"a\": 1,\n\t\"b\": 2\n}");
      CHECK(
        Value{MapType{{"a", Value{MapType{{"b", Value{1.0}}}}}}}.asString(true)
        == "{\n\t\"a\": {\n\t\t\"b\": 1\n\t}\n}");
    }
  }

  SECTION("appendToStream")
  {
    // defaults to multiline output, and appends rather than replaces
    auto str = std::ostringstream{};
    str << "prefix ";
    Value{ArrayType{Value{1.0}, Value{2.0}}}.appendToStream(str);
    CHECK(str.str() == "prefix [\n\t1,\n\t2\n]");
  }

  SECTION("contains")
  {
    SECTION("by index")
    {
      withEvaluationContext([](auto& context) {
        CHECK(Value{"ab"}.contains(context, 0));
        CHECK(Value{"ab"}.contains(context, 1));
        CHECK(!Value{"ab"}.contains(context, 2));
        CHECK(!Value{""}.contains(context, 0));

        CHECK(Value{ArrayType{Value{1.0}}}.contains(context, 0));
        CHECK(!Value{ArrayType{Value{1.0}}}.contains(context, 1));
        CHECK(!Value{ArrayType{}}.contains(context, 0));

        // every other type is not indexable by an integer
        CHECK(!Value{MapType{{"0", Value{1.0}}}}.contains(context, 0));
        CHECK(!Value{true}.contains(context, 0));
        CHECK(!Value{1.0}.contains(context, 0));
        CHECK(!boundedRange.contains(context, 0));
        CHECK(!Value::Null.contains(context, 0));
        CHECK(!Value::Undefined.contains(context, 0));
      }).ignore();
    }

    SECTION("by key")
    {
      withEvaluationContext([](auto& context) {
        CHECK(Value{MapType{{"a", Value{1.0}}}}.contains(context, "a"));
        CHECK(!Value{MapType{{"a", Value{1.0}}}}.contains(context, "b"));
        CHECK(!Value{MapType{}}.contains(context, "a"));
        CHECK(!Value::Null.contains(context, "a"));

        CHECK_THROWS_AS(Value{"ab"}.contains(context, "a"), DereferenceError);
        CHECK_THROWS_AS(Value{ArrayType{}}.contains(context, "a"), DereferenceError);
        CHECK_THROWS_AS(Value::Undefined.contains(context, "a"), DereferenceError);
      }).ignore();
    }
  }

  SECTION("keys")
  {
    withEvaluationContext([](auto& context) {
      CHECK(
        Value{MapType{{"b", Value{1.0}}, {"a", Value{2.0}}}}.keys(context)
        == std::vector<std::string>{"a", "b"});
      CHECK(Value{MapType{}}.keys(context) == std::vector<std::string>{});
      CHECK(Value::Null.keys(context) == std::vector<std::string>{});

      CHECK_THROWS_AS(Value{ArrayType{}}.keys(context), DereferenceError);
    }).ignore();
  }

  SECTION("at")
  {
    SECTION("by index")
    {
      withEvaluationContext([](auto& context) {
        CHECK(Value{"abc"}.at(context, 0) == Value{"a"});
        CHECK(Value{"abc"}.at(context, 2) == Value{"c"});
        CHECK_THROWS_AS(Value{"abc"}.at(context, 3), IndexOutOfBoundsError);

        const auto array = Value{ArrayType{Value{1.0}, Value{"a"}}};
        CHECK(array.at(context, 0) == Value{1.0});
        CHECK(array.at(context, 1) == Value{"a"});
        CHECK_THROWS_AS(array.at(context, 2), IndexOutOfBoundsError);

        CHECK_THROWS_AS(Value{MapType{}}.at(context, 0), IndexError);
        CHECK_THROWS_AS(Value{true}.at(context, 0), IndexError);
        CHECK_THROWS_AS(Value{1.0}.at(context, 0), IndexError);
        CHECK_THROWS_AS(boundedRange.at(context, 0), IndexError);
        CHECK_THROWS_AS(Value::Null.at(context, 0), IndexError);
        CHECK_THROWS_AS(Value::Undefined.at(context, 0), IndexError);
      }).ignore();
    }

    SECTION("by key")
    {
      withEvaluationContext([](auto& context) {
        const auto map = Value{MapType{{"a", Value{1.0}}}};
        CHECK(map.at(context, "a") == Value{1.0});
        CHECK_THROWS_AS(map.at(context, "b"), IndexOutOfBoundsError);

        CHECK_THROWS_AS(Value{"abc"}.at(context, "a"), IndexError);
        CHECK_THROWS_AS(Value{ArrayType{}}.at(context, "a"), IndexError);
        CHECK_THROWS_AS(Value{true}.at(context, "a"), IndexError);
        CHECK_THROWS_AS(Value{1.0}.at(context, "a"), IndexError);
        CHECK_THROWS_AS(boundedRange.at(context, "a"), IndexError);
        CHECK_THROWS_AS(Value::Null.at(context, "a"), IndexError);
        CHECK_THROWS_AS(Value::Undefined.at(context, "a"), IndexError);
      }).ignore();
    }
  }

  SECTION("atOrDefault")
  {
    SECTION("by index")
    {
      withEvaluationContext([](auto& context) {
        CHECK(Value{"abc"}.atOrDefault(context, 0) == Value{"a"});
        CHECK(Value{"abc"}.atOrDefault(context, 3) == Value::Null);
        CHECK(Value{"abc"}.atOrDefault(context, 3, Value{"x"}) == Value{"x"});

        const auto array = Value{ArrayType{Value{1.0}}};
        CHECK(array.atOrDefault(context, 0) == Value{1.0});
        CHECK(array.atOrDefault(context, 1) == Value::Null);
        CHECK(array.atOrDefault(context, 1, Value{"x"}) == Value{"x"});

        // a value that is not indexable by an integer still throws
        CHECK_THROWS_AS(Value{MapType{}}.atOrDefault(context, 0), IndexError);
        CHECK_THROWS_AS(Value{true}.atOrDefault(context, 0), IndexError);
        CHECK_THROWS_AS(Value{1.0}.atOrDefault(context, 0), IndexError);
        CHECK_THROWS_AS(boundedRange.atOrDefault(context, 0), IndexError);
        CHECK_THROWS_AS(Value::Null.atOrDefault(context, 0), IndexError);
        CHECK_THROWS_AS(Value::Undefined.atOrDefault(context, 0), IndexError);
      }).ignore();
    }

    SECTION("by key")
    {
      withEvaluationContext([](auto& context) {
        const auto map = Value{MapType{{"a", Value{1.0}}}};
        CHECK(map.atOrDefault(context, "a") == Value{1.0});
        CHECK(map.atOrDefault(context, "b") == Value::Null);
        CHECK(map.atOrDefault(context, "b", Value{"x"}) == Value{"x"});

        CHECK_THROWS_AS(Value{"abc"}.atOrDefault(context, "a"), IndexError);
        CHECK_THROWS_AS(Value{ArrayType{}}.atOrDefault(context, "a"), IndexError);
        CHECK_THROWS_AS(Value{true}.atOrDefault(context, "a"), IndexError);
        CHECK_THROWS_AS(Value{1.0}.atOrDefault(context, "a"), IndexError);
        CHECK_THROWS_AS(boundedRange.atOrDefault(context, "a"), IndexError);
        CHECK_THROWS_AS(Value::Null.atOrDefault(context, "a"), IndexError);
        CHECK_THROWS_AS(Value::Undefined.atOrDefault(context, "a"), IndexError);
      }).ignore();
    }
  }

  SECTION("operator==")
  {
    CHECK(Value{true} == Value{true});
    CHECK_FALSE(Value{true} == Value{false});
    CHECK(Value{"a"} == Value{"a"});
    CHECK_FALSE(Value{"a"} == Value{"b"});
    CHECK(Value{1.0} == Value{1.0});
    CHECK_FALSE(Value{1.0} == Value{2.0});
    CHECK(Value{ArrayType{Value{1.0}}} == Value{ArrayType{Value{1.0}}});
    CHECK_FALSE(Value{ArrayType{Value{1.0}}} == Value{ArrayType{}});
    CHECK(Value{MapType{{"a", Value{1.0}}}} == Value{MapType{{"a", Value{1.0}}}});
    CHECK_FALSE(Value{MapType{{"a", Value{1.0}}}} == Value{MapType{}});
    CHECK(boundedRange == Value{RangeType{BoundedRange{1, 3}}});
    CHECK_FALSE(boundedRange == leftBoundedRange);
    CHECK(Value::Null == Value{});
    CHECK(Value::Undefined == Value{UndefinedType::Value});

    // values of different types are never equal
    CHECK_FALSE(Value::Null == Value::Undefined);
    CHECK_FALSE(Value{1.0} == Value{"1"});
    CHECK_FALSE(Value{true} == Value{1.0});

    // a value is equal to a copy of itself, which shares its representation
    const auto value = Value{ArrayType{Value{1.0}}};
    const auto copy = value;
    CHECK(value == copy);
  }

  SECTION("operator!=")
  {
    CHECK_FALSE(Value{1.0} != Value{1.0});
    CHECK(Value{1.0} != Value{2.0});
  }

  SECTION("operator<<")
  {
    auto str = std::ostringstream{};
    str << Value{ArrayType{Value{1.0}, Value{2.0}}};
    CHECK(str.str() == "[\n\t1,\n\t2\n]");
  }
}

} // namespace tb::el
