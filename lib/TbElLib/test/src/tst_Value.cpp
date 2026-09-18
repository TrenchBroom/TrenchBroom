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
#include "el/LazyMap.h"
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
  ValueType::Vec3,
  ValueType::BBox,
  ValueType::LazyMap,
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
const auto vec3 = Value{Vec3Type{1, 2, 3}};
const auto bbox = Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}}};

struct LazyMapTestObject
{
  std::string name;
  double number;
};

const auto lazyMapTestFields = LazyMapFields<LazyMapTestObject>{
  {"name", [](const LazyMapTestObject& o) { return Value{o.name}; }},
  {"number", [](const LazyMapTestObject& o) { return Value{o.number}; }},
};

const auto lazyMapTestObject = LazyMapTestObject{"test", 42.0};
const auto testLazyMap = makeLazyMap(lazyMapTestObject, lazyMapTestFields);

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
    CHECK(Value{Vec3Type{1, 2, 3}}.type() == ValueType::Vec3);
    CHECK(
      Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}}}.type() == ValueType::BBox);
    CHECK(testLazyMap.type() == ValueType::LazyMap);
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
    CHECK(testLazyMap.typeName() == "Map");
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
    withEvaluationContext([](auto&) {
      CHECK(Value{true}.booleanValue() == true);
      CHECK(Value{false}.booleanValue() == false);
      CHECK(Value::Null.booleanValue() == false);

      CHECK_THROWS_AS(Value{"test"}.booleanValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.booleanValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.booleanValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.booleanValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.booleanValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.booleanValue(), DereferenceError);
    }).ignore();

    // the error names the type the caller asked for
    CHECK(
      withEvaluationContext([](auto&) { Value{1.0}.booleanValue(); })
      == Result<void>{Error{
        "At unknown location: Cannot dereference value '1' of type 'Number' as type "
        "'Boolean'"}});
  }

  SECTION("stringValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{"test"}.stringValue() == "test");
      CHECK(Value::Null.stringValue() == "");

      CHECK_THROWS_AS(Value{true}.stringValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.stringValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.stringValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.stringValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.stringValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.stringValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{1.0}.stringValue(); })
      == Result<void>{Error{
        "At unknown location: Cannot dereference value '1' of type 'Number' as type "
        "'String'"}});
  }

  SECTION("numberValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{1.5}.numberValue() == 1.5);
      CHECK(Value::Null.numberValue() == 0.0);

      CHECK_THROWS_AS(Value{true}.numberValue(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.numberValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.numberValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.numberValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.numberValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.numberValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.numberValue(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Number')"}});
  }

  SECTION("integerValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{1.0}.integerValue() == 1l);
      CHECK(Value{1.7}.integerValue() == 1l);
      CHECK(Value{-1.7}.integerValue() == -1l);
      CHECK(Value::Null.integerValue() == 0l);

      CHECK_THROWS_AS(Value{"test"}.integerValue(), DereferenceError);
    }).ignore();
  }

  SECTION("arrayValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{ArrayType{Value{1.0}}}.arrayValue() == ArrayType{Value{1.0}});
      CHECK(Value::Null.arrayValue() == ArrayType{});

      CHECK_THROWS_AS(Value{true}.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(vec3.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(bbox.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(testLazyMap.arrayValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.arrayValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.arrayValue(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Array')"}});
  }

  SECTION("mapValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(
        Value{MapType{{"key", Value{1.0}}}}.mapValue() == MapType{{"key", Value{1.0}}});
      CHECK(Value::Null.mapValue() == MapType{});

      CHECK_THROWS_AS(Value{true}.mapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.mapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.mapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.mapValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.mapValue(), DereferenceError);
      CHECK_THROWS_AS(vec3.mapValue(), DereferenceError);
      CHECK_THROWS_AS(bbox.mapValue(), DereferenceError);
      CHECK_THROWS_AS(testLazyMap.mapValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.mapValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.mapValue(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Map')"}});
  }

  SECTION("rangeValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(boundedRange.rangeValue() == RangeType{BoundedRange{1, 3}});
      CHECK(leftBoundedRange.rangeValue() == RangeType{LeftBoundedRange{2}});
      CHECK(rightBoundedRange.rangeValue() == RangeType{RightBoundedRange{5}});

      CHECK_THROWS_AS(Value{true}.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(vec3.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(bbox.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(testLazyMap.rangeValue(), DereferenceError);

      // unlike the other accessors, a range cannot be dereferenced from null
      CHECK_THROWS_AS(Value::Null.rangeValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.rangeValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.rangeValue(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Range')"}});
  }

  SECTION("vec3Value")
  {
    withEvaluationContext([](auto&) {
      CHECK(vec3.vec3Value() == Vec3Type{1, 2, 3});

      CHECK_THROWS_AS(Value{true}.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(bbox.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(testLazyMap.vec3Value(), DereferenceError);

      CHECK_THROWS_AS(Value::Null.vec3Value(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.vec3Value(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.vec3Value(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Vec3')"}});
  }

  SECTION("bboxValue")
  {
    withEvaluationContext([](auto&) {
      CHECK(bbox.bboxValue() == BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}});

      CHECK_THROWS_AS(Value{true}.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(vec3.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(testLazyMap.bboxValue(), DereferenceError);

      CHECK_THROWS_AS(Value::Null.bboxValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.bboxValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.bboxValue(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'BBox')"}});
  }

  SECTION("lazyMap")
  {
    withEvaluationContext([](auto&) {
      CHECK(testLazyMap.lazyMapValue().at("name") == Value{"test"});
      CHECK(testLazyMap.lazyMapValue().at("number") == Value{42.0});
      CHECK(testLazyMap.lazyMapValue().at("missing") == std::nullopt);
      CHECK(
        testLazyMap.lazyMapValue().keys() == std::vector<std::string>{"name", "number"});

      CHECK_THROWS_AS(Value{true}.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{1.0}.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{ArrayType{}}.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(Value{MapType{}}.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(boundedRange.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(vec3.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(bbox.lazyMapValue(), DereferenceError);

      CHECK_THROWS_AS(Value::Null.lazyMapValue(), DereferenceError);
      CHECK_THROWS_AS(Value::Undefined.lazyMapValue(), DereferenceError);
    }).ignore();

    CHECK(
      withEvaluationContext([](auto&) { Value{"test"}.lazyMapValue(); })
      == Result<void>{Error{
        R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Map')"}});
  }

  SECTION("asStringList")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{ArrayType{}}.asStringList() == std::vector<std::string>{});
      CHECK(Value::Null.asStringList() == std::vector<std::string>{});
      CHECK(
        Value{ArrayType{Value{"b"}, Value{"a"}, Value{"b"}, Value::Null}}.asStringList()
        == std::vector<std::string>{"b", "a", "b", ""});

      CHECK_THROWS_AS(Value{ArrayType{Value{1.0}}}.asStringList(), DereferenceError);
      CHECK_THROWS_AS(Value{"test"}.asStringList(), DereferenceError);
    }).ignore();
  }

  SECTION("asStringSet")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{ArrayType{}}.asStringSet() == std::vector<std::string>{});
      CHECK(
        Value{ArrayType{Value{"b"}, Value{"a"}, Value{"b"}}}.asStringSet()
        == std::vector<std::string>{"a", "b"});

      CHECK_THROWS_AS(Value{"test"}.asStringSet(), DereferenceError);
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
    CHECK(vec3.length() == 3u);
    CHECK(bbox.length() == 2u);
    CHECK(testLazyMap.length() == 2u);
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
    CHECK(convertibleTypes(Value{"1 2 3"}) == std::vector{Boolean, String, Number, Vec3});
    CHECK(convertibleTypes(Value{1.0}) == std::vector{Boolean, String, Number});
    CHECK(convertibleTypes(Value{ArrayType{}}) == std::vector{Array});
    CHECK(convertibleTypes(Value{MapType{}}) == std::vector{Map});
    CHECK(convertibleTypes(boundedRange) == std::vector{Range});
    CHECK(convertibleTypes(vec3) == std::vector{String, Vec3});
    CHECK(convertibleTypes(bbox) == std::vector{BBox});
    CHECK(convertibleTypes(testLazyMap) == std::vector{LazyMap});
    CHECK(
      convertibleTypes(Value::Null)
      == std::vector{Boolean, String, Number, Array, Map, Null});
    CHECK(convertibleTypes(Value::Undefined) == std::vector{Undefined});
  }

  SECTION("convertTo")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{true}.convertTo(ValueType::Boolean) == Value{true});
      CHECK(Value{false}.convertTo(ValueType::Boolean) == Value{false});
      CHECK(Value{true}.convertTo(ValueType::String) == Value{"true"});
      CHECK(Value{false}.convertTo(ValueType::String) == Value{"false"});
      CHECK(Value{true}.convertTo(ValueType::Number) == Value{1});
      CHECK(Value{false}.convertTo(ValueType::Number) == Value{0});
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(Value{true}.convertTo(ValueType::Undefined), ConversionError);

      CHECK(Value{"asdf"}.convertTo(ValueType::Boolean) == Value{true});
      CHECK(Value{"false"}.convertTo(ValueType::Boolean) == Value{false});
      CHECK(Value{""}.convertTo(ValueType::Boolean) == Value{false});
      CHECK(Value{"asdf"}.convertTo(ValueType::String) == Value{"asdf"});
      CHECK(Value{"2"}.convertTo(ValueType::Number) == Value{2});
      CHECK(Value{"-2.0"}.convertTo(ValueType::Number) == Value{-2});
      CHECK(Value{" "}.convertTo(ValueType::Number) == Value{0});
      // "1.2 3 4", the format entity properties like "origin" use
      CHECK(Value{"1 2 3"}.convertTo(ValueType::Vec3) == Value{Vec3Type{1, 2, 3}});
      CHECK(Value{"1 2 3"}.convertTo(ValueType::Vec3) == Value{Vec3Type{1, 2, 3}});
      CHECK(
        Value{"1.2 3 4"}.convertTo(ValueType::Vec3) == Value{Vec3Type{1.2, 3.0, 4.0}});
      CHECK_THROWS_AS(Value{"1 2"}.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value{"asfd"}.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(Value{"asdf"}.convertTo(ValueType::Undefined), ConversionError);

      CHECK(Value{1}.convertTo(ValueType::Boolean) == Value{true});
      CHECK(Value{2}.convertTo(ValueType::Boolean) == Value{true});
      CHECK(Value{-2}.convertTo(ValueType::Boolean) == Value{true});
      CHECK(Value{0}.convertTo(ValueType::Boolean) == Value{false});
      CHECK(Value{1.0}.convertTo(ValueType::String) == Value{"1"});
      CHECK(Value{-1.0}.convertTo(ValueType::String) == Value{"-1"});
      CHECK(Value{1.1}.convertTo(ValueType::String) == Value{"1.1000000000000001"});
      CHECK(Value{-1.1}.convertTo(ValueType::String) == Value{"-1.1000000000000001"});
      CHECK(Value{1.0}.convertTo(ValueType::Number) == Value{1});
      CHECK(Value{-1.0}.convertTo(ValueType::Number) == Value{-1});
      CHECK_THROWS_AS(Value{1}.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value{2}.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value{3}.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{6}.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value{7}.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value{8}.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(Value{4}.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(Value{5}.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::String), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::Number), ConversionError);
      CHECK(Value{ArrayType{}}.convertTo(ValueType::Array) == Value{ArrayType{}});
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(Value{ArrayType{}}.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(
        Value{ArrayType{}}.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::String), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Array), ConversionError);
      CHECK(Value{MapType{}}.convertTo(ValueType::Map) == Value{MapType{}});
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(Value{MapType{}}.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::String), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Map), ConversionError);
      CHECK(boundedRange.convertTo(ValueType::Range) == boundedRange);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(boundedRange.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(vec3.convertTo(ValueType::Boolean), ConversionError);
      // "1.2 3 4", the format entity properties like "origin" use
      CHECK(vec3.convertTo(ValueType::String) == Value{"1 2 3"});
      CHECK_THROWS_AS(vec3.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::Range), ConversionError);
      CHECK(vec3.convertTo(ValueType::Vec3) == vec3);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(vec3.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(bbox.convertTo(ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::String), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Vec3), ConversionError);
      CHECK(bbox.convertTo(ValueType::BBox) == bbox);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(bbox.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::String), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::BBox), ConversionError);
      CHECK(testLazyMap.convertTo(ValueType::LazyMap) == testLazyMap);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Null), ConversionError);
      CHECK_THROWS_AS(testLazyMap.convertTo(ValueType::Undefined), ConversionError);

      CHECK(Value::Null.convertTo(ValueType::Boolean) == Value{false});
      CHECK(Value::Null.convertTo(ValueType::String) == Value{""});
      CHECK(Value::Null.convertTo(ValueType::Number) == Value{0});
      CHECK(Value::Null.convertTo(ValueType::Array) == Value{ArrayType{}});
      CHECK(Value::Null.convertTo(ValueType::Map) == Value{MapType{}});
      CHECK_THROWS_AS(Value::Null.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value::Null.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value::Null.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value::Null.convertTo(ValueType::LazyMap), ConversionError);
      CHECK(Value::Null.convertTo(ValueType::Null) == Value::Null);
      CHECK_THROWS_AS(Value::Null.convertTo(ValueType::Undefined), ConversionError);

      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Boolean), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::String), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Number), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Array), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Map), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Range), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Vec3), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::BBox), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::LazyMap), ConversionError);
      CHECK_THROWS_AS(Value::Undefined.convertTo(ValueType::Null), ConversionError);
      CHECK(Value::Undefined.convertTo(ValueType::Undefined) == Value::Undefined);
    }).ignore();
  }

  SECTION("tryConvertTo")
  {
    withEvaluationContext([](auto&) {
      CHECK(Value{"2"}.tryConvertTo(ValueType::Number) == Value{2});
      CHECK(Value{"asdf"}.tryConvertTo(ValueType::Number) == std::nullopt);
      CHECK(Value{ArrayType{}}.tryConvertTo(ValueType::Map) == std::nullopt);
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
      CHECK(vec3.asString() == "vec(1, 2, 3)");
      CHECK(bbox.asString() == "bbox(vec(1, 2, 3), vec(4, 5, 6))");
      // prints exactly like a real Map would, since LazyMap is meant to be
      // indistinguishable from one
      CHECK(testLazyMap.asString() == R"({ "name": "test", "number": 42 })");
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
      CHECK(
        testLazyMap.asString(true) == "{\n\t\"name\": \"test\",\n\t\"number\": 42\n}");
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
      withEvaluationContext([](auto&) {
        CHECK(Value{"ab"}.contains(0));
        CHECK(Value{"ab"}.contains(1));
        CHECK(!Value{"ab"}.contains(2));
        CHECK(!Value{""}.contains(0));

        CHECK(Value{ArrayType{Value{1.0}}}.contains(0));
        CHECK(!Value{ArrayType{Value{1.0}}}.contains(1));
        CHECK(!Value{ArrayType{}}.contains(0));

        // every other type is not indexable by an integer
        CHECK(!Value{MapType{{"0", Value{1.0}}}}.contains(0));
        CHECK(!Value{true}.contains(0));
        CHECK(!Value{1.0}.contains(0));
        CHECK(!boundedRange.contains(0));
        CHECK(!vec3.contains(0));
        CHECK(!bbox.contains(0));
        CHECK(!testLazyMap.contains(0));
        CHECK(!Value::Null.contains(0));
        CHECK(!Value::Undefined.contains(0));
      }).ignore();
    }

    SECTION("by key")
    {
      withEvaluationContext([](auto&) {
        CHECK(Value{MapType{{"a", Value{1.0}}}}.contains("a"));
        CHECK(!Value{MapType{{"a", Value{1.0}}}}.contains("b"));
        CHECK(!Value{MapType{}}.contains("a"));
        CHECK(!Value::Null.contains("a"));

        CHECK(testLazyMap.contains("name"));
        CHECK(testLazyMap.contains("number"));
        CHECK(!testLazyMap.contains("missing"));

        CHECK_THROWS_AS(Value{"ab"}.contains("a"), DereferenceError);
        CHECK_THROWS_AS(Value{ArrayType{}}.contains("a"), DereferenceError);
        CHECK_THROWS_AS(Value::Undefined.contains("a"), DereferenceError);
      }).ignore();
    }
  }

  SECTION("keys")
  {
    withEvaluationContext([](auto&) {
      CHECK(
        Value{MapType{{"b", Value{1.0}}, {"a", Value{2.0}}}}.keys()
        == std::vector<std::string>{"a", "b"});
      CHECK(Value{MapType{}}.keys() == std::vector<std::string>{});
      CHECK(Value::Null.keys() == std::vector<std::string>{});
      CHECK(testLazyMap.keys() == std::vector<std::string>{"name", "number"});

      CHECK_THROWS_AS(Value{ArrayType{}}.keys(), DereferenceError);
    }).ignore();
  }

  SECTION("at")
  {
    SECTION("by index")
    {
      withEvaluationContext([](auto&) {
        CHECK(Value{"abc"}.at(0) == Value{"a"});
        CHECK(Value{"abc"}.at(2) == Value{"c"});
        CHECK_THROWS_AS(Value{"abc"}.at(3), IndexOutOfBoundsError);

        const auto array = Value{ArrayType{Value{1.0}, Value{"a"}}};
        CHECK(array.at(0) == Value{1.0});
        CHECK(array.at(1) == Value{"a"});
        CHECK_THROWS_AS(array.at(2), IndexOutOfBoundsError);

        CHECK_THROWS_AS(Value{MapType{}}.at(0), IndexError);
        CHECK_THROWS_AS(Value{true}.at(0), IndexError);
        CHECK_THROWS_AS(Value{1.0}.at(0), IndexError);
        CHECK_THROWS_AS(boundedRange.at(0), IndexError);
        CHECK_THROWS_AS(vec3.at(0), IndexError);
        CHECK_THROWS_AS(bbox.at(0), IndexError);
        CHECK_THROWS_AS(testLazyMap.at(0), IndexError);
        CHECK_THROWS_AS(Value::Null.at(0), IndexError);
        CHECK_THROWS_AS(Value::Undefined.at(0), IndexError);
      }).ignore();
    }

    SECTION("by key")
    {
      withEvaluationContext([](auto&) {
        const auto map = Value{MapType{{"a", Value{1.0}}}};
        CHECK(map.at("a") == Value{1.0});
        CHECK_THROWS_AS(map.at("b"), IndexOutOfBoundsError);

        CHECK(testLazyMap.at("name") == Value{"test"});
        CHECK_THROWS_AS(testLazyMap.at("missing"), IndexOutOfBoundsError);

        CHECK_THROWS_AS(Value{"abc"}.at("a"), IndexError);
        CHECK_THROWS_AS(Value{ArrayType{}}.at("a"), IndexError);
        CHECK_THROWS_AS(Value{true}.at("a"), IndexError);
        CHECK_THROWS_AS(Value{1.0}.at("a"), IndexError);
        CHECK_THROWS_AS(boundedRange.at("a"), IndexError);
        CHECK_THROWS_AS(vec3.at("a"), IndexError);
        CHECK_THROWS_AS(bbox.at("a"), IndexError);
        CHECK_THROWS_AS(Value::Null.at("a"), IndexError);
        CHECK_THROWS_AS(Value::Undefined.at("a"), IndexError);
      }).ignore();
    }
  }

  SECTION("atOrDefault")
  {
    SECTION("by index")
    {
      withEvaluationContext([](auto&) {
        CHECK(Value{"abc"}.atOrDefault(0) == Value{"a"});
        CHECK(Value{"abc"}.atOrDefault(3) == Value::Null);
        CHECK(Value{"abc"}.atOrDefault(3, Value{"x"}) == Value{"x"});

        const auto array = Value{ArrayType{Value{1.0}}};
        CHECK(array.atOrDefault(0) == Value{1.0});
        CHECK(array.atOrDefault(1) == Value::Null);
        CHECK(array.atOrDefault(1, Value{"x"}) == Value{"x"});

        // a value that is not indexable by an integer still throws
        CHECK_THROWS_AS(Value{MapType{}}.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(Value{true}.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(Value{1.0}.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(boundedRange.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(vec3.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(bbox.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(testLazyMap.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(Value::Null.atOrDefault(0), IndexError);
        CHECK_THROWS_AS(Value::Undefined.atOrDefault(0), IndexError);
      }).ignore();
    }

    SECTION("by key")
    {
      withEvaluationContext([](auto&) {
        const auto map = Value{MapType{{"a", Value{1.0}}}};
        CHECK(map.atOrDefault("a") == Value{1.0});
        CHECK(map.atOrDefault("b") == Value::Null);
        CHECK(map.atOrDefault("b", Value{"x"}) == Value{"x"});

        CHECK(testLazyMap.atOrDefault("name") == Value{"test"});
        CHECK(testLazyMap.atOrDefault("missing") == Value::Null);
        CHECK(testLazyMap.atOrDefault("missing", Value{"x"}) == Value{"x"});

        CHECK_THROWS_AS(Value{"abc"}.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(Value{ArrayType{}}.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(Value{true}.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(Value{1.0}.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(boundedRange.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(vec3.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(bbox.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(Value::Null.atOrDefault("a"), IndexError);
        CHECK_THROWS_AS(Value::Undefined.atOrDefault("a"), IndexError);
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
    CHECK(vec3 == Value{Vec3Type{1, 2, 3}});
    CHECK_FALSE(vec3 == Value{Vec3Type{1, 2, 4}});
    CHECK(bbox == Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}}});
    CHECK_FALSE(bbox == Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 7}}});
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

    // a LazyMap is equal to a literal copy of itself (same underlying closures,
    // caught by the fast path above before the per-alternative visit ever runs)...
    const auto lazyMapCopy = testLazyMap;
    CHECK(testLazyMap == lazyMapCopy);
    // ...but not to another LazyMap independently built over the same data: unlike
    // Map, LazyMap holds std::functions, which aren't themselves comparable, so two
    // separately-constructed LazyMaps are always "not equal" -- consistent with
    // LazyMap never being compared as a whole (see evaluateCompare in Expression.cpp)
    CHECK_FALSE(testLazyMap == makeLazyMap(lazyMapTestObject, lazyMapTestFields));
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
