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
#include "el/Expression.h"
#include "el/LazyMap.h"
#include "el/Types.h"
#include "el/Value.h"

#include "kd/result_io.h" // IWYU pragma: keep

#include "vm/bbox_io.h" // IWYU pragma: keep
#include "vm/vec_io.h"  // IWYU pragma: keep

#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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
  {"name", {ValueType::String, [](const auto& o) { return Value{o.name}; }}},
  {"number", {ValueType::Number, [](const auto& o) { return Value{o.number}; }}},
};

const auto lazyMapTestObject = LazyMapTestObject{"test", 42.0};
const auto testLazyMap = makeLazyMap(lazyMapTestObject, lazyMapTestFields);

} // namespace

TEST_CASE("Value")
{
  SECTION("constructor")
  {
    SECTION("type")
    {
      // clang-format off
      const auto
      [value,                                                 expectedType] = GENERATE(table<Value, ValueType>({
      {Value{},                                               ValueType::Null},
      {Value{true},                                           ValueType::Boolean},
      {Value{false},                                          ValueType::Boolean},
      {Value{StringType{"test"}},                             ValueType::String},
      {Value{"test"},                                         ValueType::String},
      {Value{1.0},                                            ValueType::Number},
      {Value{1},                                              ValueType::Number},
      {Value{1l},                                             ValueType::Number},
      {Value{size_t(1)},                                      ValueType::Number},
      {Value{ArrayType{}},                                    ValueType::Array},
      {Value{MapType{}},                                      ValueType::Map},
      {Value{RangeType{BoundedRange{1, 3}}},                  ValueType::Range},
      {Value{Vec3Type{1, 2, 3}},                              ValueType::Vec3},
      {Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}}}, ValueType::BBox},
      {testLazyMap,                                           ValueType::LazyMap},
      {Value{NullType::Value},                                ValueType::Null},
      {Value{UndefinedType::Value},                           ValueType::Undefined},

      {Value::Null,                                           ValueType::Null},
      {Value::Undefined,                                      ValueType::Undefined},
      }));
      // clang-format on

      CAPTURE(value);

      CHECK(value.type() == expectedType);
    }

    SECTION("integral constructors")
    {
      // the integral constructors all widen to a number

      // clang-format off
      const auto
      [value,            expectedValue] = GENERATE(table<Value, Value>({
      {Value{1},         Value{1.0}},
      {Value{1l},        Value{1.0}},
      {Value{size_t(1)}, Value{1.0}},
      }));
      // clang-format on

      CAPTURE(value);

      CHECK(value == expectedValue);
    }
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
    // clang-format off
    const auto
    [value,              expectedTypeName] = GENERATE(table<Value, std::string>({
    {Value{true},        "Boolean"},
    {Value{"test"},      "String"},
    {Value{1.0},         "Number"},
    {Value{ArrayType{}}, "Array"},
    {Value{MapType{}},   "Map"},
    {boundedRange,       "Range"},
    {testLazyMap,        "Map"},
    {Value::Null,        "Null"},
    {Value::Undefined,   "Undefined"},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(value.typeName() == expectedTypeName);
  }

  SECTION("describe")
  {
    // clang-format off
    const auto
    [value,                                    expectedDescription] = GENERATE(table<Value, std::string>({
    {Value{true},                              "true"},
    {Value{"test"},                            R"("test")"},
    {Value{1.0},                               "1"},
    {Value{ArrayType{Value{1.0}, Value{2.0}}}, "[1, 2]"},
    {Value{MapType{{"key", Value{1.0}}}},      R"({ "key": 1 })"},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(value.describe() == expectedDescription);
  }

  SECTION("booleanValue")
  {
    // clang-format off
    const auto
    [value,              expectedResult] = GENERATE(table<Value, Result<BooleanType>>({
    {Value{true},        true},
    {Value{false},       false},
    {Value::Null,        false},

    {Value{"test"},      Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Boolean')"}},
    {Value{1.0},         Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'Boolean'"}},
    {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Boolean'"}},
    {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'Boolean'"}},
    {boundedRange,       Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'Boolean'"}},
    {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Boolean'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.booleanValue(); })
      == expectedResult);
  }

  SECTION("stringValue")
  {
    // clang-format off
    const auto
    [value,              expectedResult] = GENERATE(table<Value, Result<StringType>>({
    {Value{"test"},      "test"},
    {Value::Null,        ""},

    {Value{true},        Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'String'"}},
    {Value{1.0},         Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'String'"}},
    {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'String'"}},
    {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'String'"}},
    {boundedRange,       Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'String'"}},
    {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'String'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.stringValue(); })
      == expectedResult);
  }

  SECTION("numberValue")
  {
    // clang-format off
    const auto
    [value,              expectedResult] = GENERATE(table<Value, Result<NumberType>>({
    {Value{1.5},         1.5},
    {Value::Null,        0.0},

    {Value{true},        Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'Number'"}},
    {Value{"test"},      Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Number')"}},
    {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Number'"}},
    {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'Number'"}},
    {boundedRange,       Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'Number'"}},
    {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Number'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.numberValue(); })
      == expectedResult);
  }

  SECTION("integerValue")
  {
    // clang-format off
    const auto
    [value,         expectedResult] = GENERATE(table<Value, Result<IntegerType>>({
    {Value{1.0},    1l},
    {Value{1.7},    1l},
    {Value{-1.7},   -1l},
    {Value::Null,   0l},

    {Value{"test"}, Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Number')"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.integerValue(); })
      == expectedResult);
  }

  SECTION("arrayValue")
  {
    // clang-format off
    const auto
    [value,                        expectedResult] = GENERATE(table<Value, Result<ArrayType>>({
    {Value{ArrayType{Value{1.0}}}, ArrayType{Value{1.0}}},
    {Value::Null,                  ArrayType{}},

    {Value{true},                  Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'Array'"}},
    {Value{"test"},                Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Array')"}},
    {Value{1.0},                   Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'Array'"}},
    {Value{MapType{}},             Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'Array'"}},
    {boundedRange,                 Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'Array'"}},
    {vec3,                         Error{"At unknown location: Cannot dereference value 'vec(1, 2, 3)' of type 'Vec3' as type 'Array'"}},
    {bbox,                         Error{"At unknown location: Cannot dereference value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' as type 'Array'"}},
    {testLazyMap,                  Error{R"(At unknown location: Cannot dereference value '{ "name": "test", "number": 42 }' of type 'Map' as type 'Array')"}},
    {Value::Undefined,             Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Array'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.arrayValue(); }) == expectedResult);
  }

  SECTION("mapValue")
  {
    // clang-format off
    const auto
    [value,                               expectedResult] = GENERATE(table<Value, Result<MapType>>({
    {Value{MapType{{"key", Value{1.0}}}}, MapType{{"key", Value{1.0}}}},
    {Value::Null,                         MapType{}},

    {Value{true},                         Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'Map'"}},
    {Value{"test"},                       Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Map')"}},
    {Value{1.0},                          Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'Map'"}},
    {Value{ArrayType{}},                  Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Map'"}},
    {boundedRange,                        Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'Map'"}},
    {vec3,                                Error{"At unknown location: Cannot dereference value 'vec(1, 2, 3)' of type 'Vec3' as type 'Map'"}},
    {bbox,                                Error{"At unknown location: Cannot dereference value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' as type 'Map'"}},
    {testLazyMap,                         Error{R"(At unknown location: Cannot dereference value '{ "name": "test", "number": 42 }' of type 'Map' as type 'Map')"}},
    {Value::Undefined,                    Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Map'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.mapValue(); }) == expectedResult);
  }

  SECTION("rangeValue")
  {
    // clang-format off
    const auto
    [value,              expectedResult] = GENERATE(table<Value, Result<RangeType>>({
    {boundedRange,       RangeType{BoundedRange{1, 3}}},
    {leftBoundedRange,   RangeType{LeftBoundedRange{2}}},
    {rightBoundedRange,  RangeType{RightBoundedRange{5}}},

    {Value{true},        Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'Range'"}},
    {Value{"test"},      Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Range')"}},
    {Value{1.0},         Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'Range'"}},
    {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Range'"}},
    {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'Range'"}},
    {vec3,               Error{"At unknown location: Cannot dereference value 'vec(1, 2, 3)' of type 'Vec3' as type 'Range'"}},
    {bbox,               Error{"At unknown location: Cannot dereference value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' as type 'Range'"}},
    {testLazyMap,        Error{R"(At unknown location: Cannot dereference value '{ "name": "test", "number": 42 }' of type 'Map' as type 'Range')"}},

    // unlike the other accessors, a range cannot be dereferenced from null
    {Value::Null,        Error{"At unknown location: Cannot dereference value 'null' of type 'Null' as type 'Range'"}},
    {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Range'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.rangeValue(); }) == expectedResult);
  }

  SECTION("vec3Value")
  {
    // clang-format off
    const auto
    [value,              expectedResult] = GENERATE(table<Value, Result<Vec3Type>>({
    {vec3,               Vec3Type{1, 2, 3}},

    {Value{true},        Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'Vec3'"}},
    {Value{"test"},      Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Vec3')"}},
    {Value{1.0},         Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'Vec3'"}},
    {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Vec3'"}},
    {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'Vec3'"}},
    {boundedRange,       Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'Vec3'"}},
    {bbox,               Error{"At unknown location: Cannot dereference value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' as type 'Vec3'"}},
    {testLazyMap,        Error{R"(At unknown location: Cannot dereference value '{ "name": "test", "number": 42 }' of type 'Map' as type 'Vec3')"}},

    {Value::Null,        Error{"At unknown location: Cannot dereference value 'null' of type 'Null' as type 'Vec3'"}},
    {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Vec3'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.vec3Value(); }) == expectedResult);
  }

  SECTION("bboxValue")
  {
    // clang-format off
    const auto
    [value,              expectedResult] = GENERATE(table<Value, Result<BBoxType>>({
    {bbox,               BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}}},

    {Value{true},        Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'BBox'"}},
    {Value{"test"},      Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'BBox')"}},
    {Value{1.0},         Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'BBox'"}},
    {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'BBox'"}},
    {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'BBox'"}},
    {boundedRange,       Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'BBox'"}},
    {vec3,               Error{"At unknown location: Cannot dereference value 'vec(1, 2, 3)' of type 'Vec3' as type 'BBox'"}},
    {testLazyMap,        Error{R"(At unknown location: Cannot dereference value '{ "name": "test", "number": 42 }' of type 'Map' as type 'BBox')"}},

    {Value::Null,        Error{"At unknown location: Cannot dereference value 'null' of type 'Null' as type 'BBox'"}},
    {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'BBox'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.bboxValue(); }) == expectedResult);
  }

  SECTION("lazyMap")
  {
    SECTION("lazy map")
    {
      const auto& lazyMap = testLazyMap.lazyMapValue();
      CHECK(lazyMap.at("name") == Value{"test"});
      CHECK(lazyMap.at("number") == Value{42.0});
      CHECK(lazyMap.at("missing") == std::nullopt);
      CHECK(lazyMap.keys() == std::vector<std::string>{"name", "number"});
    }

    SECTION("other types")
    {
      // clang-format off
      const auto
      [value,              expectedResult] = GENERATE(table<Value, Result<void>>({
      {Value{true},        Error{"At unknown location: Cannot dereference value 'true' of type 'Boolean' as type 'Map'"}},
      {Value{"test"},      Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Map')"}},
      {Value{1.0},         Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'Map'"}},
      {Value{ArrayType{}}, Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Map'"}},
      {Value{MapType{}},   Error{"At unknown location: Cannot dereference value '{}' of type 'Map' as type 'Map'"}},
      {boundedRange,       Error{"At unknown location: Cannot dereference value '[1..3]' of type 'Range' as type 'Map'"}},
      {vec3,               Error{"At unknown location: Cannot dereference value 'vec(1, 2, 3)' of type 'Vec3' as type 'Map'"}},
      {bbox,               Error{"At unknown location: Cannot dereference value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' as type 'Map'"}},

      {Value::Null,        Error{"At unknown location: Cannot dereference value 'null' of type 'Null' as type 'Map'"}},
      {Value::Undefined,   Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Map'"}},
      }));
      // clang-format on

      CAPTURE(value);

      CHECK(
        withEvaluationContext([&](auto&) { value.lazyMapValue(); }) == expectedResult);
    }
  }

  SECTION("asStringList")
  {
    // clang-format off
    const auto
    [value,                                                             expectedResult] = GENERATE(table<Value, Result<std::vector<std::string>>>({
    {Value{ArrayType{}},                                                std::vector<std::string>{}},
    {Value::Null,                                                       std::vector<std::string>{}},
    {Value{ArrayType{Value{"b"}, Value{"a"}, Value{"b"}, Value::Null}}, std::vector<std::string>{"b", "a", "b", ""}},

    {Value{ArrayType{Value{1.0}}},                                      Error{"At unknown location: Cannot dereference value '1' of type 'Number' as type 'String'"}},
    {Value{"test"},                                                     Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Array')"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.asStringList(); })
      == expectedResult);
  }

  SECTION("asStringSet")
  {
    // clang-format off
    const auto
    [value,                                                expectedResult] = GENERATE(table<Value, Result<std::vector<std::string>>>({
    {Value{ArrayType{}},                                   std::vector<std::string>{}},
    {Value{ArrayType{Value{"b"}, Value{"a"}, Value{"b"}}}, std::vector<std::string>{"a", "b"}},

    {Value{"test"},                                        Error{R"(At unknown location: Cannot dereference value '"test"' of type 'String' as type 'Array')"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(
      withEvaluationContext([&](auto&) { return value.asStringSet(); })
      == expectedResult);
  }

  SECTION("length")
  {
    // clang-format off
    const auto
    [value,                                    expectedLength] = GENERATE(table<Value, size_t>({
    {Value{true},                              1u},
    {Value{""},                                0u},
    {Value{"test"},                            4u},
    {Value{1.0},                               1u},
    {Value{ArrayType{}},                       0u},
    {Value{ArrayType{Value{1.0}, Value{2.0}}}, 2u},
    {Value{MapType{}},                         0u},
    {Value{MapType{{"key", Value{1.0}}}},      1u},
    {boundedRange,                             2u},
    {vec3,                                     3u},
    {bbox,                                     2u},
    {testLazyMap,                              2u},
    {Value::Null,                              0u},
    {Value::Undefined,                         0u},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(value.length() == expectedLength);
  }

  SECTION("convertibleTo")
  {
    using enum ValueType;

    SECTION("convertible types")
    {
      // clang-format off
      const auto
      [value,              expectedTypes] = GENERATE(table<Value, std::vector<ValueType>>({
      {Value{true},        std::vector{Boolean, String, Number}},
      {Value{"test"},      std::vector{Boolean, String}},
      {Value{"2.0"},       std::vector{Boolean, String, Number}},
      {Value{" "},         std::vector{Boolean, String, Number}},
      {Value{"1 2 3"},     std::vector{Boolean, String, Number, Vec3}},
      {Value{1.0},         std::vector{Boolean, String, Number}},
      {Value{ArrayType{}}, std::vector{Array}},
      {Value{MapType{}},   std::vector{Map}},
      {boundedRange,       std::vector{Range}},
      {vec3,               std::vector{String, Vec3}},
      {bbox,               std::vector{BBox}},
      {testLazyMap,        std::vector{Map, LazyMap}},
      {Value::Null,       std::vector{Boolean, String, Number, Array, Map, Null}},
      {Value::Undefined,   std::vector{Undefined}},
      }));
      // clang-format on

      CAPTURE(value);

      CHECK(convertibleTypes(value) == expectedTypes);
    }

    SECTION("strings that are not vectors")
    {
      const auto value = GENERATE(values<Value>({
        Value{"1 2 3 4"},
        Value{"1 2 3abc"},
      }));

      CAPTURE(value);

      CHECK(!value.convertibleTo(Vec3));
    }
  }

  SECTION("convertTo")
  {
    // clang-format off
    const auto
    [value,                                 type,                 expectedResult] = GENERATE(table<Value, ValueType, Result<Value>>({
    {Value{true},                           ValueType::Boolean,   Value{true}},
    {Value{false},                          ValueType::Boolean,   Value{false}},
    {Value{true},                           ValueType::String,    Value{"true"}},
    {Value{false},                          ValueType::String,    Value{"false"}},
    {Value{true},                           ValueType::Number,    Value{1}},
    {Value{false},                          ValueType::Number,    Value{0}},
    {Value{true},                           ValueType::Array,     Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Array'"}},
    {Value{true},                           ValueType::Map,       Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Map'"}},
    {Value{true},                           ValueType::Range,     Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Range'"}},
    {Value{true},                           ValueType::Vec3,      Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Vec3'"}},
    {Value{true},                           ValueType::BBox,      Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'BBox'"}},
    {Value{true},                           ValueType::LazyMap,   Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Map'"}},
    {Value{true},                           ValueType::Null,      Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Null'"}},
    {Value{true},                           ValueType::Undefined, Error{"At unknown location: Cannot convert value 'true' of type 'Boolean' to type 'Undefined'"}},

    {Value{"asdf"},                         ValueType::Boolean,   Value{true}},
    {Value{"false"},                        ValueType::Boolean,   Value{false}},
    {Value{""},                             ValueType::Boolean,   Value{false}},
    {Value{"asdf"},                         ValueType::String,    Value{"asdf"}},
    {Value{"2"},                            ValueType::Number,    Value{2}},
    {Value{"-2.0"},                         ValueType::Number,    Value{-2}},
    {Value{" "},                            ValueType::Number,    Value{0}},
    // "1.2 3 4", the format entity properties like "origin" use
    {Value{"1 2 3"},                        ValueType::Vec3,      Value{Vec3Type{1, 2, 3}}},
    {Value{"1.2 3 4"},                      ValueType::Vec3,      Value{Vec3Type{1.2, 3.0, 4.0}}},
    {Value{" 1\t2  3 "},                    ValueType::Vec3,      Value{Vec3Type{1, 2, 3}}},
    {Value{"1 2"},                          ValueType::Vec3,      Error{R"(At unknown location: Cannot convert value '"1 2"' of type 'String' to type 'Vec3')"}},
    {Value{"1 2 3 4"},                      ValueType::Vec3,      Error{R"(At unknown location: Cannot convert value '"1 2 3 4"' of type 'String' to type 'Vec3')"}},
    {Value{"1 2 3abc"},                     ValueType::Vec3,      Error{R"(At unknown location: Cannot convert value '"1 2 3abc"' of type 'String' to type 'Vec3')"}},
    {Value{"1abc 2 3"},                     ValueType::Vec3,      Error{R"(At unknown location: Cannot convert value '"1abc 2 3"' of type 'String' to type 'Vec3')"}},
    {Value{"asdf"},                         ValueType::Number,    Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Number')"}},
    {Value{"asdf"},                         ValueType::Array,     Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Array')"}},
    {Value{"asfd"},                         ValueType::Map,       Error{R"(At unknown location: Cannot convert value '"asfd"' of type 'String' to type 'Map')"}},
    {Value{"asdf"},                         ValueType::Range,     Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Range')"}},
    {Value{"asdf"},                         ValueType::Vec3,      Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Vec3')"}},
    {Value{"asdf"},                         ValueType::BBox,      Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'BBox')"}},
    {Value{"asdf"},                         ValueType::LazyMap,   Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Map')"}},
    {Value{"asdf"},                         ValueType::Null,      Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Null')"}},
    {Value{"asdf"},                         ValueType::Undefined, Error{R"(At unknown location: Cannot convert value '"asdf"' of type 'String' to type 'Undefined')"}},

    {Value{1},                              ValueType::Boolean,   Value{true}},
    {Value{2},                              ValueType::Boolean,   Value{true}},
    {Value{-2},                             ValueType::Boolean,   Value{true}},
    {Value{0},                              ValueType::Boolean,   Value{false}},
    {Value{1.0},                            ValueType::String,    Value{"1"}},
    {Value{-1.0},                           ValueType::String,    Value{"-1"}},
    {Value{1.1},                            ValueType::String,    Value{"1.1000000000000001"}},
    {Value{-1.1},                           ValueType::String,    Value{"-1.1000000000000001"}},
    {Value{1.0},                            ValueType::Number,    Value{1}},
    {Value{-1.0},                           ValueType::Number,    Value{-1}},
    {Value{1},                              ValueType::Array,     Error{"At unknown location: Cannot convert value '1' of type 'Number' to type 'Array'"}},
    {Value{2},                              ValueType::Map,       Error{"At unknown location: Cannot convert value '2' of type 'Number' to type 'Map'"}},
    {Value{3},                              ValueType::Range,     Error{"At unknown location: Cannot convert value '3' of type 'Number' to type 'Range'"}},
    {Value{6},                              ValueType::Vec3,      Error{"At unknown location: Cannot convert value '6' of type 'Number' to type 'Vec3'"}},
    {Value{7},                              ValueType::BBox,      Error{"At unknown location: Cannot convert value '7' of type 'Number' to type 'BBox'"}},
    {Value{8},                              ValueType::LazyMap,   Error{"At unknown location: Cannot convert value '8' of type 'Number' to type 'Map'"}},
    {Value{4},                              ValueType::Null,      Error{"At unknown location: Cannot convert value '4' of type 'Number' to type 'Null'"}},
    {Value{5},                              ValueType::Undefined, Error{"At unknown location: Cannot convert value '5' of type 'Number' to type 'Undefined'"}},

    {Value{ArrayType{}},                    ValueType::Boolean,   Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Boolean'"}},
    {Value{ArrayType{}},                    ValueType::String,    Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'String'"}},
    {Value{ArrayType{}},                    ValueType::Number,    Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Number'"}},
    {Value{ArrayType{}},                    ValueType::Array,     Value{ArrayType{}}},
    {Value{ArrayType{}},                    ValueType::Map,       Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Map'"}},
    {Value{ArrayType{}},                    ValueType::Range,     Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Range'"}},
    {Value{ArrayType{}},                    ValueType::Vec3,      Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Vec3'"}},
    {Value{ArrayType{}},                    ValueType::BBox,      Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'BBox'"}},
    {Value{ArrayType{}},                    ValueType::LazyMap,   Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Map'"}},
    {Value{ArrayType{}},                    ValueType::Null,      Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Null'"}},
    {Value{ArrayType{}},                    ValueType::Undefined, Error{"At unknown location: Cannot convert value '[]' of type 'Array' to type 'Undefined'"}},

    {Value{MapType{}},                      ValueType::Boolean,   Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Boolean'"}},
    {Value{MapType{}},                      ValueType::String,    Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'String'"}},
    {Value{MapType{}},                      ValueType::Number,    Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Number'"}},
    {Value{MapType{}},                      ValueType::Array,     Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Array'"}},
    {Value{MapType{}},                      ValueType::Map,       Value{MapType{}}},
    {Value{MapType{}},                      ValueType::Range,     Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Range'"}},
    {Value{MapType{}},                      ValueType::Vec3,      Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Vec3'"}},
    {Value{MapType{}},                      ValueType::BBox,      Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'BBox'"}},
    {Value{MapType{}},                      ValueType::LazyMap,   Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Map'"}},
    {Value{MapType{}},                      ValueType::Null,      Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Null'"}},
    {Value{MapType{}},                      ValueType::Undefined, Error{"At unknown location: Cannot convert value '{}' of type 'Map' to type 'Undefined'"}},

    {boundedRange,                          ValueType::Boolean,   Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Boolean'"}},
    {boundedRange,                          ValueType::String,    Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'String'"}},
    {boundedRange,                          ValueType::Number,    Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Number'"}},
    {boundedRange,                          ValueType::Array,     Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Array'"}},
    {boundedRange,                          ValueType::Map,       Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Map'"}},
    {boundedRange,                          ValueType::Range,     boundedRange},
    {boundedRange,                          ValueType::Vec3,      Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Vec3'"}},
    {boundedRange,                          ValueType::BBox,      Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'BBox'"}},
    {boundedRange,                          ValueType::LazyMap,   Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Map'"}},
    {boundedRange,                          ValueType::Null,      Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Null'"}},
    {boundedRange,                          ValueType::Undefined, Error{"At unknown location: Cannot convert value '[1..3]' of type 'Range' to type 'Undefined'"}},

    {vec3,                                  ValueType::Boolean,   Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Boolean'"}},
    // "1.2 3 4", the format entity properties like "origin" use
    {vec3,                                  ValueType::String,    Value{"1 2 3"}},
    // components are formatted like numbers, without losing precision
    {Value{Vec3Type{1234567.5, 0.125, -4}}, ValueType::String,    Value{"1234567.5 0.125 -4"}},
    {vec3,                                  ValueType::Number,    Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Number'"}},
    {vec3,                                  ValueType::Array,     Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Array'"}},
    {vec3,                                  ValueType::Map,       Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Map'"}},
    {vec3,                                  ValueType::Range,     Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Range'"}},
    {vec3,                                  ValueType::Vec3,      vec3},
    {vec3,                                  ValueType::BBox,      Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'BBox'"}},
    {vec3,                                  ValueType::LazyMap,   Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Map'"}},
    {vec3,                                  ValueType::Null,      Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Null'"}},
    {vec3,                                  ValueType::Undefined, Error{"At unknown location: Cannot convert value 'vec(1, 2, 3)' of type 'Vec3' to type 'Undefined'"}},

    {bbox,                                  ValueType::Boolean,   Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Boolean'"}},
    {bbox,                                  ValueType::String,    Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'String'"}},
    {bbox,                                  ValueType::Number,    Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Number'"}},
    {bbox,                                  ValueType::Array,     Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Array'"}},
    {bbox,                                  ValueType::Map,       Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Map'"}},
    {bbox,                                  ValueType::Range,     Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Range'"}},
    {bbox,                                  ValueType::Vec3,      Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Vec3'"}},
    {bbox,                                  ValueType::BBox,      bbox},
    {bbox,                                  ValueType::LazyMap,   Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Map'"}},
    {bbox,                                  ValueType::Null,      Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Null'"}},
    {bbox,                                  ValueType::Undefined, Error{"At unknown location: Cannot convert value 'bbox(vec(1, 2, 3), vec(4, 5, 6))' of type 'BBox' to type 'Undefined'"}},

    {testLazyMap,                           ValueType::Boolean,   Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Boolean')"}},
    {testLazyMap,                           ValueType::String,    Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'String')"}},
    {testLazyMap,                           ValueType::Number,    Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Number')"}},
    {testLazyMap,                           ValueType::Array,     Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Array')"}},
    {testLazyMap,                           ValueType::Map,       Value{MapType{{"name", Value{"test"}}, {"number", Value{42}}}}},
    {testLazyMap,                           ValueType::Range,     Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Range')"}},
    {testLazyMap,                           ValueType::Vec3,      Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Vec3')"}},
    {testLazyMap,                           ValueType::BBox,      Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'BBox')"}},
    {testLazyMap,                           ValueType::LazyMap,   testLazyMap},
    {testLazyMap,                           ValueType::Null,      Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Null')"}},
    {testLazyMap,                           ValueType::Undefined, Error{R"(At unknown location: Cannot convert value '{ "name": "test", "number": 42 }' of type 'Map' to type 'Undefined')"}},

    {Value::Null,                           ValueType::Boolean,   Value{false}},
    {Value::Null,                           ValueType::String,    Value{""}},
    {Value::Null,                           ValueType::Number,    Value{0}},
    {Value::Null,                           ValueType::Array,     Value{ArrayType{}}},
    {Value::Null,                           ValueType::Map,       Value{MapType{}}},
    {Value::Null,                           ValueType::Range,     Error{"At unknown location: Cannot convert value 'null' of type 'Null' to type 'Range'"}},
    {Value::Null,                           ValueType::Vec3,      Error{"At unknown location: Cannot convert value 'null' of type 'Null' to type 'Vec3'"}},
    {Value::Null,                           ValueType::BBox,      Error{"At unknown location: Cannot convert value 'null' of type 'Null' to type 'BBox'"}},
    {Value::Null,                           ValueType::LazyMap,   Error{"At unknown location: Cannot convert value 'null' of type 'Null' to type 'Map'"}},
    {Value::Null,                           ValueType::Null,      Value::Null},
    {Value::Null,                           ValueType::Undefined, Error{"At unknown location: Cannot convert value 'null' of type 'Null' to type 'Undefined'"}},

    {Value::Undefined,                      ValueType::Boolean,   Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Boolean'"}},
    {Value::Undefined,                      ValueType::String,    Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'String'"}},
    {Value::Undefined,                      ValueType::Number,    Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Number'"}},
    {Value::Undefined,                      ValueType::Array,     Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Array'"}},
    {Value::Undefined,                      ValueType::Map,       Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Map'"}},
    {Value::Undefined,                      ValueType::Range,     Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Range'"}},
    {Value::Undefined,                      ValueType::Vec3,      Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Vec3'"}},
    {Value::Undefined,                      ValueType::BBox,      Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'BBox'"}},
    {Value::Undefined,                      ValueType::LazyMap,   Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Map'"}},
    {Value::Undefined,                      ValueType::Null,      Error{"At unknown location: Cannot convert value 'undefined' of type 'Undefined' to type 'Null'"}},
    {Value::Undefined,                      ValueType::Undefined, Value::Undefined},
    }));
    // clang-format on

    CAPTURE(value, type);

    CHECK(
      withEvaluationContext([&](auto&) { return value.convertTo(type); })
      == expectedResult);
  }

  SECTION("tryConvertTo")
  {
    // clang-format off
    const auto
    [value,              type,              expectedValue] = GENERATE(table<Value, ValueType, std::optional<Value>>({
    {Value{"2"},         ValueType::Number, Value{2}},
    {Value{"asdf"},      ValueType::Number, std::nullopt},
    {Value{ArrayType{}}, ValueType::Map,    std::nullopt},
    }));
    // clang-format on

    CAPTURE(value, type);

    CHECK(value.tryConvertTo(type) == expectedValue);
  }

  SECTION("asString")
  {
    SECTION("single line")
    {
      // clang-format off
      const auto
      [value,                                                    expectedString] = GENERATE(table<Value, std::string>({
      {Value{true},                                              "true"},
      {Value{false},                                             "false"},
      {Value{"test"},                                            R"("test")"},
      {Value{R"(a"b\c)"},                                        R"("a\"b\c")"},
      {Value{R"(C:\quake\)"},                                    R"("C:\quake\\")"},
      {Value{16.0},                                              "16"},
      {Value{-16.0},                                             "-16"},
      {Value{1.1},                                               "1.1000000000000001"},
      {Value{ArrayType{}},                                       "[]"},
      {Value{ArrayType{Value{1.0}}},                             "[1]"},
      {Value{ArrayType{Value{1.0}, Value{"a"}}},                 R"([1, "a"])"},
      {Value{ArrayType{Value{ArrayType{Value{1.0}}}}},           "[[1]]"},
      {Value{MapType{}},                                         "{}"},
      {Value{MapType{{"a", Value{1.0}}}},                        R"({ "a": 1 })"},
      {Value{MapType{{"a", Value{1.0}}, {"b", Value{2.0}}}},     R"({ "a": 1, "b": 2 })"},
      {Value{MapType{{"a", Value{MapType{{"b", Value{1.0}}}}}}}, R"({ "a": { "b": 1 } })"},
      {boundedRange,                                             "[1..3]"},
      {leftBoundedRange,                                         "[2..]"},
      {rightBoundedRange,                                        "[..5]"},
      {vec3,                                                     "vec(1, 2, 3)"},
      {bbox,                                                     "bbox(vec(1, 2, 3), vec(4, 5, 6))"},
      // prints exactly like a real Map would, since LazyMap is meant to be
      // indistinguishable from one
      {testLazyMap,                                              R"({ "name": "test", "number": 42 })"},
      {Value::Null,                                              "null"},
      {Value::Undefined,                                         "undefined"},
      }));
      // clang-format on

      CAPTURE(value);

      CHECK(value.asString() == expectedString);
    }

    SECTION("multiline")
    {
      // clang-format off
      const auto
      [value,                                                    expectedString] = GENERATE(table<Value, std::string>({
      {Value{true},                                              "true"},
      {Value{ArrayType{}},                                       "[]"},
      {Value{ArrayType{Value{1.0}, Value{2.0}}},                 "[\n\t1,\n\t2\n]"},
      {Value{ArrayType{Value{ArrayType{Value{1.0}}}}},           "[\n\t[\n\t\t1\n\t]\n]"},
      {Value{MapType{}},                                         "{}"},
      {Value{MapType{{"a", Value{1.0}}, {"b", Value{2.0}}}},     "{\n\t\"a\": 1,\n\t\"b\": 2\n}"},
      {Value{MapType{{"a", Value{MapType{{"b", Value{1.0}}}}}}}, "{\n\t\"a\": {\n\t\t\"b\": 1\n\t}\n}"},
      {testLazyMap,                                              "{\n\t\"name\": \"test\",\n\t\"number\": 42\n}"},
      }));
      // clang-format on

      CAPTURE(value);

      CHECK(value.asString(true) == expectedString);
    }
  }

  SECTION("appendToStream")
  {
    // defaults to multiline output, and appends rather than replaces
    auto str = std::ostringstream{};
    str << "prefix ";
    Value{ArrayType{Value{1.0}, Value{2.0}}}.appendToStream(str);
    CHECK(str.str() == "prefix [\n\t1,\n\t2\n]");

    // with legacy escaping, every backslash and quote is escaped, also in nested values
    auto legacyStr = std::ostringstream{};
    Value{ArrayType{
            Value{R"(a"b\c)"},
            Value{MapType{{"path", Value{R"(\\server\share)"}}}},
          }}
      .appendToStream(legacyStr, false, "", StringEscaping::Legacy);
    // MSVC cannot parse this raw string literal inline in CHECK
    const auto expected = R"(["a\"b\\c", { "path": "\\\\server\\share" }])";
    CHECK(legacyStr.str() == expected);
  }

  SECTION("contains")
  {
    SECTION("by index")
    {
      // clang-format off
      const auto
      [value,                             index, expectedResult] = GENERATE(table<Value, size_t, bool>({
      {Value{"ab"},                       0,     true},
      {Value{"ab"},                       1,     true},
      {Value{"ab"},                       2,     false},
      {Value{""},                         0,     false},

      {Value{ArrayType{Value{1.0}}},      0,     true},
      {Value{ArrayType{Value{1.0}}},      1,     false},
      {Value{ArrayType{}},                0,     false},

      // every other type is not indexable by an integer
      {Value{MapType{{"0", Value{1.0}}}}, 0,     false},
      {Value{true},                       0,     false},
      {Value{1.0},                        0,     false},
      {boundedRange,                      0,     false},
      {vec3,                              0,     false},
      {bbox,                              0,     false},
      {testLazyMap,                       0,     false},
      {Value::Null,                       0,     false},
      {Value::Undefined,                  0,     false},
      }));
      // clang-format on

      CAPTURE(value, index);

      CHECK(value.contains(index) == expectedResult);
    }

    SECTION("by key")
    {
      // clang-format off
      const auto
      [value,                             key,       expectedResult] = GENERATE(table<Value, std::string, Result<bool>>({
      {Value{MapType{{"a", Value{1.0}}}}, "a",       true},
      {Value{MapType{{"a", Value{1.0}}}}, "b",       false},
      {Value{MapType{}},                  "a",       false},
      {Value::Null,                       "a",       false},
      {testLazyMap,                       "name",    true},
      {testLazyMap,                       "number",  true},
      {testLazyMap,                       "missing", false},

      {Value{"ab"},                       "a",       Error{R"(At unknown location: Cannot dereference value '"ab"' of type 'String' as type 'Map')"}},
      {Value{ArrayType{}},                "a",       Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Map'"}},
      {Value::Undefined,                  "a",       Error{"At unknown location: Cannot dereference value 'undefined' of type 'Undefined' as type 'Map'"}},
      }));
      // clang-format on

      CAPTURE(value, key);

      CHECK(
        withEvaluationContext([&](auto&) { return value.contains(key); })
        == expectedResult);
    }
  }

  SECTION("keys")
  {
    // clang-format off
    const auto
    [value,                                                expectedResult] = GENERATE(table<Value, Result<std::vector<std::string>>>({
    {Value{MapType{{"b", Value{1.0}}, {"a", Value{2.0}}}}, std::vector<std::string>{"a", "b"}},
    {Value{MapType{}},                                     std::vector<std::string>{}},
    {Value::Null,                                          std::vector<std::string>{}},
    {testLazyMap,                                          std::vector<std::string>{"name", "number"}},

    {Value{ArrayType{}},                                   Error{"At unknown location: Cannot dereference value '[]' of type 'Array' as type 'Map'"}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(withEvaluationContext([&](auto&) { return value.keys(); }) == expectedResult);
  }

  SECTION("at")
  {
    SECTION("by index")
    {
      // clang-format off
      const auto
      [value,                                    index, expectedResult] = GENERATE(table<Value, size_t, Result<Value>>({
      {Value{"abc"},                             0,     Value{"a"}},
      {Value{"abc"},                             2,     Value{"c"}},
      {Value{"abc"},                             3,     Error{R"(At unknown location: Index 3 is out of bounds for '"abc"')"}},

      {Value{ArrayType{Value{1.0}, Value{"a"}}}, 0,     Value{1.0}},
      {Value{ArrayType{Value{1.0}, Value{"a"}}}, 1,     Value{"a"}},
      {Value{ArrayType{Value{1.0}, Value{"a"}}}, 2,     Error{R"(At unknown location: Index 2 is out of bounds for '[1, "a"]')"}},

      {Value{MapType{}},                         0,     Error{"At unknown location: 0 is not a compatible index for '{}'"}},
      {Value{true},                              0,     Error{"At unknown location: 0 is not a compatible index for 'true'"}},
      {Value{1.0},                               0,     Error{"At unknown location: 0 is not a compatible index for '1'"}},
      {boundedRange,                             0,     Error{"At unknown location: 0 is not a compatible index for '[1..3]'"}},
      {vec3,                                     0,     Error{"At unknown location: 0 is not a compatible index for 'vec(1, 2, 3)'"}},
      {bbox,                                     0,     Error{"At unknown location: 0 is not a compatible index for 'bbox(vec(1, 2, 3), vec(4, 5, 6))'"}},
      {testLazyMap,                              0,     Error{R"(At unknown location: 0 is not a compatible index for '{ "name": "test", "number": 42 }')"}},
      {Value::Null,                             0,     Error{"At unknown location: 0 is not a compatible index for 'null'"}},
      {Value::Undefined,                         0,     Error{"At unknown location: 0 is not a compatible index for 'undefined'"}},
      }));
      // clang-format on

      CAPTURE(value, index);

      CHECK(
        withEvaluationContext([&](auto&) { return value.at(index); }) == expectedResult);
    }

    SECTION("by key")
    {
      // clang-format off
      const auto
      [value,                             key,       expectedResult] = GENERATE(table<Value, std::string, Result<Value>>({
      {Value{MapType{{"a", Value{1.0}}}}, "a",       Value{1.0}},
      {Value{MapType{{"a", Value{1.0}}}}, "b",       Error{R"(At unknown location: Key 'b' not found in '{ "a": 1 }')"}},
      {testLazyMap,                       "name",    Value{"test"}},
      {testLazyMap,                       "missing", Error{R"(At unknown location: Key 'missing' not found in '{ "name": "test", "number": 42 }')"}},

      {Value{"abc"},                      "a",       Error{R"(At unknown location: 'a' is not a compatible key for '"abc"')"}},
      {Value{ArrayType{}},                "a",       Error{"At unknown location: 'a' is not a compatible key for '[]'"}},
      {Value{true},                       "a",       Error{"At unknown location: 'a' is not a compatible key for 'true'"}},
      {Value{1.0},                        "a",       Error{"At unknown location: 'a' is not a compatible key for '1'"}},
      {boundedRange,                      "a",       Error{"At unknown location: 'a' is not a compatible key for '[1..3]'"}},
      {vec3,                              "a",       Error{"At unknown location: 'a' is not a compatible key for 'vec(1, 2, 3)'"}},
      {bbox,                              "a",       Error{"At unknown location: 'a' is not a compatible key for 'bbox(vec(1, 2, 3), vec(4, 5, 6))'"}},
      {Value::Null,                       "a",       Error{"At unknown location: 'a' is not a compatible key for 'null'"}},
      {Value::Undefined,                  "a",       Error{"At unknown location: 'a' is not a compatible key for 'undefined'"}},
      }));
      // clang-format on

      CAPTURE(value, key);

      CHECK(
        withEvaluationContext([&](auto&) { return value.at(key); }) == expectedResult);
    }
  }

  SECTION("atOrDefault")
  {
    SECTION("by index")
    {
      // clang-format off
      const auto
      [value,                        index, defaultValue, expectedResult] = GENERATE(table<Value, size_t, std::optional<Value>, Result<Value>>({
      {Value{"abc"},                 0,     std::nullopt, Value{"a"}},
      {Value{"abc"},                 3,     std::nullopt, Value::Null},
      {Value{"abc"},                 3,     Value{"x"},   Value{"x"}},

      {Value{ArrayType{Value{1.0}}}, 0,     std::nullopt, Value{1.0}},
      {Value{ArrayType{Value{1.0}}}, 1,     std::nullopt, Value::Null},
      {Value{ArrayType{Value{1.0}}}, 1,     Value{"x"},   Value{"x"}},

      // a value that is not indexable by an integer still throws
      {Value{MapType{}},             0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for '{}'"}},
      {Value{true},                  0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for 'true'"}},
      {Value{1.0},                   0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for '1'"}},
      {boundedRange,                 0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for '[1..3]'"}},
      {vec3,                         0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for 'vec(1, 2, 3)'"}},
      {bbox,                         0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for 'bbox(vec(1, 2, 3), vec(4, 5, 6))'"}},
      {testLazyMap,                  0,     std::nullopt, Error{R"(At unknown location: 0 is not a compatible index for '{ "name": "test", "number": 42 }')"}},
      {Value::Null,                 0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for 'null'"}},
      {Value::Undefined,             0,     std::nullopt, Error{"At unknown location: 0 is not a compatible index for 'undefined'"}},
      }));
      // clang-format on

      CAPTURE(value, index, defaultValue);

      CHECK(
        withEvaluationContext([&](auto&) {
          return defaultValue ? value.atOrDefault(index, *defaultValue)
                              : value.atOrDefault(index);
        })
        == expectedResult);
    }

    SECTION("by key")
    {
      // clang-format off
      const auto
      [value,                             key,       defaultValue, expectedResult] = GENERATE(table<Value, std::string, std::optional<Value>, Result<Value>>({
      {Value{MapType{{"a", Value{1.0}}}}, "a",       std::nullopt, Value{1.0}},
      {Value{MapType{{"a", Value{1.0}}}}, "b",       std::nullopt, Value::Null},
      {Value{MapType{{"a", Value{1.0}}}}, "b",       Value{"x"},   Value{"x"}},
      {testLazyMap,                       "name",    std::nullopt, Value{"test"}},
      {testLazyMap,                       "missing", std::nullopt, Value::Null},
      {testLazyMap,                       "missing", Value{"x"},   Value{"x"}},

      {Value{"abc"},                      "a",       std::nullopt, Error{R"(At unknown location: 'a' is not a compatible key for '"abc"')"}},
      {Value{ArrayType{}},                "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for '[]'"}},
      {Value{true},                       "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for 'true'"}},
      {Value{1.0},                        "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for '1'"}},
      {boundedRange,                      "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for '[1..3]'"}},
      {vec3,                              "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for 'vec(1, 2, 3)'"}},
      {bbox,                              "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for 'bbox(vec(1, 2, 3), vec(4, 5, 6))'"}},
      {Value::Null,                       "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for 'null'"}},
      {Value::Undefined,                  "a",       std::nullopt, Error{"At unknown location: 'a' is not a compatible key for 'undefined'"}},
      }));
      // clang-format on

      CAPTURE(value, key, defaultValue);

      CHECK(
        withEvaluationContext([&](auto&) {
          return defaultValue ? value.atOrDefault(key, *defaultValue)
                              : value.atOrDefault(key);
        })
        == expectedResult);
    }
  }

  SECTION("location")
  {
    // clang-format off
    const auto
    [value,                                                                                    expectedLocation] = GENERATE(table<Value, std::optional<FileLocation>>({
    {Value{1.0},                                                                               std::nullopt},
    {Value{1.0}.producedBy(ExpressionNode{LiteralExpression{Value{1.0}}}),                     std::nullopt},
    {Value{1.0}.producedBy(ExpressionNode{LiteralExpression{Value{1.0}}, FileLocation{1, 2}}), FileLocation{1, 2}},
    {Value{1.0}.producedBy(ExpressionNode{LiteralExpression{Value{1.0}}, FileLocation{3}}),    FileLocation{3}},
    }));
    // clang-format on

    CAPTURE(value);

    CHECK(value.location() == expectedLocation);
  }

  SECTION("producedBy")
  {
    const auto location1 = FileLocation{1, 2};
    const auto location2 = FileLocation{3, 4};
    const auto expression1 = ExpressionNode{LiteralExpression{Value{1.0}}, location1};
    const auto expression2 = ExpressionNode{LiteralExpression{Value{2.0}}, location2};

    SECTION("by expression node")
    {
      const auto value = Value{1.0};

      const auto producedValue = value.producedBy(expression1);
      CHECK(producedValue == value);
      CHECK(producedValue.location() == location1);
      CHECK(value.location() == std::nullopt);

      CHECK(Value{1.0}.producedBy(expression1).location() == location1);

      // the first producer is kept
      CHECK(producedValue.producedBy(expression2).location() == location1);
      CHECK(
        Value{1.0}.producedBy(expression1).producedBy(expression2).location()
        == location1);

      // a producer without a location doesn't count
      CHECK(
        Value{1.0}
          .producedBy(ExpressionNode{LiteralExpression{Value{1.0}}})
          .producedBy(expression1)
          .location()
        == location1);
    }

    SECTION("by value")
    {
      const auto original = Value{2.0}.producedBy(expression1);
      const auto value = Value{1.0};

      const auto producedValue = value.producedBy(original);
      CHECK(producedValue == value);
      CHECK(producedValue.location() == location1);
      CHECK(value.location() == std::nullopt);

      CHECK(Value{1.0}.producedBy(original).location() == location1);

      // the first producer is kept
      CHECK(
        Value{1.0}.producedBy(expression2).producedBy(original).location() == location2);

      // an original without a producer has no effect
      CHECK(value.producedBy(Value{2.0}).location() == std::nullopt);
      CHECK(Value{1.0}.producedBy(Value{2.0}).location() == std::nullopt);
    }
  }

  SECTION("operator==")
  {
    SECTION("values")
    {
      // clang-format off
      const auto
      [lhs,                               rhs,                                                   expectedResult] = GENERATE(table<Value, Value, bool>({
      {Value{true},                       Value{true},                                           true},
      {Value{true},                       Value{false},                                          false},
      {Value{"a"},                        Value{"a"},                                            true},
      {Value{"a"},                        Value{"b"},                                            false},
      {Value{1.0},                        Value{1.0},                                            true},
      {Value{1.0},                        Value{2.0},                                            false},
      {Value{ArrayType{Value{1.0}}},      Value{ArrayType{Value{1.0}}},                          true},
      {Value{ArrayType{Value{1.0}}},      Value{ArrayType{}},                                    false},
      {Value{MapType{{"a", Value{1.0}}}}, Value{MapType{{"a", Value{1.0}}}},                     true},
      {Value{MapType{{"a", Value{1.0}}}}, Value{MapType{}},                                      false},
      {boundedRange,                      Value{RangeType{BoundedRange{1, 3}}},                  true},
      {boundedRange,                      leftBoundedRange,                                      false},
      {vec3,                              Value{Vec3Type{1, 2, 3}},                              true},
      {vec3,                              Value{Vec3Type{1, 2, 4}},                              false},
      {bbox,                              Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 6}}}, true},
      {bbox,                              Value{BBoxType{Vec3Type{1, 2, 3}, Vec3Type{4, 5, 7}}}, false},
      {testLazyMap,                       makeLazyMap(lazyMapTestObject, lazyMapTestFields),     true},
      {testLazyMap,                       makeLazyMap(LazyMapTestObject{}, lazyMapTestFields),   false},
      {Value::Null,                       Value{},                                               true},
      {Value::Undefined,                  Value{UndefinedType::Value},                           true},

      // values of different types are never equal
      {Value::Null,                       Value::Undefined,                                      false},
      {Value{1.0},                        Value{"1"},                                            false},
      {Value{true},                       Value{1.0},                                            false},
      {testLazyMap,                       testLazyMap.convertTo(ValueType::Map),                 false},
      }));
      // clang-format on

      CAPTURE(lhs, rhs);

      CHECK((lhs == rhs) == expectedResult);
    }

    SECTION("copies")
    {
      // a value is equal to a copy of itself, which shares its representation
      const auto value = Value{ArrayType{Value{1.0}}};
      const auto copy = value;
      CHECK(value == copy);

      // NaN is equal to NaN, so that a number is equal to a copy of itself even though
      // the copy doesn't share its representation
      const auto nan = std::numeric_limits<NumberType>::quiet_NaN();
      const auto number = Value{nan};
      const auto numberCopy = number;
      CHECK(number == numberCopy);
      CHECK_FALSE(Value{nan} == Value{1.0});
    }
  }

  SECTION("operator!=")
  {
    // clang-format off
    const auto
    [lhs,        rhs,        expectedResult] = GENERATE(table<Value, Value, bool>({
    {Value{1.0}, Value{1.0}, false},
    {Value{1.0}, Value{2.0}, true},
    }));
    // clang-format on

    CAPTURE(lhs, rhs);

    CHECK((lhs != rhs) == expectedResult);
  }

  SECTION("operator<<")
  {
    auto str = std::ostringstream{};
    str << Value{ArrayType{Value{1.0}, Value{2.0}}};
    CHECK(str.str() == "[\n\t1,\n\t2\n]");
  }
}

} // namespace tb::el
