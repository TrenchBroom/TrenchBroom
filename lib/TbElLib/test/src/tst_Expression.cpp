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
#include "el/ParseExpression.h"
#include "el/TestUtils.h"
#include "el/Value.h"
#include "el/VariableStore.h"

#include "kd/result_io.h" // IWYU pragma: keep

#include <fmt/ostream.h>

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

namespace tb::el
{
namespace
{

using V = Value;

auto evaluate(const std::string& expression, const MapType& variables = {})
{
  return withEvaluationContext(
    [&](auto& context) {
      return parseExpression(ParseMode::Strict, expression).value().evaluate(context);
    },
    VariableTable{variables});
}

auto tryEvaluate(const std::string& expression, const MapType& variables = {})
{
  return withEvaluationContext(
    [&](auto& context) {
      return parseExpression(ParseMode::Strict, expression).value().tryEvaluate(context);
    },
    VariableTable{variables});
}

std::vector<std::string> preorderVisit(const std::string& str)
{
  auto result = std::vector<std::string>{};

  parseExpression(ParseMode::Strict, str)
    .value()
    .accept(kdl::overload(
      [&](const LiteralExpression& literalExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(literalExpression)));
      },
      [&](const VariableExpression& variableExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(variableExpression)));
      },
      [&](const auto& thisLambda, const ArrayExpression& arrayExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(arrayExpression)));
        for (const auto& element : arrayExpression.elements)
        {
          element.accept(thisLambda);
        }
      },
      [&](const auto& thisLambda, const MapExpression& mapExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(mapExpression)));
        for (const auto& [key, element] : mapExpression.elements)
        {
          element.accept(thisLambda);
        }
      },
      [&](const auto& thisLambda, const UnaryExpression& unaryExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(unaryExpression)));
        unaryExpression.operand.accept(thisLambda);
      },
      [&](const auto& thisLambda, const BinaryExpression& binaryExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(binaryExpression)));
        binaryExpression.leftOperand.accept(thisLambda);
        binaryExpression.rightOperand.accept(thisLambda);
      },
      [&](const auto& thisLambda, const SubscriptExpression& subscriptExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(subscriptExpression)));
        subscriptExpression.leftOperand.accept(thisLambda);
        subscriptExpression.rightOperand.accept(thisLambda);
      },
      [&](const auto& thisLambda, const DotExpression& dotExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(dotExpression)));
        dotExpression.operand.accept(thisLambda);
      },
      [&](const auto& thisLambda, const CallExpression& callExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(callExpression)));
        for (const auto& argument : callExpression.arguments)
        {
          argument.accept(thisLambda);
        }
      },
      [&](const auto& thisLambda, const SwitchExpression& switchExpression) {
        result.push_back(fmt::format("{}", fmt::streamed(switchExpression)));
        for (const auto& caseExpression : switchExpression.cases)
        {
          caseExpression.accept(thisLambda);
        }
      }));
  return result;
}

} // namespace

TEST_CASE("Expression")
{
  SECTION("Value literals")
  {
    using T = std::tuple<std::string, Value>;

    // clang-format off
    const auto
    [expression,     expectedValue] = GENERATE(values<T>({
    {"true",         Value{true}},
    {"false",        Value{false}},
    {"'asdf'",       Value{"asdf"}},
    {"2",            Value{2}},
    {"-2",           Value{-2}},
    {"[2, 3]",       Value{ArrayType{Value{2}, Value{3}}}},
    {"{k1:2, k2:3}", Value{MapType{{"k1", Value{2}}, {"k2", Value{3}}}}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedValue);
  }

  SECTION("Variables")
  {
    using T = std::tuple<std::string, MapType, Value>;

    // clang-format off
    const auto
    [expression, variables,            expectedValue] = GENERATE(values<T>({
    {"x",        {{"x", Value{true}}}, Value{true}},
    {"x",        {{"y", Value{true}}}, Value::Undefined},
    {"x",        {{"x", Value{7}}},    Value{7}},
    {"x",        {},                   Value::Undefined},
    }));
    // clang-format on

    CAPTURE(expression, variables);

    CHECK(evaluate(expression, variables) == expectedValue);
  }

  SECTION("Arrays")
  {
    using T = std::tuple<std::string, MapType, ArrayType>;

    // clang-format off
    const auto
    [expression,  variables,               expectedValue] = GENERATE(values<T>({
    {"[]",        {},                      {}},
    {"[1, 2, 3]", {},                      {Value{1}, Value{2}, Value{3}}},
    {"[1, 2, x]", {{"x", Value{"test"}}},  {Value{1}, Value{2}, Value{"test"}}},
    }));
    // clang-format on

    CAPTURE(expression, variables);

    CHECK(evaluate(expression, variables) == Value{expectedValue});
  }

  SECTION("Maps")
  {
    using T = std::tuple<std::string, MapType, MapType>;

    // clang-format off
    const auto
    [expression,                     variables,          expectedValue] = GENERATE(values<T>({
    {"{}",                           {},                 {}},
    {"{k: true}",                    {},                 {{"k", Value{true}}}},
    {"{k1: true, k2: 3, k3: 3 + 7}", {},                 {{"k1", Value{true}}, {"k2", Value{3}}, {"k3", Value{10}}}},
    {"{k1: 'asdf', k2: x}",          {{"x", Value{55}}}, {{"k1", Value{"asdf"}}, {"k2", Value{55}}}},
    }));
    // clang-format on

    CAPTURE(expression, variables);

    CHECK(evaluate(expression, variables) == Value{expectedValue});
  }

  SECTION("Operators")
  {
    using T = std::tuple<std::string, Result<Value>>;

    // clang-format off
    const auto
    [expression,        expectedResult] = GENERATE(values<T>({
    // Unary plus
    {"+true",           Value{1}},
    {"+false",          Value{0}},
    {"+1",              Value{1}},
    {"+'test'",         Error{R"(At line 1, column 1: Cannot evaluate expression '+"test"': Invalid type String)"}},
    {"+'2'",            Value{2}},
    {"+null",           Error{R"(At line 1, column 1: Cannot evaluate expression '+null': Invalid type Null)"}},
    {"+[]",             Error{R"(At line 1, column 1: Cannot evaluate expression '+[]': Invalid type Array)"}},
    {"+{}",             Error{R"(At line 1, column 1: Cannot evaluate expression '+{}': Invalid type Map)"}},

    // Unary minus
    {"-true",           Value{-1}},
    {"-false",          Value{0}},
    {"-1",              Value{-1}},
    {"-'2'",            Value{-2}},
    {"-'test'",         Error{R"(At line 1, column 1: Cannot evaluate expression '-"test"': Invalid type String)"}},
    {"-null",           Error{R"(At line 1, column 1: Cannot evaluate expression '-null': Invalid type Null)"}},
    {"-[]",             Error{R"(At line 1, column 1: Cannot evaluate expression '-[]': Invalid type Array)"}},
    {"-{}",             Error{R"(At line 1, column 1: Cannot evaluate expression '-{}': Invalid type Map)"}},

    // Addition
    {"true + true",     Value{2}},
    {"false + 3",       Value{3}},
    {"true + 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'true + "test"': Invalid operand types Boolean and String)"}},
    {"true + '1.23'",   Value{2.23}},
    {"true + null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'true + null': Invalid operand types Boolean and Null)"}},
    {"true + []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true + []': Invalid operand types Boolean and Array)"}},
    {"true + {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true + {}': Invalid operand types Boolean and Map)"}},

    {"1 + true",        Value{2}},
    {"3 + -1",          Value{2}},
    {"1 + '1.23'",      Value{2.23}},
    {"1 + 'test'",      Error{R"(At line 1, column 3: Cannot evaluate expression '1 + "test"': Invalid operand types Number and String)"}},
    {"1 + null",        Error{R"(At line 1, column 3: Cannot evaluate expression '1 + null': Invalid operand types Number and Null)"}},
    {"1 + []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 + []': Invalid operand types Number and Array)"}},
    {"1 + {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 + {}': Invalid operand types Number and Map)"}},

    {"'test' + true",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" + true': Invalid operand types String and Boolean)"}},
    {"'test' + 2",      Error{R"(At line 1, column 8: Cannot evaluate expression '"test" + 2': Invalid operand types String and Number)"}},
    {"'1.23' + 2",      Value{3.23}},
    {"'this' + 'test'", Value{"thistest"}},
    {"'this' + '1.23'", Value{"this1.23"}},
    {"'test' + null",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" + null': Invalid operand types String and Null)"}},
    {"'test' + []",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" + []': Invalid operand types String and Array)"}},
    {"'test' + {}",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" + {}': Invalid operand types String and Map)"}},

    {"null + true",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null + true': Invalid operand types Null and Boolean)"}},
    {"null + 2",        Error{R"(At line 1, column 6: Cannot evaluate expression 'null + 2': Invalid operand types Null and Number)"}},
    {"null + 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'null + "test"': Invalid operand types Null and String)"}},
    {"null + null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null + null': Invalid operand types Null and Null)"}},
    {"null + []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null + []': Invalid operand types Null and Array)"}},
    {"null + {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null + {}': Invalid operand types Null and Map)"}},

    {"[] + true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] + true': Invalid operand types Array and Boolean)"}},
    {"[] + 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] + 2': Invalid operand types Array and Number)"}},
    {"[] + 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '[] + "test"': Invalid operand types Array and String)"}},
    {"[] + null",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] + null': Invalid operand types Array and Null)"}},
    {"[1, 2] + [2, 3]", Value{ArrayType{Value{1}, Value{2}, Value{2}, Value{3}}}},
    {"[] + {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] + {}': Invalid operand types Array and Map)"}},

    {"{} + true",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} + true': Invalid operand types Map and Boolean)"}},
    {"{} + 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} + 2': Invalid operand types Map and Number)"}},
    {"{} + 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '{} + "test"': Invalid operand types Map and String)"}},
    {"{} + null",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} + null': Invalid operand types Map and Null)"}},
    {"{} + []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} + []': Invalid operand types Map and Array)"}},
    {"{k1: 1, k2: 2, k3: 3} + {k3: 4, k4: 5}", Value{MapType{
        {"k1", Value{1}},
        {"k2", Value{2}},
        {"k3", Value{4}},
        {"k4", Value{5}},
    }}},

    // Subtraction
    {"true - true",     Value{0}},
    {"false - 3",       Value{-3}},
    {"true - 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'true - "test"': Invalid operand types Boolean and String)"}},
    {"true - '2.0'",    Value{-1.0}},
    {"true - null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'true - null': Invalid operand types Boolean and Null)"}},
    {"true - []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true - []': Invalid operand types Boolean and Array)"}},
    {"true - {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true - {}': Invalid operand types Boolean and Map)"}},

    {"1 - true",        Value{0}},
    {"3 - 1",           Value{2}},
    {"1 - 'test'",      Error{R"(At line 1, column 3: Cannot evaluate expression '1 - "test"': Invalid operand types Number and String)"}},
    {"1 - '2.23'",      Value{-1.23}},
    {"1 - null",        Error{R"(At line 1, column 3: Cannot evaluate expression '1 - null': Invalid operand types Number and Null)"}},
    {"1 - []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 - []': Invalid operand types Number and Array)"}},
    {"1 - {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 - {}': Invalid operand types Number and Map)"}},

    {"'test' - true",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" - true': Invalid operand types String and Boolean)"}},
    {"'test' - 2",      Error{R"(At line 1, column 8: Cannot evaluate expression '"test" - 2': Invalid operand types String and Number)"}},
    {"'3.23' - 2",      Value{1.23}},
    {"'this' - 'test'", Error{R"(At line 1, column 8: Cannot evaluate expression '"this" - "test"': Invalid operand types String and String)"}},
    {"'test' - null",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" - null': Invalid operand types String and Null)"}},
    {"'test' - []",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" - []': Invalid operand types String and Array)"}},
    {"'test' - {}",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" - {}': Invalid operand types String and Map)"}},

    {"null - true",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null - true': Invalid operand types Null and Boolean)"}},
    {"null - 2",        Error{R"(At line 1, column 6: Cannot evaluate expression 'null - 2': Invalid operand types Null and Number)"}},
    {"null - 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'null - "test"': Invalid operand types Null and String)"}},
    {"null - null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null - null': Invalid operand types Null and Null)"}},
    {"null - []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null - []': Invalid operand types Null and Array)"}},
    {"null - {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null - {}': Invalid operand types Null and Map)"}},

    {"[] - true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] - true': Invalid operand types Array and Boolean)"}},
    {"[] - 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] - 2': Invalid operand types Array and Number)"}},
    {"[] - 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '[] - "test"': Invalid operand types Array and String)"}},
    {"[] - null",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] - null': Invalid operand types Array and Null)"}},
    {"[] - []",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] - []': Invalid operand types Array and Array)"}},
    {"[] - {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] - {}': Invalid operand types Array and Map)"}},

    {"{} - true",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} - true': Invalid operand types Map and Boolean)"}},
    {"{} - 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} - 2': Invalid operand types Map and Number)"}},
    {"{} - 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '{} - "test"': Invalid operand types Map and String)"}},
    {"{} - null",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} - null': Invalid operand types Map and Null)"}},
    {"{} - []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} - []': Invalid operand types Map and Array)"}},
    {"{} - {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} - {}': Invalid operand types Map and Map)"}},

    // Multiplication
    {"true * true",     Value{1}},
    {"true * false",    Value{0}},
    {"true * 3",        Value{3}},
    {"true * '3'",      Value{3}},
    {"true * 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'true * "test"': Invalid operand types Boolean and String)"}},
    {"true * null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'true * null': Invalid operand types Boolean and Null)"}},
    {"true * []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true * []': Invalid operand types Boolean and Array)"}},
    {"true * {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true * {}': Invalid operand types Boolean and Map)"}},

    {"1 * true",        Value{1}},
    {"3 * 2",           Value{6}},
    {"1 * 'test'",      Error{R"(At line 1, column 3: Cannot evaluate expression '1 * "test"': Invalid operand types Number and String)"}},
    {"2 * '2.23'",      Value{4.46}},
    {"1 * null",        Error{R"(At line 1, column 3: Cannot evaluate expression '1 * null': Invalid operand types Number and Null)"}},
    {"1 * []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 * []': Invalid operand types Number and Array)"}},
    {"1 * {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 * {}': Invalid operand types Number and Map)"}},

    {"'test' * true",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" * true': Invalid operand types String and Boolean)"}},
    {"'test' * 2",      Error{R"(At line 1, column 8: Cannot evaluate expression '"test" * 2': Invalid operand types String and Number)"}},
    {"'1.23' * 2",      Value{2.46}},
    {"'this' * 'test'", Error{R"(At line 1, column 8: Cannot evaluate expression '"this" * "test"': Invalid operand types String and String)"}},
    {"'test' * null",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" * null': Invalid operand types String and Null)"}},
    {"'test' * []",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" * []': Invalid operand types String and Array)"}},
    {"'test' * {}",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" * {}': Invalid operand types String and Map)"}},

    {"null * true",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null * true': Invalid operand types Null and Boolean)"}},
    {"null * 2",        Error{R"(At line 1, column 6: Cannot evaluate expression 'null * 2': Invalid operand types Null and Number)"}},
    {"null * 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'null * "test"': Invalid operand types Null and String)"}},
    {"null * null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null * null': Invalid operand types Null and Null)"}},
    {"null * []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null * []': Invalid operand types Null and Array)"}},
    {"null * {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null * {}': Invalid operand types Null and Map)"}},

    {"[] * true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] * true': Invalid operand types Array and Boolean)"}},
    {"[] * 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] * 2': Invalid operand types Array and Number)"}},
    {"[] * 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '[] * "test"': Invalid operand types Array and String)"}},
    {"[] * null",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] * null': Invalid operand types Array and Null)"}},
    {"[] * []",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] * []': Invalid operand types Array and Array)"}},
    {"[] * {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] * {}': Invalid operand types Array and Map)"}},

    {"{} * true",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} * true': Invalid operand types Map and Boolean)"}},
    {"{} * 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} * 2': Invalid operand types Map and Number)"}},
    {"{} * 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '{} * "test"': Invalid operand types Map and String)"}},
    {"{} * null",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} * null': Invalid operand types Map and Null)"}},
    {"{} * []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} * []': Invalid operand types Map and Array)"}},
    {"{} * {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} * {}': Invalid operand types Map and Map)"}},

    // Division
    {"true / true",     Value{1}},
    {"true / false",    Value{std::numeric_limits<NumberType>::infinity()}},
    {"true / 3",        Value{1.0/3.0}},
    {"true / '2'",      Value{1.0/2.0}},
    {"true / 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'true / "test"': Invalid operand types Boolean and String)"}},
    {"true / null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'true / null': Invalid operand types Boolean and Null)"}},
    {"true / []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true / []': Invalid operand types Boolean and Array)"}},
    {"true / {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true / {}': Invalid operand types Boolean and Map)"}},

    {"1 / true",        Value{1}},
    {"3 / 2",           Value{1.5}},
    {"1 / 'test'",      Error{R"(At line 1, column 3: Cannot evaluate expression '1 / "test"': Invalid operand types Number and String)"}},
    {"1 / '3.0'",       Value{1.0/3.0}},
    {"1 / null",        Error{R"(At line 1, column 3: Cannot evaluate expression '1 / null': Invalid operand types Number and Null)"}},
    {"1 / []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 / []': Invalid operand types Number and Array)"}},
    {"1 / {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 / {}': Invalid operand types Number and Map)"}},

    {"'test' / true",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" / true': Invalid operand types String and Boolean)"}},
    {"'test' / 2",      Error{R"(At line 1, column 8: Cannot evaluate expression '"test" / 2': Invalid operand types String and Number)"}},
    {"'3' / 2",         Value{3.0/2.0}},
    {"'this' / 'test'", Error{R"(At line 1, column 8: Cannot evaluate expression '"this" / "test"': Invalid operand types String and String)"}},
    {"'test' / null",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" / null': Invalid operand types String and Null)"}},
    {"'test' / []",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" / []': Invalid operand types String and Array)"}},
    {"'test' / {}",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" / {}': Invalid operand types String and Map)"}},

    {"null / true",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null / true': Invalid operand types Null and Boolean)"}},
    {"null / 2",        Error{R"(At line 1, column 6: Cannot evaluate expression 'null / 2': Invalid operand types Null and Number)"}},
    {"null / 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'null / "test"': Invalid operand types Null and String)"}},
    {"null / null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null / null': Invalid operand types Null and Null)"}},
    {"null / []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null / []': Invalid operand types Null and Array)"}},
    {"null / {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null / {}': Invalid operand types Null and Map)"}},

    {"[] / true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] / true': Invalid operand types Array and Boolean)"}},
    {"[] / 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] / 2': Invalid operand types Array and Number)"}},
    {"[] / 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '[] / "test"': Invalid operand types Array and String)"}},
    {"[] / null",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] / null': Invalid operand types Array and Null)"}},
    {"[] / []",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] / []': Invalid operand types Array and Array)"}},
    {"[] / {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] / {}': Invalid operand types Array and Map)"}},

    {"{} / true",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} / true': Invalid operand types Map and Boolean)"}},
    {"{} / 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} / 2': Invalid operand types Map and Number)"}},
    {"{} / 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '{} / "test"': Invalid operand types Map and String)"}},
    {"{} / null",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} / null': Invalid operand types Map and Null)"}},
    {"{} / []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} / []': Invalid operand types Map and Array)"}},
    {"{} / {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} / {}': Invalid operand types Map and Map)"}},


    // Modulus
    {"true % true",     Value{0}},
    {"true % -2",       Value{1}},
    {"true % 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'true % "test"': Invalid operand types Boolean and String)"}},
    {"true % '1'",      Value{0}},
    {"true % null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'true % null': Invalid operand types Boolean and Null)"}},
    {"true % []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true % []': Invalid operand types Boolean and Array)"}},
    {"true % {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true % {}': Invalid operand types Boolean and Map)"}},

    {"3 % -2",          Value{1}},
    {"3 % '-2'",        Value{1}},
    {"1 % 'test'",      Error{R"(At line 1, column 3: Cannot evaluate expression '1 % "test"': Invalid operand types Number and String)"}},
    {"1 % null",        Error{R"(At line 1, column 3: Cannot evaluate expression '1 % null': Invalid operand types Number and Null)"}},
    {"1 % []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 % []': Invalid operand types Number and Array)"}},
    {"1 % {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 % {}': Invalid operand types Number and Map)"}},

    {"'test' % true",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" % true': Invalid operand types String and Boolean)"}},
    {"'test' % 2",      Error{R"(At line 1, column 8: Cannot evaluate expression '"test" % 2': Invalid operand types String and Number)"}},
    {"'3' % 2",         Value{1}},
    {"'this' % 'test'", Error{R"(At line 1, column 8: Cannot evaluate expression '"this" % "test"': Invalid operand types String and String)"}},
    {"'test' % null",   Error{R"(At line 1, column 8: Cannot evaluate expression '"test" % null': Invalid operand types String and Null)"}},
    {"'test' % []",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" % []': Invalid operand types String and Array)"}},
    {"'test' % {}",     Error{R"(At line 1, column 8: Cannot evaluate expression '"test" % {}': Invalid operand types String and Map)"}},

    {"null % true",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null % true': Invalid operand types Null and Boolean)"}},
    {"null % 2",        Error{R"(At line 1, column 6: Cannot evaluate expression 'null % 2': Invalid operand types Null and Number)"}},
    {"null % 'test'",   Error{R"(At line 1, column 6: Cannot evaluate expression 'null % "test"': Invalid operand types Null and String)"}},
    {"null % null",     Error{R"(At line 1, column 6: Cannot evaluate expression 'null % null': Invalid operand types Null and Null)"}},
    {"null % []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null % []': Invalid operand types Null and Array)"}},
    {"null % {}",       Error{R"(At line 1, column 6: Cannot evaluate expression 'null % {}': Invalid operand types Null and Map)"}},

    {"[] % true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] % true': Invalid operand types Array and Boolean)"}},
    {"[] % 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] % 2': Invalid operand types Array and Number)"}},
    {"[] % 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '[] % "test"': Invalid operand types Array and String)"}},
    {"[] % null",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] % null': Invalid operand types Array and Null)"}},
    {"[] % []",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] % []': Invalid operand types Array and Array)"}},
    {"[] % {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] % {}': Invalid operand types Array and Map)"}},

    {"{} % true",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} % true': Invalid operand types Map and Boolean)"}},
    {"{} % 2",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} % 2': Invalid operand types Map and Number)"}},
    {"{} % 'test'",     Error{R"(At line 1, column 4: Cannot evaluate expression '{} % "test"': Invalid operand types Map and String)"}},
    {"{} % null",       Error{R"(At line 1, column 4: Cannot evaluate expression '{} % null': Invalid operand types Map and Null)"}},
    {"{} % []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} % []': Invalid operand types Map and Array)"}},
    {"{} % {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} % {}': Invalid operand types Map and Map)"}},

    // Logical negation
    {"!true",           Value{false}},
    {"!false",          Value{true}},
    {"!1",              Error{R"(At line 1, column 1: Cannot evaluate expression '!1': Invalid type Number)"}},
    {"!'test'",         Error{R"(At line 1, column 1: Cannot evaluate expression '!"test"': Invalid type String)"}},
    {"!null",           Error{R"(At line 1, column 1: Cannot evaluate expression '!null': Invalid type Null)"}},
    {"![]",             Error{R"(At line 1, column 1: Cannot evaluate expression '![]': Invalid type Array)"}},
    {"!{}",             Error{R"(At line 1, column 1: Cannot evaluate expression '!{}': Invalid type Map)"}},

    // Logical conjunction
    {"false && false",  Value{false}},
    {"false && true",   Value{false}},
    {"true && false",   Value{false}},
    {"true && true",    Value{true}},
    {"null && true",    Value{false}},
    {"true && null",    Value{false}},
    {"null && null",    Value{false}},

    // Logical disjunction
    {"false || false",  Value{false}},
    {"false || true",   Value{true}},
    {"true || false",   Value{true}},
    {"true || true",    Value{true}},
    {"null || true",    Value{true}},
    {"false || null",   Value{false}},
    {"null || null",    Value{false}},

    // Undefined propagates through the logical operators
    {"undefined && true",  Value::Undefined},
    {"true && undefined",  Value::Undefined},
    {"undefined || true",  Value::Undefined},
    {"false || undefined", Value::Undefined},

    // A right operand that is neither boolean nor null is an error
    {"true && 1",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true && 1': Invalid operand types Boolean and Number)"}},
    {"true && 'a'",     Error{R"(At line 1, column 6: Cannot evaluate expression 'true && "a"': Invalid operand types Boolean and String)"}},
    {"false || []",     Error{R"(At line 1, column 7: Cannot evaluate expression 'false || []': Invalid operand types Boolean and Array)"}},

    // A left operand that is neither boolean nor null is an error too, and the error
    // names the type of the right operand rather than a placeholder
    {"1 && true",       Error{R"(At line 1, column 3: Cannot evaluate expression '1 && true': Invalid operand types Number and Boolean)"}},
    {"'a' || false",    Error{R"(At line 1, column 5: Cannot evaluate expression '"a" || false': Invalid operand types String and Boolean)"}},
    {"[1] && true",     Error{R"(At line 1, column 5: Cannot evaluate expression '[1] && true': Invalid operand types Array and Boolean)"}},
    {"{} || false",     Error{R"(At line 1, column 4: Cannot evaluate expression '{} || false': Invalid operand types Map and Boolean)"}},
    {"1 && 'a'",        Error{R"(At line 1, column 3: Cannot evaluate expression '1 && "a"': Invalid operand types Number and String)"}},

    // Undefined on either side wins, whatever the other side is
    {"1 && undefined",  Value::Undefined},
    {"'a' || undefined",Value::Undefined},

    // Logical short circuit evaluation
    {"false && x[-1]",  Value{false}},
    {"true || x[-1]",   Value{true}},

    // Undefined propagates through the unary and algebraic operators
    {"+undefined",      Value::Undefined},
    {"-undefined",      Value::Undefined},
    {"!undefined",      Value::Undefined},
    {"~undefined",      Value::Undefined},
    {"1 + undefined",   Value::Undefined},
    {"undefined + 1",   Value::Undefined},
    {"1 - undefined",   Value::Undefined},
    {"1 * undefined",   Value::Undefined},
    {"1 / undefined",   Value::Undefined},
    {"1 % undefined",   Value::Undefined},
    {"1 & undefined",   Value::Undefined},
    {"undefined & 1",   Value::Undefined},
    {"1 ^ undefined",   Value::Undefined},
    {"1 | undefined",   Value::Undefined},
    {"1 << undefined",  Value::Undefined},
    {"1 >> undefined",  Value::Undefined},

    // Bitwise negation
    {"~23423",          Value{~23423}},
    {"~23423.1",        Value{~23423}},
    {"~23423.8",        Value{~23423}},
    {"~true",           Error{R"(At line 1, column 1: Cannot evaluate expression '~true': Invalid type Boolean)"}},
    {"~'23423'",        Value{~23423}},
    {"~'asdf'",         Error{R"(At line 1, column 1: Cannot evaluate expression '~"asdf"': Invalid type String)"}},
    {"~null",           Error{R"(At line 1, column 1: Cannot evaluate expression '~null': Invalid type Null)"}},
    {"~[]",             Error{R"(At line 1, column 1: Cannot evaluate expression '~[]': Invalid type Array)"}},
    {"~{}",             Error{R"(At line 1, column 1: Cannot evaluate expression '~{}': Invalid type Map)"}},

    // Bitwise and
    {"0 & 0",           Value{0 & 0}},
    {"123 & 456",       Value{123 & 456}},
    {"true & 123",      Value{1 & 123}},
    {"123 & true",      Value{123 & 1}},
    {"'asdf' & 123",    Error{R"(At line 1, column 8: Cannot evaluate expression '"asdf" & 123': Invalid operand types String and Number)"}},
    {"'456' & 123",     Value{456 & 123}},
    {"123 & 'asdf'",    Error{R"(At line 1, column 5: Cannot evaluate expression '123 & "asdf"': Invalid operand types Number and String)"}},
    {"123 & '456'",     Value{123 & 456}},
    {"null & 123",      Value{0 & 123}},
    {"123 & null",      Value{123 & 0}},
    {"[] & 123",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] & 123': Invalid operand types Array and Number)"}},
    {"123 & []",        Error{R"(At line 1, column 5: Cannot evaluate expression '123 & []': Invalid operand types Number and Array)"}},
    {"{} & 123",        Error{R"(At line 1, column 4: Cannot evaluate expression '{} & 123': Invalid operand types Map and Number)"}},
    {"123 & {}",        Error{R"(At line 1, column 5: Cannot evaluate expression '123 & {}': Invalid operand types Number and Map)"}},

    // Bitwise or
    {"0 | 0",           Value{0 | 0}},
    {"123 | 456",       Value{123 | 456}},
    {"true | 123",      Value{1 | 123}},
    {"123 | true",      Value{123 | 1}},
    {"'asdf' | 123",    Error{R"(At line 1, column 8: Cannot evaluate expression '"asdf" | 123': Invalid operand types String and Number)"}},
    {"'456' | 123",     Value{456 | 123}},
    {"123 | 'asdf'",    Error{R"(At line 1, column 5: Cannot evaluate expression '123 | "asdf"': Invalid operand types Number and String)"}},
    {"123 | '456'",     Value{123 | 456}},
    {"null | 123",      Value{0 | 123}},
    {"123 | null",      Value{123 | 0}},
    {"[] | 123",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] | 123': Invalid operand types Array and Number)"}},
    {"123 | []",        Error{R"(At line 1, column 5: Cannot evaluate expression '123 | []': Invalid operand types Number and Array)"}},
    {"{} | 123",        Error{R"(At line 1, column 4: Cannot evaluate expression '{} | 123': Invalid operand types Map and Number)"}},
    {"123 | {}",        Error{R"(At line 1, column 5: Cannot evaluate expression '123 | {}': Invalid operand types Number and Map)"}},

    // Bitwise xor
    {"0 ^ 0",           Value{0 ^ 0}},
    {"123 ^ 456",       Value{123 ^ 456}},
    {"true ^ 123",      Value{1 ^ 123}},
    {"123 ^ true",      Value{123 ^ 1}},
    {"'asdf' ^ 123",    Error{R"(At line 1, column 8: Cannot evaluate expression '"asdf" ^ 123': Invalid operand types String and Number)"}},
    {"'456' ^ 123",     Value{456 ^ 123}},
    {"123 ^ 'asdf'",    Error{R"(At line 1, column 5: Cannot evaluate expression '123 ^ "asdf"': Invalid operand types Number and String)"}},
    {"123 ^ '456'",     Value{123 ^ 456}},
    {"null ^ 123",      Value{0 ^ 123}},
    {"123 ^ null",      Value{123 ^ 0}},
    {"[] ^ 123",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] ^ 123': Invalid operand types Array and Number)"}},
    {"123 ^ []",        Error{R"(At line 1, column 5: Cannot evaluate expression '123 ^ []': Invalid operand types Number and Array)"}},
    {"{} ^ 123",        Error{R"(At line 1, column 4: Cannot evaluate expression '{} ^ 123': Invalid operand types Map and Number)"}},
    {"123 ^ {}",        Error{R"(At line 1, column 5: Cannot evaluate expression '123 ^ {}': Invalid operand types Number and Map)"}},

    // Bitwise shift left
    {"1 << 2",          Value{1 << 2}},
    {"true << 2",       Value{1 << 2}},
    {"1 << false",      Value{1 << 0}},
    {"'asdf' << 2",     Error{R"(At line 1, column 8: Cannot evaluate expression '"asdf" << 2': Invalid operand types String and Number)"}},
    {"'1' << 2",        Value{1 << 2}},
    {"1 << 'asdf'",     Error{R"(At line 1, column 3: Cannot evaluate expression '1 << "asdf"': Invalid operand types Number and String)"}},
    {"1 << '2'",        Value{1 << 2}},
    {"null << 2",       Value{0 << 2}},
    {"1 << null",       Value{1 << 0}},
    {"[] << 2",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] << 2': Invalid operand types Array and Number)"}},
    {"1 << []",         Error{R"(At line 1, column 3: Cannot evaluate expression '1 << []': Invalid operand types Number and Array)"}},
    {"{} << 2",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} << 2': Invalid operand types Map and Number)"}},
    {"1 << {}",         Error{R"(At line 1, column 3: Cannot evaluate expression '1 << {}': Invalid operand types Number and Map)"}},

    // Bitwise shift right
    {"1 >> 2",          Value{1 >> 2}},
    {"true >> 2",       Value{1 >> 2}},
    {"1 >> false",      Value{1 >> 0}},
    {"'asdf' >> 2",     Error{R"(At line 1, column 8: Cannot evaluate expression '"asdf" >> 2': Invalid operand types String and Number)"}},
    {"'1' >> 2",        Value{1 >> 2}},
    {"1 >> 'asdf'",     Error{R"(At line 1, column 3: Cannot evaluate expression '1 >> "asdf"': Invalid operand types Number and String)"}},
    {"1 >> '2'",        Value{1 >> 2}},
    {"null >> 2",       Value{0 >> 2}},
    {"1 >> null",       Value{1 >> 0}},
    {"[] >> 2",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] >> 2': Invalid operand types Array and Number)"}},
    {"1 >> []",         Error{R"(At line 1, column 3: Cannot evaluate expression '1 >> []': Invalid operand types Number and Array)"}},
    {"{} >> 2",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} >> 2': Invalid operand types Map and Number)"}},
    {"1 >> {}",         Error{R"(At line 1, column 3: Cannot evaluate expression '1 >> {}': Invalid operand types Number and Map)"}},

    // Comparison
    {"false < false",   Value{false}},
    {"false < true",    Value{true}},
    {"true < false",    Value{false}},
    {"true < true",     Value{false}},

    {"false < 0",       Value{false}},
    {"false < 1",       Value{true}},
    {"false < 'true'",  Value{true}},
    {"false < 'false'", Value{false}},
    {"false < ''",      Value{false}},
    {"false < null",    Value{false}},
    {"false < []",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false < []': Invalid operand types Boolean and Array)"}},
    {"false < {}",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false < {}': Invalid operand types Boolean and Map)"}},

    {"0 < 0",           Value{false}},
    {"0 < 1",           Value{true}},
    {"0 < 'true'",      Error{R"(At line 1, column 3: Cannot evaluate expression '0 < "true"': At line 1, column 5: Cannot convert value '"true"' of type 'String' to type 'Number')"}},
    {"0 < 'false'",     Error{R"(At line 1, column 3: Cannot evaluate expression '0 < "false"': At line 1, column 5: Cannot convert value '"false"' of type 'String' to type 'Number')"}},
    {"0 < ''",          Value{false}},
    {"0 < '0'",         Value{false}},
    {"0 < '1'",         Value{true}},
    {"0 < null",        Value{false}},
    {"0 < []",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 < []': Invalid operand types Number and Array)"}},
    {"0 < {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 < {}': Invalid operand types Number and Map)"}},

    {"'a' < 0",         Error{R"(At line 1, column 5: Cannot evaluate expression '"a" < 0': At line 1, column 1: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'a' < 1",         Error{R"(At line 1, column 5: Cannot evaluate expression '"a" < 1': At line 1, column 1: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'a' < 'true'",    Value{true}},
    {"'a' < 'false'",   Value{true}},
    {"'a' < ''",        Value{false}},
    {"'a' < 'b'",       Value{true}},
    {"'a' < 'a'",       Value{false}},
    {"'aa' < 'ab'",     Value{true}},
    {"'a' < null",      Value{false}},
    {"'a' < []",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" < []': Invalid operand types String and Array)"}},
    {"'a' < {}",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" < {}': Invalid operand types String and Map)"}},
    {"'0' < 1",         Value{true}},
    {"'1' < 0",         Value{false}},

    {"null < true",     Value{true}},
    {"null < false",    Value{true}},
    {"null < 0",        Value{true}},
    {"null < 1",        Value{true}},
    {"null < ''",       Value{true}},
    {"null < 'a'",      Value{true}},
    {"null < null",     Value{false}},
    {"null < []",       Value{true}},
    {"null < {}",       Value{true}},

    {"[] < true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] < true': Invalid operand types Array and Boolean)"}},
    {"[] < false",      Error{R"(At line 1, column 4: Cannot evaluate expression '[] < false': Invalid operand types Array and Boolean)"}},
    {"[] < 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] < 0': Invalid operand types Array and Number)"}},
    {"[] < 1",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] < 1': Invalid operand types Array and Number)"}},
    {"[] < ''",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] < ""': Invalid operand types Array and String)"}},
    {"[] < 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] < "a"': Invalid operand types Array and String)"}},
    {"[] < null",       Value{false}},
    {"[] < []",         Value{false}},
    {"[1] < [1]",       Value{false}},
    {"[1] < [2]",       Value{true}},
    {"[1] < [1,2]",     Value{true}},
    {"[1,2] < [1,2]",   Value{false}},
    {"[1,2] < [1,2,3]", Value{true}},
    {"[1,2,3] < [1,2]", Value{false}},
    {"[] < {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] < {}': Invalid operand types Array and Map)"}},

    {"{} < true",             Error{R"(At line 1, column 4: Cannot evaluate expression '{} < true': Invalid operand types Map and Boolean)"}},
    {"{} < false",            Error{R"(At line 1, column 4: Cannot evaluate expression '{} < false': Invalid operand types Map and Boolean)"}},
    {"{} < 0",                Error{R"(At line 1, column 4: Cannot evaluate expression '{} < 0': Invalid operand types Map and Number)"}},
    {"{} < 1",                Error{R"(At line 1, column 4: Cannot evaluate expression '{} < 1': Invalid operand types Map and Number)"}},
    {"{} < ''",               Error{R"(At line 1, column 4: Cannot evaluate expression '{} < ""': Invalid operand types Map and String)"}},
    {"{} < 'a'",              Error{R"(At line 1, column 4: Cannot evaluate expression '{} < "a"': Invalid operand types Map and String)"}},
    {"{} < null",             Value{false}},
    {"{} < []",               Error{R"(At line 1, column 4: Cannot evaluate expression '{} < []': Invalid operand types Map and Array)"}},
    {"{} < {}",               Value{false}},
    {"{k1:1} < {k1:1}",       Value{false}},
    {"{k1:1} < {k2:1}",       Value{true}},
    {"{k2:1} < {k1:1}",       Value{false}},
    {"{k1:1} < {k1:2}",       Value{true}},
    {"{k1:1} < {k1:1, k2:2}", Value{true}},
    {"{k1:1} < {k1:2, k2:2}", Value{true}},

    {"false <= false",   Value{true}},
    {"false <= true",    Value{true}},
    {"true <= false",    Value{false}},
    {"true <= true",     Value{true}},

    {"false <= 0",       Value{true}},
    {"false <= 1",       Value{true}},
    {"false <= 'true'",  Value{true}},
    {"false <= 'false'", Value{true}},
    {"false <= ''",      Value{true}},
    {"false <= null",    Value{false}},
    {"false <= []",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false <= []': Invalid operand types Boolean and Array)"}},
    {"false <= {}",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false <= {}': Invalid operand types Boolean and Map)"}},

    {"0 <= 0",           Value{true}},
    {"0 <= 1",           Value{true}},
    {"0 <= 'true'",      Error{R"(At line 1, column 3: Cannot evaluate expression '0 <= "true"': At line 1, column 6: Cannot convert value '"true"' of type 'String' to type 'Number')"}},
    {"0 <= 'false'",     Error{R"(At line 1, column 3: Cannot evaluate expression '0 <= "false"': At line 1, column 6: Cannot convert value '"false"' of type 'String' to type 'Number')"}},
    {"0 <= ''",          Value{true}},
    {"0 <= '0'",         Value{true}},
    {"0 <= '1'",         Value{true}},
    {"0 <= null",        Value{false}},
    {"0 <= []",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 <= []': Invalid operand types Number and Array)"}},
    {"0 <= {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 <= {}': Invalid operand types Number and Map)"}},

    {"'a' <= 0",         Error{R"(At line 1, column 5: Cannot evaluate expression '"a" <= 0': At line 1, column 1: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'a' <= 1",         Error{R"(At line 1, column 5: Cannot evaluate expression '"a" <= 1': At line 1, column 1: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'a' <= 'true'",    Value{true}},
    {"'a' <= 'false'",   Value{true}},
    {"'a' <= ''",        Value{false}},
    {"'a' <= 'b'",       Value{true}},
    {"'a' <= 'a'",       Value{true}},
    {"'aa' <= 'ab'",     Value{true}},
    {"'a' <= null",      Value{false}},
    {"'a' <= []",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" <= []': Invalid operand types String and Array)"}},
    {"'a' <= {}",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" <= {}': Invalid operand types String and Map)"}},
    {"'0' <= 1",         Value{true}},
    {"'1' <= 0",         Value{false}},

    {"null <= true",     Value{true}},
    {"null <= false",    Value{true}},
    {"null <= 0",        Value{true}},
    {"null <= 1",        Value{true}},
    {"null <= ''",       Value{true}},
    {"null <= 'a'",      Value{true}},
    {"null <= null",     Value{true}},
    {"null <= []",       Value{true}},
    {"null <= {}",       Value{true}},

    {"[] <= true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= true': Invalid operand types Array and Boolean)"}},
    {"[] <= false",      Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= false': Invalid operand types Array and Boolean)"}},
    {"[] <= 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= 0': Invalid operand types Array and Number)"}},
    {"[] <= 1",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= 1': Invalid operand types Array and Number)"}},
    {"[] <= ''",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= ""': Invalid operand types Array and String)"}},
    {"[] <= 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= "a"': Invalid operand types Array and String)"}},
    {"[] <= null",       Value{false}},
    {"[] <= []",         Value{true}},
    {"[1] <= [1]",       Value{true}},
    {"[1] <= [2]",       Value{true}},
    {"[1] <= [1,2]",     Value{true}},
    {"[1,2] <= [1,2]",   Value{true}},
    {"[1,2] <= [1,2,3]", Value{true}},
    {"[1,2,3] <= [1,2]", Value{false}},
    {"[] <= {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] <= {}': Invalid operand types Array and Map)"}},

    {"{} <= true",             Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= true': Invalid operand types Map and Boolean)"}},
    {"{} <= false",            Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= false': Invalid operand types Map and Boolean)"}},
    {"{} <= 0",                Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= 0': Invalid operand types Map and Number)"}},
    {"{} <= 1",                Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= 1': Invalid operand types Map and Number)"}},
    {"{} <= ''",               Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= ""': Invalid operand types Map and String)"}},
    {"{} <= 'a'",              Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= "a"': Invalid operand types Map and String)"}},
    {"{} <= null",             Value{false}},
    {"{} <= []",               Error{R"(At line 1, column 4: Cannot evaluate expression '{} <= []': Invalid operand types Map and Array)"}},
    {"{} <= {}",               Value{true}},
    {"{k1:1} <= {k1:1}",       Value{true}},
    {"{k1:1} <= {k2:1}",       Value{true}},
    {"{k2:1} <= {k1:1}",       Value{false}},
    {"{k1:1} <= {k1:2}",       Value{true}},
    {"{k1:1} <= {k1:1, k2:2}", Value{true}},
    {"{k1:1} <= {k1:2, k2:2}", Value{true}},

    {"false > false",   Value{false}},
    {"true > false",    Value{true}},
    {"false > true",    Value{false}},
    {"true > true",     Value{false}},

    {"0 > false",       Value{false}},
    {"1 > false",       Value{true}},
    {"'true' > false",  Value{true}},
    {"'false' > false", Value{false}},
    {"'' > false",      Value{false}},
    {"null > false",    Value{false}},
    {"[] > false",      Error{R"(At line 1, column 4: Cannot evaluate expression '[] > false': Invalid operand types Array and Boolean)"}},
    {"{} > false",      Error{R"(At line 1, column 4: Cannot evaluate expression '{} > false': Invalid operand types Map and Boolean)"}},

    {"0 > 0",           Value{false}},
    {"1 > 0",           Value{true}},
    {"'true' > 0",      Error{R"(At line 1, column 8: Cannot evaluate expression '"true" > 0': At line 1, column 1: Cannot convert value '"true"' of type 'String' to type 'Number')"}},
    {"'false' > 0",     Error{R"(At line 1, column 9: Cannot evaluate expression '"false" > 0': At line 1, column 1: Cannot convert value '"false"' of type 'String' to type 'Number')"}},
    {"'' > 0",          Value{false}},
    {"'0' > 0",         Value{false}},
    {"'1' > 0",         Value{true}},
    {"null > 0",        Value{false}},
    {"[] > 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] > 0': Invalid operand types Array and Number)"}},
    {"{} > 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} > 0': Invalid operand types Map and Number)"}},

    {"0 > 'a'",         Error{R"(At line 1, column 3: Cannot evaluate expression '0 > "a"': At line 1, column 5: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"1 > 'a'",         Error{R"(At line 1, column 3: Cannot evaluate expression '1 > "a"': At line 1, column 5: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'true' > 'a'",    Value{true}},
    {"'false' > 'a'",   Value{true}},
    {"'' > 'a'",        Value{false}},
    {"'b' > 'a'",       Value{true}},
    {"'a' > 'a'",       Value{false}},
    {"'ab' > 'aa'",     Value{true}},
    {"null > 'a'",      Value{false}},
    {"[] > 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] > "a"': Invalid operand types Array and String)"}},
    {"{} > 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '{} > "a"': Invalid operand types Map and String)"}},
    {"1 > '0'",         Value{true}},
    {"0 > '1'",         Value{false}},

    {"true > null",     Value{true}},
    {"false > null",    Value{true}},
    {"0 > null",        Value{true}},
    {"1 > null",        Value{true}},
    {"'' > null",       Value{true}},
    {"'a' > null",      Value{true}},
    {"null > null",     Value{false}},
    {"[] > null",       Value{true}},
    {"{} > null",       Value{true}},

    {"true > []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true > []': Invalid operand types Boolean and Array)"}},
    {"false > []",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false > []': Invalid operand types Boolean and Array)"}},
    {"0 > []",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 > []': Invalid operand types Number and Array)"}},
    {"1 > []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 > []': Invalid operand types Number and Array)"}},
    {"'' > []",         Error{R"(At line 1, column 4: Cannot evaluate expression '"" > []': Invalid operand types String and Array)"}},
    {"'a' > []",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" > []': Invalid operand types String and Array)"}},
    {"null > []",       Value{false}},
    {"[] > []",         Value{false}},
    {"[1] > [1]",       Value{false}},
    {"[2] > [1]",       Value{true}},
    {"[1,2] > [1]",     Value{true}},
    {"[1,2] > [1,2]",   Value{false}},
    {"[1,2,3] > [1,2]", Value{true}},
    {"[1,2] > [1,2,3]", Value{false}},
    {"{} > []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} > []': Invalid operand types Map and Array)"}},

    {"true > {}",             Error{R"(At line 1, column 6: Cannot evaluate expression 'true > {}': Invalid operand types Boolean and Map)"}},
    {"false > {}",            Error{R"(At line 1, column 7: Cannot evaluate expression 'false > {}': Invalid operand types Boolean and Map)"}},
    {"0 > {}",                Error{R"(At line 1, column 3: Cannot evaluate expression '0 > {}': Invalid operand types Number and Map)"}},
    {"1 > {}",                Error{R"(At line 1, column 3: Cannot evaluate expression '1 > {}': Invalid operand types Number and Map)"}},
    {"'' > {}",               Error{R"(At line 1, column 4: Cannot evaluate expression '"" > {}': Invalid operand types String and Map)"}},
    {"'a' > {}",              Error{R"(At line 1, column 5: Cannot evaluate expression '"a" > {}': Invalid operand types String and Map)"}},
    {"null > {}",             Value{false}},
    {"[] > {}",               Error{R"(At line 1, column 4: Cannot evaluate expression '[] > {}': Invalid operand types Array and Map)"}},
    {"{} > {}",               Value{false}},
    {"{k1:1} > {k1:1}",       Value{false}},
    {"{k2:1} > {k1:1}",       Value{true}},
    {"{k1:1} > {k2:1}",       Value{false}},
    {"{k1:2} > {k1:1}",       Value{true}},
    {"{k1:1, k2:2} > {k1:1}", Value{true}},
    {"{k1:2, k2:2} > {k1:1}", Value{true}},

    {"false >= false",   Value{true}},
    {"true >= false",    Value{true}},
    {"false >= true",    Value{false}},
    {"true >= true",     Value{true}},

    {"0 >= false",       Value{true}},
    {"1 >= false",       Value{true}},
    {"'true' >= false",  Value{true}},
    {"'false' >= false", Value{true}},
    {"'' >= false",      Value{true}},
    {"null >= false",    Value{false}},
    {"[] >= false",      Error{R"(At line 1, column 4: Cannot evaluate expression '[] >= false': Invalid operand types Array and Boolean)"}},
    {"{} >= false",      Error{R"(At line 1, column 4: Cannot evaluate expression '{} >= false': Invalid operand types Map and Boolean)"}},

    {"0 >= 0",           Value{true}},
    {"1 >= 0",           Value{true}},
    {"'true' >= 0",      Error{R"(At line 1, column 8: Cannot evaluate expression '"true" >= 0': At line 1, column 1: Cannot convert value '"true"' of type 'String' to type 'Number')"}},
    {"'false' >= 0",     Error{R"(At line 1, column 9: Cannot evaluate expression '"false" >= 0': At line 1, column 1: Cannot convert value '"false"' of type 'String' to type 'Number')"}},
    {"'' >= 0",          Value{true}},
    {"'0' >= 0",         Value{true}},
    {"'1' >= 0",         Value{true}},
    {"null >= 0",        Value{false}},
    {"[] >= 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] >= 0': Invalid operand types Array and Number)"}},
    {"{} >= 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '{} >= 0': Invalid operand types Map and Number)"}},

    {"0 >= 'a'",         Error{R"(At line 1, column 3: Cannot evaluate expression '0 >= "a"': At line 1, column 6: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"1 >= 'a'",         Error{R"(At line 1, column 3: Cannot evaluate expression '1 >= "a"': At line 1, column 6: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'true' >= 'a'",    Value{true}},
    {"'false' >= 'a'",   Value{true}},
    {"'' >= 'a'",        Value{false}},
    {"'b' >= 'a'",       Value{true}},
    {"'a' >= 'a'",       Value{true}},
    {"'ab' >= 'aa'",     Value{true}},
    {"null >= 'a'",      Value{false}},
    {"[] >= 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] >= "a"': Invalid operand types Array and String)"}},
    {"{} >= 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '{} >= "a"': Invalid operand types Map and String)"}},
    {"1 >= '0'",         Value{true}},
    {"0 >= '1'",         Value{false}},

    {"true >= null",     Value{true}},
    {"false >= null",    Value{true}},
    {"0 >= null",        Value{true}},
    {"1 >= null",        Value{true}},
    {"'' >= null",       Value{true}},
    {"'a' >= null",      Value{true}},
    {"null >= null",     Value{true}},
    {"[] >= null",       Value{true}},
    {"{} >= null",       Value{true}},

    {"true >= []",       Error{R"(At line 1, column 6: Cannot evaluate expression 'true >= []': Invalid operand types Boolean and Array)"}},
    {"false >= []",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false >= []': Invalid operand types Boolean and Array)"}},
    {"0 >= []",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 >= []': Invalid operand types Number and Array)"}},
    {"1 >= []",          Error{R"(At line 1, column 3: Cannot evaluate expression '1 >= []': Invalid operand types Number and Array)"}},
    {"'' >= []",         Error{R"(At line 1, column 4: Cannot evaluate expression '"" >= []': Invalid operand types String and Array)"}},
    {"'a' >= []",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" >= []': Invalid operand types String and Array)"}},
    {"null >= []",       Value{false}},
    {"[] >= []",         Value{true}},
    {"[1] >= [1]",       Value{true}},
    {"[2] >= [1]",       Value{true}},
    {"[1,2] >= [1]",     Value{true}},
    {"[1,2] >= [1,2]",   Value{true}},
    {"[1,2,3] >= [1,2]", Value{true}},
    {"[1,2] >= [1,2,3]", Value{false}},
    {"{} >= []",         Error{R"(At line 1, column 4: Cannot evaluate expression '{} >= []': Invalid operand types Map and Array)"}},

    {"true >= {}",             Error{R"(At line 1, column 6: Cannot evaluate expression 'true >= {}': Invalid operand types Boolean and Map)"}},
    {"false >= {}",            Error{R"(At line 1, column 7: Cannot evaluate expression 'false >= {}': Invalid operand types Boolean and Map)"}},
    {"0 >= {}",                Error{R"(At line 1, column 3: Cannot evaluate expression '0 >= {}': Invalid operand types Number and Map)"}},
    {"1 >= {}",                Error{R"(At line 1, column 3: Cannot evaluate expression '1 >= {}': Invalid operand types Number and Map)"}},
    {"'' >= {}",               Error{R"(At line 1, column 4: Cannot evaluate expression '"" >= {}': Invalid operand types String and Map)"}},
    {"'a' >= {}",              Error{R"(At line 1, column 5: Cannot evaluate expression '"a" >= {}': Invalid operand types String and Map)"}},
    {"null >= {}",             Value{false}},
    {"[] >= {}",               Error{R"(At line 1, column 4: Cannot evaluate expression '[] >= {}': Invalid operand types Array and Map)"}},
    {"{} >= {}",               Value{true}},
    {"{k1:1} >= {k1:1}",       Value{true}},
    {"{k2:1} >= {k1:1}",       Value{true}},
    {"{k1:1} >= {k2:1}",       Value{false}},
    {"{k1:2} >= {k1:1}",       Value{true}},
    {"{k1:1, k2:2} >= {k1:1}", Value{true}},
    {"{k1:2, k2:2} >= {k1:1}", Value{true}},

    {"false == false",   Value{true}},
    {"false == true",    Value{false}},
    {"true == false",    Value{false}},
    {"true == true",     Value{true}},

    {"false == 0",       Value{true}},
    {"false == 1",       Value{false}},
    {"false == 'true'",  Value{false}},
    {"false == 'false'", Value{true}},
    {"false == ''",      Value{true}},
    {"false == null",    Value{false}},
    {"false == []",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false == []': Invalid operand types Boolean and Array)"}},
    {"false == {}",      Error{R"(At line 1, column 7: Cannot evaluate expression 'false == {}': Invalid operand types Boolean and Map)"}},

    {"0 == 0",           Value{true}},
    {"0 == 1",           Value{false}},
    {"0 == 'true'",      Error{R"(At line 1, column 3: Cannot evaluate expression '0 == "true"': At line 1, column 6: Cannot convert value '"true"' of type 'String' to type 'Number')"}},
    {"0 == 'false'",     Error{R"(At line 1, column 3: Cannot evaluate expression '0 == "false"': At line 1, column 6: Cannot convert value '"false"' of type 'String' to type 'Number')"}},
    {"0 == ''",          Value{true}},
    {"0 == '0'",         Value{true}},
    {"0 == '1'",         Value{false}},
    {"0 == null",        Value{false}},
    {"0 == []",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 == []': Invalid operand types Number and Array)"}},
    {"0 == {}",          Error{R"(At line 1, column 3: Cannot evaluate expression '0 == {}': Invalid operand types Number and Map)"}},

    {"'a' == 0",         Error{R"(At line 1, column 5: Cannot evaluate expression '"a" == 0': At line 1, column 1: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'a' == 1",         Error{R"(At line 1, column 5: Cannot evaluate expression '"a" == 1': At line 1, column 1: Cannot convert value '"a"' of type 'String' to type 'Number')"}},
    {"'a' == 'b'",       Value{false}},
    {"'a' == 'a'",       Value{true}},
    {"'aa' == 'ab'",     Value{false}},
    {"'a' == null",      Value{false}},
    {"'a' == []",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" == []': Invalid operand types String and Array)"}},
    {"'a' == {}",        Error{R"(At line 1, column 5: Cannot evaluate expression '"a" == {}': Invalid operand types String and Map)"}},
    {"'0' == 0",         Value{true}},
    {"'0' == 1",         Value{false}},

    {"null == true",     Value{false}},
    {"null == false",    Value{false}},
    {"null == 0",        Value{false}},
    {"null == 1",        Value{false}},
    {"null == ''",       Value{false}},
    {"null == 'a'",      Value{false}},
    {"null == null",     Value{true}},
    {"null == []",       Value{false}},
    {"null == {}",       Value{false}},

    {"[] == true",       Error{R"(At line 1, column 4: Cannot evaluate expression '[] == true': Invalid operand types Array and Boolean)"}},
    {"[] == false",      Error{R"(At line 1, column 4: Cannot evaluate expression '[] == false': Invalid operand types Array and Boolean)"}},
    {"[] == 0",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] == 0': Invalid operand types Array and Number)"}},
    {"[] == 1",          Error{R"(At line 1, column 4: Cannot evaluate expression '[] == 1': Invalid operand types Array and Number)"}},
    {"[] == ''",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] == ""': Invalid operand types Array and String)"}},
    {"[] == 'a'",        Error{R"(At line 1, column 4: Cannot evaluate expression '[] == "a"': Invalid operand types Array and String)"}},
    {"[] == null",       Value{false}},
    {"[] == []",         Value{true}},
    {"[1] == [1]",       Value{true}},
    {"[1] == [2]",       Value{false}},
    {"[1] == [1,2]",     Value{false}},
    {"[1,2] == [1,2]",   Value{true}},
    {"[1,2] == [1,2,3]", Value{false}},
    {"[1,2,3] == [1,2]", Value{false}},
    {"[] == {}",         Error{R"(At line 1, column 4: Cannot evaluate expression '[] == {}': Invalid operand types Array and Map)"}},

    {"{} == true",             Error{R"(At line 1, column 4: Cannot evaluate expression '{} == true': Invalid operand types Map and Boolean)"}},
    {"{} == false",            Error{R"(At line 1, column 4: Cannot evaluate expression '{} == false': Invalid operand types Map and Boolean)"}},
    {"{} == 0",                Error{R"(At line 1, column 4: Cannot evaluate expression '{} == 0': Invalid operand types Map and Number)"}},
    {"{} == 1",                Error{R"(At line 1, column 4: Cannot evaluate expression '{} == 1': Invalid operand types Map and Number)"}},
    {"{} == ''",               Error{R"(At line 1, column 4: Cannot evaluate expression '{} == ""': Invalid operand types Map and String)"}},
    {"{} == 'a'",              Error{R"(At line 1, column 4: Cannot evaluate expression '{} == "a"': Invalid operand types Map and String)"}},
    {"{} == null",             Value{false}},
    {"{} == []",               Error{R"(At line 1, column 4: Cannot evaluate expression '{} == []': Invalid operand types Map and Array)"}},
    {"{} == {}",               Value{true}},
    {"{k1:1} == {k1:1}",       Value{true}},
    {"{k1:1} == {k2:1}",       Value{false}},
    {"{k2:1} == {k1:1}",       Value{false}},
    {"{k1:1} == {k1:2}",       Value{false}},
    {"{k1:1} == {k1:1, k2:2}", Value{false}},
    {"{k1:1} == {k1:2, k2:2}", Value{false}},

    {"undefined == undefined", Value{true}},
    {"undefined == null",      Value{false}},
    {"null == undefined",      Value{false}},
    {"undefined == false",     Value{false}},
    {"undefined == 0",         Value{false}},
    {"undefined == ''",        Value{false}},
    {"undefined == []",        Value{false}},
    {"undefined == {}",        Value{false}},

    // Undefined sorts below everything else, null included
    {"undefined < null",       Value{true}},
    {"undefined <= null",      Value{true}},
    {"undefined > null",       Value{false}},
    {"undefined >= null",      Value{false}},
    {"null < undefined",       Value{false}},
    {"null <= undefined",      Value{false}},
    {"null > undefined",       Value{true}},
    {"null >= undefined",      Value{true}},
    {"null != undefined",      Value{true}},
    {"undefined < 0",          Value{true}},
    {"0 > undefined",          Value{true}},
    {"[] > undefined",         Value{true}},
    {"{} > undefined",         Value{true}},

    // Case
    {"true -> 'asdf'",  Value{"asdf"}},
    {"false -> 'asdf'", Value::Undefined},
    {"false -> x[-1]",  Value::Undefined},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedResult);
  }

  SECTION("Comparison with ranges")
  {
    // The grammar only accepts a range inside a subscript, so a range value can
    // only reach a comparison by way of a variable.
    const auto variables = MapType{{"r", Value{RangeType{BoundedRange{1, 3}}}}};

    // clang-format off
    const auto
    [expression,       expectedResult] = GENERATE(table<std::string, Result<Value>>({
    {"r == null",      Value{false}},
    {"r > null",       Value{true}},
    {"r == undefined", Value{false}},
    {"r > undefined",  Value{true}},
    {"null == r",      Value{false}},
    {"undefined == r", Value{false}},

    // a range is not comparable to anything else, not even another range
    {"r == r",         Error{"At line 1, column 3: Cannot evaluate expression 'r == r': Invalid operand types Range and Range"}},
    {"r == 0",         Error{"At line 1, column 3: Cannot evaluate expression 'r == 0': Invalid operand types Range and Number"}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression, variables) == expectedResult);
  }

  SECTION("Vec3")
  {
    SECTION("Subscript")
    {
      // clang-format off
      const auto
      [expression,             expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"vec(1, 2, 3)[0]",      Value{1.0}},
      {"vec(1, 2, 3)[1]",      Value{2.0}},
      {"vec(1, 2, 3)[2]",      Value{3.0}},
      {"vec(1, 2, 3)[-1]",     Value{3.0}},
      {"vec(1, 2, 3)[3]",      Error{"At line 1, column 13: Cannot evaluate expression 'vec(1, 2, 3)[3]': Index 3 is out of bounds for 'vec(1, 2, 3)'"}},

      {R"(vec(1, 2, 3)["x"])", Value{1.0}},
      {R"(vec(1, 2, 3)["y"])", Value{2.0}},
      {R"(vec(1, 2, 3)["z"])", Value{3.0}},
      {R"(vec(1, 2, 3)["w"])", Value::Undefined},

      {"vec(1, 2, 3).x",       Value{1.0}},
      {"vec(1, 2, 3).y",       Value{2.0}},
      {"vec(1, 2, 3).z",       Value{3.0}},
      {"vec(1, 2, 3).w",       Value::Undefined},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("Unary operators")
    {
      // clang-format off
      const auto
      [expression,      expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"+vec(1, 2, 3)", Value{Vec3Type{1, 2, 3}}},
      {"-vec(1, 2, 3)", Value{Vec3Type{-1, -2, -3}}},
      {"!vec(1, 2, 3)", Error{"At line 1, column 1: Cannot evaluate expression '!vec(1, 2, 3)': Invalid type Vec3"}},
      {"~vec(1, 2, 3)", Error{"At line 1, column 1: Cannot evaluate expression '~vec(1, 2, 3)': Invalid type Vec3"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("Arithmetic")
    {
      // clang-format off
      const auto
      [expression,                    expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"vec(1, 2, 3) + vec(4, 5, 6)", Value{Vec3Type{5, 7, 9}}},
      {"vec(4, 5, 6) - vec(1, 2, 3)", Value{Vec3Type{3, 3, 3}}},
      {"vec(1, 2, 3) * 2",            Value{Vec3Type{2, 4, 6}}},
      {"2 * vec(1, 2, 3)",            Value{Vec3Type{2, 4, 6}}},
      {"vec(1, 2, 3) / 2",            Value{Vec3Type{0.5, 1.0, 1.5}}},

      {"vec(1, 2, 3) * vec(4, 5, 6)", Value{Vec3Type{4, 10, 18}}},
      {"vec(1, 2, 3) / vec(4, 5, 6)", Value{Vec3Type{0.25, 0.4, 0.5}}},
      {"12 / vec(4, 5, 6)",           Value{Vec3Type{3.0, 2.4, 2.0}}},

      // a string operand is converted to a vector, or to a number to scale a vector by
      {R"("1 2 3" + vec(1, 1, 1))",   Value{Vec3Type{2, 3, 4}}},
      {R"(vec(1, 1, 1) - "1 2 3")",   Value{Vec3Type{0, -1, -2}}},
      {R"("1 2 3" * vec(2, 2, 2))",   Value{Vec3Type{2, 4, 6}}},
      {R"("1 2 3" / vec(2, 2, 2))",   Value{Vec3Type{0.5, 1, 1.5}}},
      {R"(vec(1, 2, 3) * "2")",       Value{Vec3Type{2, 4, 6}}},
      {R"("2" * vec(1, 2, 3))",       Value{Vec3Type{2, 4, 6}}},
      {R"(vec(2, 4, 6) / "2")",       Value{Vec3Type{1, 2, 3}}},
      {"vec(1, 2, 3) * true",         Value{Vec3Type{1, 2, 3}}},

      // two strings are never converted to vectors
      {R"("1 2 3" + "1 2 3")",        Value{"1 2 31 2 3"}},

      {"vec(1, 2, 3) + undefined",    Value::Undefined},
      {"undefined + vec(1, 2, 3)",    Value::Undefined},

      {"vec(1, 2, 3) % vec(4, 5, 6)", Error{"At line 1, column 14: Cannot evaluate expression 'vec(1, 2, 3) % vec(4, 5, 6)': Invalid operand types Vec3 and Vec3"}},
      {"vec(1, 2, 3) + 2",            Error{"At line 1, column 14: Cannot evaluate expression 'vec(1, 2, 3) + 2': Invalid operand types Vec3 and Number"}},
      {R"(vec(1, 2, 3) * "a")",       Error{R"(At line 1, column 14: Cannot evaluate expression 'vec(1, 2, 3) * "a"': Invalid operand types Vec3 and String)"}},
      {"vec(1, 2, 3) * null",         Error{"At line 1, column 14: Cannot evaluate expression 'vec(1, 2, 3) * null': Invalid operand types Vec3 and Null"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("Comparison")
    {
      // clang-format off
      const auto
      [expression,                     expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"vec(1, 2, 3) == vec(1, 2, 3)", Value{true}},
      {"vec(1, 2, 3) == vec(4, 5, 6)", Value{false}},
      {"vec(1, 2, 3) != vec(4, 5, 6)", Value{true}},
      {"vec(1, 2, 3) < vec(4, 5, 6)",  Value{true}},
      {"vec(4, 5, 6) > vec(1, 2, 3)",  Value{true}},

      // a string is converted to a vector
      {R"(vec(1, 2, 3) == "1 2 3")",   Value{true}},
      {R"("1 2 3" == vec(1, 2, 3))",   Value{true}},
      {R"(vec(1, 2, 3) != "1 2 4")",   Value{true}},
      {R"("1 2 3" < vec(1, 2, 4))",    Value{true}},

      {"vec(1, 2, 3) == 1",            Error{"At line 1, column 14: Cannot evaluate expression 'vec(1, 2, 3) == 1': Invalid operand types Vec3 and Number"}},
      {R"(vec(1, 2, 3) == "a")",       Error{R"(At line 1, column 14: Cannot evaluate expression 'vec(1, 2, 3) == "a"': At line 1, column 17: Cannot convert value '"a"' of type 'String' to type 'Vec3')"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }
  }

  SECTION("BBox")
  {
    SECTION("Subscript")
    {
      // clang-format off
      const auto
      [expression,                                   expectedValue] = GENERATE(table<std::string, Value>({
      {"bbox(vec(1, 2, 3), vec(4, 5, 6)).min",       Value{Vec3Type{1, 2, 3}}},
      {"bbox(vec(1, 2, 3), vec(4, 5, 6)).max",       Value{Vec3Type{4, 5, 6}}},
      {R"(bbox(vec(1, 2, 3), vec(4, 5, 6))["min"])", Value{Vec3Type{1, 2, 3}}},
      {R"(bbox(vec(1, 2, 3), vec(4, 5, 6))["max"])", Value{Vec3Type{4, 5, 6}}},
      {R"(bbox(vec(1, 2, 3), vec(4, 5, 6))["w"])",   Value::Undefined},
      {"bbox(vec(1, 2, 3), vec(4, 5, 6)).w",         Value::Undefined},

      // corners are normalized component-wise, so argument order doesn't matter
      {"bbox(vec(4, 5, 6), vec(1, 2, 3)).min",       Value{Vec3Type{1, 2, 3}}},
      {"bbox(vec(4, 5, 6), vec(1, 2, 3)).max",       Value{Vec3Type{4, 5, 6}}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedValue);
    }

    SECTION("Unary operators")
    {
      CHECK(
        evaluate("!bbox(vec(1, 2, 3), vec(4, 5, 6))")
        == Error{"At line 1, column 1: Cannot evaluate expression "
                 "'!bbox(vec(1, 2, 3), vec(4, 5, 6))': Invalid type BBox"});
    }

    SECTION("Arithmetic")
    {
      CHECK(
        evaluate("bbox(vec(1, 2, 3), vec(4, 5, 6)) + bbox(vec(1, 2, 3), vec(4, 5, 6))")
        == Error{"At line 1, column 34: Cannot evaluate expression "
                 "'bbox(vec(1, 2, 3), vec(4, 5, 6)) + bbox(vec(1, 2, 3), vec(4, 5, "
                 "6))': Invalid operand types BBox and BBox"});
    }

    SECTION("Comparison")
    {
      // clang-format off
      const auto
      [expression,                                                             expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"bbox(vec(1, 2, 3), vec(4, 5, 6)) == bbox(vec(1, 2, 3), vec(4, 5, 6))", Value{true}},
      {"bbox(vec(1, 2, 3), vec(4, 5, 6)) == bbox(vec(0, 0, 0), vec(1, 1, 1))", Value{false}},
      {"bbox(vec(1, 2, 3), vec(4, 5, 6)) != bbox(vec(0, 0, 0), vec(1, 1, 1))", Value{true}},

      {"bbox(vec(1, 2, 3), vec(4, 5, 6)) == 1",                                Error{"At line 1, column 34: Cannot evaluate expression 'bbox(vec(1, 2, 3), vec(4, 5, 6)) == 1': Invalid operand types BBox and Number"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }
  }

  SECTION("Subscript")
  {
    using T = std::tuple<std::string, Result<Value>>;

    // clang-format off
    const auto
    [expression,                         expectedResult] = GENERATE(values<T>({
    // Single index
    {"'asdf'[0]",                        Value{"a"}},
    {"'asdf'[1]",                        Value{"s"}},
    {"'asdf'[3]",                        Value{"f"}},
    {"'asdf'[-1]",                       Value{"f"}},
    {"'asdf'[-4]",                       Value{"a"}},
    {"'asdf'[true]",                     Value{"s"}},
    {"'asdf'[false]",                    Value{"a"}},

    // Positive indices
    {"'asdf'[0, 1]",                     Value{"as"}},
    {"'asdf'[0, 1, 2]",                  Value{"asd"}},
    {"'asdf'[1, 2]",                     Value{"sd"}},
    {"'asdf'[1, 2, 7]",                  Value{"sd"}},
    {"'asdf'[3, 2, 1, 0]",               Value{"fdsa"}},

    // Negative indices
    {"'asdf'[0, -1]",                    Value{"af"}},
    {"'asdf'[-4, -3, -2, -1]",           Value{"asdf"}},

    // Range
    {"'asdf'[0..1]",                     Value{"as"}},
    {"'asdf'[1..2]",                     Value{"sd"}},
    {"'asdf'[0..5]",                     Value{"asdf"}},
    {"'asdf'[3..0]",                     Value{"fdsa"}},
    {"'asdf'[3..1]",                     Value{"fds"}},
    {"'asdf'[3..2]",                     Value{"fd"}},
    {"'asdf'[3..3]",                     Value{"f"}},
    {"'asdf'[3..4]",                     Value{"f"}},
    {"'asdf'[0..]",                      Value{"asdf"}},
    {"'asdf'[1..]",                      Value{"sdf"}},
    {"'asdf'[..0]",                      Value{"fdsa"}},
    {"'asdf'[..1]",                      Value{"fds"}},
    {"'asdf'[..2]",                      Value{"fd"}},
    {"'asdf'[..3]",                      Value{"f"}},
    {"'asdf'[..4]",                      Value{"f"}},
    {"'asdf'[..5]",                      Value{"f"}},
    {"'asdf'[-4..-1]",                   Value{"asdf"}},
    {"'asdf'[-4..0]",                    Value{"asdfa"}},
    {"'asdf'[-4..1]",                    Value{"asdfas"}},
    {"'asdf'[-4..4]",                    Value{"asdfasdf"}},
    {"'asdf'[-4..]",                     Value{"asdfasdf"}},
    {"'asdf'[..-4]",                     Value{"fdsafdsa"}},

    // Mixed
    {"'asdfxyz'[0, 1..3, 3..1, -1..-3]", Value{"asdffdszyx"}},

    // Chained
    {"[1, 2, [4, 5]][2][1]",             Value{5}},

    // For Arrays
    {"[1, 2, 3][0]",                     Value{1}},
    {"[1, 2, 3][-1]",                    Value{3}},
    {"[1, 2, 3][0, 2]",                  Value{ArrayType{Value{1}, Value{3}}}},
    {"[1, 2, 3][2, 1, 0]",               Value{ArrayType{Value{3}, Value{2}, Value{1}}}},
    {"[1, 2, 3][0, -1]",                 Value{ArrayType{Value{1}, Value{3}}}},
    {"[1, 2, 3][0..1]",                  Value{ArrayType{Value{1}, Value{2}}}},
    {"[1, 2, 3][2..0]",                  Value{ArrayType{Value{3}, Value{2}, Value{1}}}},
    {"[1, 2, 3][1..]",                   Value{ArrayType{Value{2}, Value{3}}}},
    {"[1, 2, 3][..1]",                   Value{ArrayType{Value{3}, Value{2}}}},
    {"[1, 2, 3][0, 1..2]",               Value{ArrayType{Value{1}, Value{2}, Value{3}}}},

    // For Maps
    {"{a: 1, b: 2, c: 3}['a']",          Value{1}},
    {"{a: 1, b: 2, c: 3}['b']",          Value{2}},
    {"{a: 1, b: 2, c: 3}['c']",          Value{3}},
    {"{a: 1, b: 2, c: 3}['a', 'c']",     Value{MapType{{"a", Value{1}}, {"c", Value{3}}}}},
    {"{a: 1, b: 2, c: 3}['c', 'a']",     Value{MapType{{"a", Value{1}}, {"c", Value{3}}}}},
    {"{a: 1, b: 2, c: 3}['a', 'a']",     Value{MapType{{"a", Value{1}}}}},

    // Missing keys are dropped rather than reported
    {"{a: 1, b: 2, c: 3}['a', 'd']",     Value{MapType{{"a", Value{1}}}}},
    {"{a: 1, b: 2, c: 3}['d', 'e']",     Value{MapType{}}},

    // Out of bounds
    {"'asdf'[5]",                        Value{""}},
    {"'asdf'[-5]",                       Value{""}},
    {"'asdf'[4, 5]",                     Value{""}},
    {"[1, 2, 3][-4]",                    Error{"At line 1, column 10: Cannot evaluate expression '[1, 2, 3][-4]': Index -4 is out of bounds for '[1, 2, 3]'"}},
    {"[0, 1, 2, 3][5]",                  Error{"At line 1, column 13: Cannot evaluate expression '[0, 1, 2, 3][5]': Index 5 is out of bounds for '[0, 1, 2, 3]'"}},
    {"[1, 2, 3][0, 5]",                  Error{"At line 1, column 10: Cannot evaluate expression '[1, 2, 3][[0, 5]]': Index 5 is out of bounds for '[1, 2, 3]'"}},
    {"[1, 2, 3][0..5]",                  Error{"At line 1, column 10: Cannot evaluate expression '[1, 2, 3][0..5]': Index 3 is out of bounds for '[1, 2, 3]'"}},
    {"{a: 1, b: 2, c: 3}['d']",          Value::Undefined},

    // An undefined range bound makes the whole range undefined, which cannot index
    {"'asdf'[undefined..1]",             Error{R"(At line 1, column 7: Cannot evaluate expression '"asdf"[undefined..1]': 'undefined' is not a compatible index for '"asdf"')"}},
    {"'asdf'[1..undefined]",             Error{R"(At line 1, column 7: Cannot evaluate expression '"asdf"[1..undefined]': 'undefined' is not a compatible index for '"asdf"')"}},

    // A map can only be indexed by strings
    {"{a: 1}['a', 1]",                   Error{"At line 1, column 13: Cannot convert value '1' of type 'Number' to type 'String'"}},

    // Incompatible index types
    {"1[0]",                             Error{"At line 1, column 2: Cannot evaluate expression '1[0]': '0' is not a compatible index for '1'"}},
    {"true[0]",                          Error{"At line 1, column 5: Cannot evaluate expression 'true[0]': '0' is not a compatible index for 'true'"}},
    {"null[0]",                          Error{"At line 1, column 5: Cannot evaluate expression 'null[0]': '0' is not a compatible index for 'null'"}},
    {"'asdf'['a']",                      Error{R"(At line 1, column 7: Cannot evaluate expression '"asdf"["a"]': '"a"' is not a compatible index for '"asdf"')"}},
    {"{a: 1}[0]",                        Error{R"(At line 1, column 7: Cannot evaluate expression '{ "a": 1 }[0]': '0' is not a compatible index for '{ "a": 1 }')"}},

    // Subscripting Undefined
    {"undefined[0]",                     Value::Undefined},
    {"undefined['key']",                 Value::Undefined},

    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedResult);
  }

  SECTION("Subscript of a Range")
  {
    const auto variables = MapType{{"r", Value{RangeType{BoundedRange{1, 3}}}}};

    CHECK(
      evaluate("r[0]", variables)
      == Error{"At line 1, column 2: Cannot evaluate expression 'r[0]': '0' is not a "
               "compatible index for '[1..3]'"});
  }

  SECTION("Dot access")
  {
    // clang-format off
    const auto
    [expression,  variables,                                                       expectedValue] = GENERATE(table<std::string, MapType, Value>({
    // a.b evaluates identically to a["b"], for a bound Map variable
    {"m.b",       {{"m", Value{MapType{{"b", Value{1}}}}}},                        Value{1}},
    {R"(m["b"])", {{"m", Value{MapType{{"b", Value{1}}}}}},                        Value{1}},

    // A missing key is Undefined via dot access too, same as via bracket subscript
    {"m.missing", {{"m", Value{MapType{{"b", Value{1}}}}}},                        Value::Undefined},

    // Chains
    {"m.m.b",     {{"m", Value{MapType{{"m", Value{MapType{{"b", Value{2}}}}}}}}}, Value{2}},
    }));
    // clang-format on

    CAPTURE(expression, variables);

    CHECK(evaluate(expression, variables) == expectedValue);
  }

  SECTION("Function calls")
  {
    SECTION("unknown functions")
    {
      // clang-format off
      const auto
      [expression,     expectedError] = GENERATE(table<std::string, Error>({
      {"f()",          Error{"At line 1, column 1: Cannot evaluate expression 'f()': Unknown function: 'f'"}},
      {"f(1, 2)",      Error{"At line 1, column 1: Cannot evaluate expression 'f(1, 2)': Unknown function: 'f'"}},

      // arguments are still evaluated (and can themselves error) before the "unknown
      // function" check happens
      {"f([] + true)", Error{"At line 1, column 6: Cannot evaluate expression '[] + true': Invalid operand types Array and Boolean"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedError);
    }

    SECTION("vec")
    {
      // clang-format off
      const auto
      [expression,                 expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"vec(1, 2, 3)",             Value{Vec3Type{1, 2, 3}}},
      {"vec(1.5, -2, 0)",          Value{Vec3Type{1.5, -2.0, 0.0}}},

      // arguments don't have to be literals
      {"vec(1 + 1, 2 * 2, 3 - 1)", Value{Vec3Type{2, 4, 2}}},

      // arguments are converted to numbers, and an undefined argument yields undefined
      {R"(vec("2", "2", true))",   Value{Vec3Type{2, 2, 1}}},
      {"vec(undefined, 2, 3)",     Value::Undefined},
      {R"(vec("a", 2, 3))",        Error{R"(At line 1, column 1: Cannot evaluate expression 'vec("a", 2, 3)': At line 1, column 5: Cannot convert value '"a"' of type 'String' to type 'Number')"}},

      {"vec()",                    Error{"At line 1, column 1: Cannot evaluate expression 'vec()': vec() expects 3 arguments, but got 0"}},
      {"vec(1, 2)",                Error{"At line 1, column 1: Cannot evaluate expression 'vec(1, 2)': vec() expects 3 arguments, but got 2"}},
      {"vec(1, 2, 3, 4)",          Error{"At line 1, column 1: Cannot evaluate expression 'vec(1, 2, 3, 4)': vec() expects 3 arguments, but got 4"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("bbox")
    {
      // clang-format off
      const auto
      [expression,                                       expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"bbox(vec(1, 2, 3), vec(4, 5, 6))",               Value{BBoxType{Vec3Type{1.0, 2.0, 3.0}, Vec3Type{4.0, 5.0, 6.0}}}},

      // corners are normalized component-wise, so argument order doesn't matter
      {"bbox(vec(4, 5, 6), vec(1, 2, 3))",               Value{BBoxType{Vec3Type{1.0, 2.0, 3.0}, Vec3Type{4.0, 5.0, 6.0}}}},

      // strings containing vectors are converted, and an undefined argument yields
      // undefined
      {R"(bbox("1 2 3", vec(4, 5, 6)))",                 Value{BBoxType{Vec3Type{1.0, 2.0, 3.0}, Vec3Type{4.0, 5.0, 6.0}}}},
      {"bbox(undefined, vec(4, 5, 6))",                  Value::Undefined},
      {"bbox(1, vec(4, 5, 6))",                          Error{"At line 1, column 1: Cannot evaluate expression 'bbox(1, vec(4, 5, 6))': At line 1, column 6: Cannot convert value '1' of type 'Number' to type 'Vec3'"}},

      {"bbox()",                                         Error{"At line 1, column 1: Cannot evaluate expression 'bbox()': bbox() expects 2 arguments, but got 0"}},
      {"bbox(vec(1, 2, 3))",                             Error{"At line 1, column 1: Cannot evaluate expression 'bbox(vec(1, 2, 3))': bbox() expects 2 arguments, but got 1"}},
      {"bbox(vec(1, 2, 3), vec(4, 5, 6), vec(7, 8, 9))", Error{"At line 1, column 1: Cannot evaluate expression 'bbox(vec(1, 2, 3), vec(4, 5, 6), vec(7, 8, 9))': bbox() expects 2 arguments, but got 3"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("distanceTo")
    {
      // clang-format off
      const auto
      [expression,                                             expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"distanceTo(vec(0, 0, 0), vec(3, 4, 0))",               Value{5.0}},
      {"distanceTo(vec(1, 2, 3), vec(1, 2, 3))",               Value{0.0}},

      // the order of the arguments doesn't matter
      {"distanceTo(vec(3, 4, 0), vec(0, 0, 0))",               Value{5.0}},

      // strings containing vectors are converted, and an undefined argument yields
      // undefined
      {R"(distanceTo("3 4 0", vec(0, 0, 0)))",                 Value{5.0}},
      {"distanceTo(undefined, vec(0, 0, 0))",                  Value::Undefined},
      {"distanceTo(vec(0, 0, 0), undefined)",                  Value::Undefined},
      {R"(distanceTo("a", vec(0, 0, 0)))",                     Error{R"(At line 1, column 1: Cannot evaluate expression 'distanceTo("a", vec(0, 0, 0))': At line 1, column 12: Cannot convert value '"a"' of type 'String' to type 'Vec3')"}},

      {"distanceTo(vec(0, 0, 0))",                             Error{"At line 1, column 1: Cannot evaluate expression 'distanceTo(vec(0, 0, 0))': distanceTo() expects 2 arguments, but got 1"}},
      {"distanceTo()",                                         Error{"At line 1, column 1: Cannot evaluate expression 'distanceTo()': distanceTo() expects 2 arguments, but got 0"}},
      {"distanceTo(vec(0, 0, 0), vec(1, 1, 1), vec(2, 2, 2))", Error{"At line 1, column 1: Cannot evaluate expression 'distanceTo(vec(0, 0, 0), vec(1, 1, 1), vec(2, 2, 2))': distanceTo() expects 2 arguments, but got 3"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("intersects")
    {
      // clang-format off
      const auto
      [expression,                                                                       expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"intersects(bbox(vec(0, 0, 0), vec(2, 2, 2)), bbox(vec(1, 1, 1), vec(3, 3, 3)))", Value{true}},
      {"intersects(bbox(vec(0, 0, 0), vec(1, 1, 1)), bbox(vec(2, 2, 2), vec(3, 3, 3)))", Value{false}},
      {"intersects(bbox(vec(1, 1, 1), vec(3, 3, 3)), bbox(vec(0, 0, 0), vec(2, 2, 2)))", Value{true}},

      // an undefined argument yields undefined
      {"intersects(undefined, bbox(vec(0, 0, 0), vec(1, 1, 1)))",                        Value::Undefined},
      {"intersects(bbox(vec(0, 0, 0), vec(1, 1, 1)), undefined)",                        Value::Undefined},
      {"intersects(1, bbox(vec(0, 0, 0), vec(1, 1, 1)))",                                Error{"At line 1, column 1: Cannot evaluate expression 'intersects(1, bbox(vec(0, 0, 0), vec(1, 1, 1)))': At line 1, column 12: Cannot dereference value '1' of type 'Number' as type 'BBox'"}},

      {"intersects(bbox(vec(0, 0, 0), vec(1, 1, 1)))",                                   Error{"At line 1, column 1: Cannot evaluate expression 'intersects(bbox(vec(0, 0, 0), vec(1, 1, 1)))': intersects() expects 2 arguments, but got 1"}},
      {"intersects()",                                                                   Error{"At line 1, column 1: Cannot evaluate expression 'intersects()': intersects() expects 2 arguments, but got 0"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("like")
    {
      const auto patterns = MapType{
        {"path", Value{R"(C:\quake\id1\gfx.wad)"}},
        {"escapedBackslash", Value{R"(id1\\gfx)"}},
        {"escapedStar", Value{R"(\*)"}},
        {"escapedStarThenStar", Value{R"(a\**)"}},
        {"trailingBackslash", Value{R"(quake\)"}},
        {"escapedTrailingBackslash", Value{R"(quake\\)"}},
        {"literalBackslash", Value{R"(id1\gfx)"}},
        {"literalBackslashThenStar", Value{R"(*id1\g*)"}},
      };

      // clang-format off
      const auto
      [expression,                                       expectedResult] = GENERATE(table<std::string, Result<Value>>({
      // String like String: case-insensitive glob, and prefix/infix forms agree
      {R"(like("Hello World", "hello*"))",               Value{true}},
      {R"("Hello World" like "hello*")",                 Value{true}},
      {R"("Hello World" like "*world")",                 Value{true}},
      {R"("Hello World" like "h?llo*")",                 Value{true}},
      {R"("Hello World" like "goodbye*")",               Value{false}},

      // a pattern with no glob metacharacters matches as a substring, not exactly
      {R"("func_detail" like "detail")",                 Value{true}},
      {R"("func_detail" like "func_detail")",            Value{true}},
      {R"("detail" like "func_detail")",                 Value{false}},

      // in a string literal, a backslash that precedes another backslash or a quote
      // escapes it, but any other backslash is kept
      {R"("quake*" like "quake\*")",                     Value{true}},
      {R"("quake*" like "quake\\*")",                    Value{true}},
      {R"("quake\x" like "quake\\*")",                   Value{false}},
      {R"("quake\x" like "quake\\\\*")",                 Value{true}},
      {R"("id1\gfx.wad" like "id1\gfx")",                Value{true}},
      {R"("id1\gfx.wad" like "id1\\gfx")",               Value{true}},

      // an escaped wildcard is matched literally and doesn't prevent substring matching
      {"path like escapedBackslash",                     Value{true}},
      {R"("a*b" like escapedStar)",                      Value{true}},
      {R"("a*bc" like escapedStarThenStar)",             Value{true}},
      {R"("xa*bc" like escapedStarThenStar)",            Value{false}},

      // a trailing backslash that doesn't escape anything matches a literal backslash
      {"path like trailingBackslash",                    Value{true}},
      {R"("quake*" like trailingBackslash)",             Value{false}},
      {"path like escapedTrailingBackslash",             Value{true}},

      // so does a backslash that doesn't escape a wildcard or another backslash
      {"path like literalBackslash",                     Value{true}},
      {R"("id1g" like literalBackslash)",                Value{false}},
      {"path like literalBackslashThenStar",             Value{true}},

      // Array like String: true if any element matches
      {R"(["a", "b", "trigger_once"] like "*trigger*")", Value{true}},
      {R"(["a", "b", "c"] like "*trigger*")",            Value{false}},
      // non-String elements are simply skipped, not an error
      {R"([1, "trigger_once"] like "*trigger*")",        Value{true}},

      // Map like String: true if any key or String value matches
      {R"({target: "door1"} like "target")",             Value{true}},
      {R"({target: "door1"} like "door*")",              Value{true}},
      {R"({target: "door1"} like "light*")",             Value{false}},
      // non-String values are simply skipped, not an error
      {R"({a: 1} like "a")",                             Value{true}},
      {R"({a: 1} like "1")",                             Value{false}},

      // never throws -- Undefined on either side, or an unsupported type combination,
      // just fails to match
      {R"(undefined like "x")",                          Value{false}},
      {R"("x" like undefined)",                          Value{false}},
      {R"(1 like "1")",                                  Value{false}},
      {R"("x" like 1)",                                  Value{false}},

      {R"(like("a"))",                                   Error{R"(At line 1, column 1: Cannot evaluate expression 'like("a")': like() expects 2 arguments, but got 1)"}},
      {"like()",                                         Error{"At line 1, column 1: Cannot evaluate expression 'like()': like() expects 2 arguments, but got 0"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression, patterns) == expectedResult);
    }

    SECTION("contains")
    {
      // clang-format off
      const auto
      [expression,                                                                   expectedResult] = GENERATE(table<std::string, Result<Value>>({
      // Array contains X: element membership, prefix/infix forms agree
      {R"(contains([1, 2, 3], 2))",                                                  Value{true}},
      {R"([1, 2, 3] contains 2)",                                                    Value{true}},
      {R"([1, 2, 3] contains 4)",                                                    Value{false}},
      {R"(["Detail", "Trigger"] contains "Detail")",                                 Value{true}},

      // elements are compared like `==`, which converts strings to numbers, and an
      // element that can't be compared to the value doesn't match
      {R"([1, 2, 3] contains "2")",                                                  Value{true}},
      {R"(["1", "2"] contains 2)",                                                   Value{true}},
      {R"(["a", 2] contains 2)",                                                     Value{true}},
      {R"([vec(1, 1, 1)] contains "a")",                                             Value{false}},
      {R"(["1 2 3"] contains vec(1, 2, 3))",                                         Value{true}},

      // Map contains String: key membership
      {R"({a: 1, b: 2} contains "a")",                                               Value{true}},
      {R"({a: 1, b: 2} contains "c")",                                               Value{false}},
      // a non-String rhs against a Map is simply false, not an error
      {R"({a: 1, b: 2} contains 1)",                                                 Value{false}},

      // BBox contains Vec3: point containment
      {"bbox(vec(0, 0, 0), vec(2, 2, 2)) contains vec(1, 1, 1)",                     Value{true}},
      {"bbox(vec(0, 0, 0), vec(2, 2, 2)) contains vec(3, 3, 3)",                     Value{false}},

      // BBox contains BBox: full containment
      {"bbox(vec(0, 0, 0), vec(3, 3, 3)) contains bbox(vec(1, 1, 1), vec(2, 2, 2))", Value{true}},
      {"bbox(vec(0, 0, 0), vec(3, 3, 3)) contains bbox(vec(0, 0, 0), vec(3, 3, 3))", Value{true}},
      {"bbox(vec(0, 0, 0), vec(3, 3, 3)) contains bbox(vec(1, 1, 1), vec(4, 4, 4))", Value{false}},

      // never throws -- Undefined on either side, or an unsupported type combination,
      // just fails to match
      {"[1, 2, 3] contains undefined",                                               Value{false}},
      {"undefined contains 1",                                                       Value{false}},
      {R"({a: 1} contains vec(1, 1, 1))",                                            Value{false}},

      {"contains(1)",                                                                Error{"At line 1, column 1: Cannot evaluate expression 'contains(1)': contains() expects 2 arguments, but got 1"}},
      {"contains()",                                                                 Error{"At line 1, column 1: Cannot evaluate expression 'contains()': contains() expects 2 arguments, but got 0"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("in")
    {
      // clang-format off
      const auto
      [expression,                                                             expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"in(2, [1, 2, 3])",                                                     Value{true}},
      {"2 in [1, 2, 3]",                                                       Value{true}},
      {"4 in [1, 2, 3]",                                                       Value{false}},
      {R"("Detail" in ["Detail", "Trigger"])",                                 Value{true}},

      {R"("a" in {a: 1, b: 2})",                                               Value{true}},
      {R"("c" in {a: 1, b: 2})",                                               Value{false}},
      {"1 in {a: 1, b: 2}",                                                    Value{false}},

      {"vec(1, 1, 1) in bbox(vec(0, 0, 0), vec(2, 2, 2))",                     Value{true}},
      {"vec(3, 3, 3) in bbox(vec(0, 0, 0), vec(2, 2, 2))",                     Value{false}},

      {"bbox(vec(1, 1, 1), vec(2, 2, 2)) in bbox(vec(0, 0, 0), vec(3, 3, 3))", Value{true}},
      {"bbox(vec(1, 1, 1), vec(4, 4, 4)) in bbox(vec(0, 0, 0), vec(3, 3, 3))", Value{false}},

      // a in b agrees with b contains a
      {"contains([1, 2, 3], 2)",                                               Value{true}},
      {"[1, 2, 3] contains 2",                                                 Value{true}},

      {"1 in [1, 2] && 3 in [3, 4]",                                           Value{true}},
      {"1 in [1, 2] == true",                                                  Value{true}},

      {"undefined in [1, 2, 3]",                                               Value{false}},
      {"1 in undefined",                                                       Value{false}},
      {"vec(1, 1, 1) in {a: 1}",                                               Value{false}},

      {"in(1)",                                                                Error{"At line 1, column 1: Cannot evaluate expression 'in(1)': in() expects 2 arguments, but got 1"}},
      {"in()",                                                                 Error{"At line 1, column 1: Cannot evaluate expression 'in()': in() expects 2 arguments, but got 0"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }

    SECTION("is")
    {
      // clang-format off
      const auto
      [expression,                                                             expectedResult] = GENERATE(table<std::string, Result<Value>>({
      {"is(1, 1)",                                                             Value{true}},
      {"1 is 1",                                                               Value{true}},
      {"1 is 2",                                                               Value{false}},
      {R"("a" is "a")",                                                        Value{true}},
      {R"("a" is "b")",                                                        Value{false}},
      {"true is true",                                                         Value{true}},
      {"true is false",                                                        Value{false}},
      {"null is null",                                                         Value{true}},
      {"[1, 2] is [1, 2]",                                                     Value{true}},
      {"[1, 2] is [1, 3]",                                                     Value{false}},
      {"{a: 1} is {a: 1}",                                                     Value{true}},
      {"{a: 1} is {a: 2}",                                                     Value{false}},
      {"vec(1, 2, 3) is vec(1, 2, 3)",                                         Value{true}},
      {"vec(1, 2, 3) is vec(3, 2, 1)",                                         Value{false}},
      {"bbox(vec(0, 0, 0), vec(1, 1, 1)) is bbox(vec(0, 0, 0), vec(1, 1, 1))", Value{true}},
      {"bbox(vec(0, 0, 0), vec(1, 1, 1)) is bbox(vec(0, 0, 0), vec(2, 2, 2))", Value{false}},

      // is applies the same implicit conversions as ==
      {R"(1 is "1")",                                                          Value{true}},
      {R"(1 == "1")",                                                          Value{true}},
      {"1 is true",                                                            Value{true}},
      {"1 == true",                                                            Value{true}},
      {"1 is null",                                                            Value{false}},
      {"1 == null",                                                            Value{false}},

      {"undefined is undefined",                                               Value{true}},
      {"1 is undefined",                                                       Value{false}},
      {"undefined is 1",                                                       Value{false}},

      // infix calls bind more tightly than other binary operators
      {"1 is 1 && 2 is 2",                                                     Value{true}},
      {"1 is 1 == true",                                                       Value{true}},

      // chained infix calls are evaluated from left to right
      {"1 in [1, 2] is true",                                                  Value{true}},
      {"3 in [1, 2] is false",                                                 Value{true}},

      // throws for incomparable types
      {"false is []",                                                          Error{"At line 1, column 7: Cannot evaluate expression 'false is []': Invalid operand types Boolean and Array"}},
      {"is(false, [])",                                                        Error{"At line 1, column 1: Cannot evaluate expression 'is(false, [])': Invalid operand types Boolean and Array"}},
      {"false == []",                                                          Error{"At line 1, column 7: Cannot evaluate expression 'false == []': Invalid operand types Boolean and Array"}},

      {"is(1)",                                                                Error{"At line 1, column 1: Cannot evaluate expression 'is(1)': is() expects 2 arguments, but got 1"}},
      {"is()",                                                                 Error{"At line 1, column 1: Cannot evaluate expression 'is()': is() expects 2 arguments, but got 0"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(evaluate(expression) == expectedResult);
    }
  }

  SECTION("Infix calls")
  {
    // clang-format off
    const auto
    [expression,                                                                         expectedValue] = GENERATE(table<std::string, Value>({
    // an infix call agrees with the corresponding prefix call
    {"vec(0, 0, 0) distanceTo vec(3, 4, 0)",                                             Value{5.0}},
    {"distanceTo(vec(0, 0, 0), vec(3, 4, 0))",                                           Value{5.0}},

    {"vec(0, 0, 0) distanceTo vec(3, 4, 0) == vec(0, 0, 0) distanceTo vec(6, 8, 0)",     Value{false}},
    {"distanceTo(vec(0, 0, 0), vec(3, 4, 0)) == distanceTo(vec(0, 0, 0), vec(6, 8, 0))", Value{false}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedValue);
  }

  SECTION("Switch")
  {
    using T = std::tuple<std::string, Result<Value>>;

    // clang-format off
    const auto
    [expression, expectedResult] = GENERATE(values<T>({
    {R"(
    {{
    true -> 1,
    x -> 2
    }}
    )",          Value{1}},
    {R"(
    {{
    x -> 2,
    true -> 1
    }}
    )",          Value{1}},
    {R"(
    {{}}
    )",          Value::Undefined},
    {R"(
    {{
    x -> 2,
    y -> 1
    }}
    )",          Value::Undefined},
    {R"(
    {{
    false -> 1,
    2
    }}
    )",          Value{2}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedResult);
  }

  SECTION("Operator precedence")
  {
    using T = std::tuple<std::string, Value>;

    // clang-format off
    const auto
    [expression,                  expectedValue] = GENERATE(values<T>({
    {"1 + 2 - 3",                 Value{1.0 + 2.0 - 3.0}},
    {"1 - 2 + 3",                 Value{1.0 - 2.0 + 3.0}},
    {"2 * 3 + 4",                 Value{2.0 * 3.0 + 4.0}},
    {"2 + 3 * 4",                 Value{2.0 + 3.0 * 4.0}},
    {"2 * 3 - 4",                 Value{2.0 * 3.0 - 4.0}},
    {"2 - 3 * 4",                 Value{2.0 - 3.0 * 4.0}},
    {"6 / 2 + 4",                 Value{6.0 / 2.0 + 4}},
    {"6 + 2 / 4",                 Value{6.0 + 2.0 / 4.0}},
    {"6 / 2 - 4",                 Value{6.0 / 2.0 - 4.0}},
    {"6 - 2 / 4",                 Value{6.0 - 2.0 / 4.0}},
    {"2 * 6 / 4",                 Value{2.0 * 6.0 / 4.0}},
    {"2 / 6 * 4",                 Value{2.0 / 6.0 * 4.0}},
    {"2 + 3 * 4 + 5",             Value{2 + 3 * 4 + 5}},
    {"2 * 3 + 4 + 5",             Value{2 * 3 + 4 + 5}},
    {"2 * 3 + 4 & 5",             Value{2 * 3 + 4 & 5}},
    {"7 + 2 * 3 + 2 * 2",         Value{7 + 2 * 3 + 2 * 2}},
    {"7 + 2 / 3 + 2 * 2",         Value{7.0 + 2.0 / 3.0 + 2.0 * 2.0}},

    {"false && false || true",    Value{true}},
    {"!true && !true || !false",  Value{true}},

    {"3 < 10 || 10 > 2",          Value{true}},
    {"3 + 2 < 3 + 3 + 0 && true", Value{((3 + 2) < (3 + 3 + 0)) && true}},

    {"2 + 3 < 2 + 4",             Value{true}},

    {"(2+1)*3",                   Value{(2+1)*3}},
    {"(2+1)*((1+1)*2)",           Value{(2+1)*((1+1)*2)}},

    {"true && false -> true",     Value::Undefined},
    {"true && true -> false",     Value{false}},
    {"2 + 3 < 2 + 4 -> 6 % 5",    Value{1}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedValue);
  }

  SECTION("tryEvaluate")
  {
    using T = std::tuple<std::string, MapType, Value>;

    // clang-format off
    const auto
    [expression,           variables,           expectedValue] = GENERATE(values<T>({
    {"1",                  {},                  Value{1.0}},
    {"a",                  {{"a", Value{2.0}}}, Value{2.0}},
    {"1 + a",              {{"a", Value{2.0}}}, Value{3.0}},
    {"a",                  {},                  Value::Undefined},
    {"1 + a",              {},                  Value::Undefined},
    {"[a, 1, 2]",          {},                  Value{ArrayType{Value::Undefined, Value{1.0}, Value{2.0}}}},
    {"{a: 1, b: x, c: 3}", {},                  Value{MapType{{"a", Value{1.0}}, {"b", Value::Undefined}, {"c", Value{3.0}},}}},

    // Unlike evaluate, tryEvaluate swallows evaluation errors and yields undefined
    {"1 + 'a'",            {},                  Value::Undefined},
    {"[1 + 'a', 2]",       {},                  Value{ArrayType{Value::Undefined, Value{2.0}}}},
    {"{a: 1 + 'a'}",       {},                  Value{MapType{{"a", Value::Undefined}}}},
    {"1[0]",               {},                  Value::Undefined},
    {"vec('a', 2, 3)",     {},                  Value::Undefined},
    {"intersects(1, 2)",   {},                  Value::Undefined},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(tryEvaluate(expression, variables) == expectedValue);
  }

  SECTION("optimize")
  {
    using T = std::tuple<std::string, ExpressionNode>;

    // clang-format off
    const auto
    [expression,                expectedExpression] = GENERATE(values<T>({
    {"3 + 7",                   lit(10)},
    {"x == 1",                  eq(var("x"), lit(1))},
    {"[1, 2, 3]",               lit(ArrayType{Value{1}, Value{2}, Value{3}})},
    {"[1 + 2, 2, a]",           arr({lit(3), lit(2), var("a")})},
    {"{a:1, b:2, c:3}",         lit(MapType{{"a", Value{1}}, {"b", Value{2}}, {"c", Value{3}}})},
    {"{{ true -> 1, x -> 2 }}", lit(1)},
    {"{{ x -> 2, true -> 1 }}", swt({
                                    cs(var("x"), lit(2)),
                                    lit(1),
                                })},
    {"{{ x == 1 -> 2, 1 }}",    swt({
                                    cs(eq(var("x"), lit(1)), lit(2)),
                                    lit(1),
                                })},

    // An expression only folds if all of its operands are constant, even if an operand
    // would be short circuited
    {"false && x",              logAnd(lit(false), var("x"))},
    {"true || x",               logOr(lit(true), var("x"))},
    {"true && x",               logAnd(lit(true), var("x"))},
    {"false || x",              logOr(lit(false), var("x"))},

    // Unary expressions
    {"-1",                      lit(-1)},
    {"!true",                   lit(false)},
    {"-x",                      minus(var("x"))},

    // Subscript expressions
    {"'asdf'[1]",               lit("s")},
    {"'asdf'[1..2]",            lit("sd")},
    {"[1, 2, 3][1]",            lit(2)},
    {"{a:1, b:2}['a']",         lit(1)},
    {"x[1]",                    scr(var("x"), lit(1))},
    {"'asdf'[x]",               scr(lit("asdf"), var("x"))},

    // Dot access expressions
    {"{a:1}.a",                 lit(1)},
    {"x.a",                     dot(var("x"), "a")},

    // Function calls
    {"vec(1, 2, 3)",            lit(Vec3Type{1, 2, 3})},
    {"vec(x, 2, 3)",            call("vec", {var("x"), lit(2), lit(3)})},
    {"vec(1, 2, 3) distanceTo vec(1, 2, 3)", lit(0)},
    {"x distanceTo vec(1, 2, 3)", inf("distanceTo", var("x"), lit(Vec3Type{1, 2, 3}))},

    // An operand that isn't constant is never evaluated, so an operation that fails for
    // an undefined operand doesn't prevent the expression from being optimized
    {"distanceTo(x, vec(0, 0, 0)) < 1", ls(call("distanceTo", {var("x"), lit(Vec3Type{0, 0, 0})}), lit(1))},
    {"x intersects y",          inf("intersects", var("x"), var("y"))},
    {"{a:1}[x] == 1",           eq(scr(lit(MapType{{"a", Value{1}}}), var("x")), lit(1))},

    // A map with a non constant element cannot be folded
    {"{a:1, b:x}",              map({{"a", lit(1)}, {"b", var("x")}})},

    // An empty switch folds to undefined
    {"{{}}",                    lit(Value::Undefined)},

    // A case that evaluates to undefined is never selected, so it is dropped
    {"{{ undefined, 2 }}",      lit(2)},
    {"{{ false -> 1, 2 }}",     lit(2)},
    {"{{ false -> 1 }}",        lit(Value::Undefined)},
    {"{{ x -> 1, undefined, 2 }}", swt({cs(var("x"), lit(1)), lit(2)})},
    }));
    // clang-format on

    CAPTURE(expression);

    withEvaluationContext([&,
                           expression_ = expression,
                           expectedExpression_ = expectedExpression](auto& context) {
      CHECK(
        parseExpression(ParseMode::Strict, expression_).value().optimize(context)
        == expectedExpression_);
    }).ignore();
  }

  SECTION("asString")
  {
    using T = std::tuple<std::string, std::string>;

    // clang-format off
    const auto
    [expression,         expectedString] = GENERATE(values<T>({
    // Unary operators
    {"+1",               "+1"},
    {"-1",               "-1"},
    {"!true",            "!true"},
    {"~1",               "~1"},
    {"(1)",              "( 1 )"},
    {"'asdf'[1..]",      R"("asdf"[1..])"},
    {"'asdf'[..1]",      R"("asdf"[..1])"},

    // Binary operators
    {"1 + 2",            "1 + 2"},
    {"1 - 2",            "1 - 2"},
    {"1 * 2",            "1 * 2"},
    {"1 / 2",            "1 / 2"},
    {"1 % 2",            "1 % 2"},
    {"true && false",    "true && false"},
    {"true || false",    "true || false"},
    {"1 & 2",            "1 & 2"},
    {"1 ^ 2",            "1 ^ 2"},
    {"1 | 2",            "1 | 2"},
    {"1 << 2",           "1 << 2"},
    {"1 >> 2",           "1 >> 2"},
    {"1 < 2",            "1 < 2"},
    {"1 <= 2",           "1 <= 2"},
    {"1 > 2",            "1 > 2"},
    {"1 >= 2",           "1 >= 2"},
    {"1 == 2",           "1 == 2"},
    {"1 != 2",           "1 != 2"},
    {"'asdf'[1..2]",     R"("asdf"[1..2])"},
    {"true -> 1",        "true -> 1"},

    // Other expressions
    {"x",                "x"},
    {"[1, 2]",           "[1, 2]"},
    {"{a: 1}",           R"({ "a": 1 })"},
    {"{{ x -> 1, 2 }}",  "{{ x -> 1, 2 }}"},

    // Dot access round-trips as dot access, not as the equivalent bracket subscript
    {"x.y",              "x.y"},
    {"x.y.z",            "x.y.z"},
    {"x.y[0]",           "x.y[0]"},

    // Function calls
    {"f()",              "f()"},
    {"f(1)",             "f(1)"},
    {"f(1, 2)",          "f(1, 2)"},
    {"f(g())",           "f(g())"},

    // A constructed BBox round-trips through its own construction syntax
    {"bbox(vec(1, 2, 3), vec(4, 5, 6))", "bbox(vec(1, 2, 3), vec(4, 5, 6))"},

    // Infix calls round-trip as infix, not as the equivalent prefix call
    {"a like b",         "a like b"},
    {"a like b like c",  "a like b like c"},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(
      parseExpression(ParseMode::Strict, expression).value().asString()
      == expectedString);
  }

  SECTION("operator!=")
  {
    CHECK(lit(1) != lit(2));
    CHECK_FALSE(lit(1) != lit(1));

    CHECK(LiteralExpression{Value{1}} != LiteralExpression{Value{2}});
    CHECK(VariableExpression{"a"} != VariableExpression{"b"});
    CHECK(ArrayExpression{{lit(1)}} != ArrayExpression{{lit(2)}});
    CHECK(MapExpression{{{"a", lit(1)}}} != MapExpression{{{"b", lit(1)}}});
    CHECK(
      UnaryExpression{UnaryOperation::Minus, lit(1)}
      != UnaryExpression{UnaryOperation::Plus, lit(1)});
    CHECK(
      BinaryExpression{binop::Addition{}, lit(1), lit(2)}
      != BinaryExpression{binop::Subtraction{}, lit(1), lit(2)});
    CHECK(
      SubscriptExpression{lit("ab"), lit(0)} != SubscriptExpression{lit("ab"), lit(1)});
    CHECK(SwitchExpression{{lit(1)}} != SwitchExpression{{lit(2)}});

    // the operands are compared too, not just the operation
    CHECK(
      UnaryExpression{UnaryOperation::Minus, lit(1)}
      != UnaryExpression{UnaryOperation::Minus, lit(2)});
    CHECK(
      BinaryExpression{binop::Addition{}, lit(1), lit(2)}
      != BinaryExpression{binop::Addition{}, lit(3), lit(2)});
    CHECK(
      BinaryExpression{binop::Addition{}, lit(1), lit(2)}
      != BinaryExpression{binop::Addition{}, lit(1), lit(3)});
    CHECK(
      SubscriptExpression{lit("ab"), lit(0)} != SubscriptExpression{lit("cd"), lit(0)});
  }

  SECTION("optimize agrees with evaluate for every binary operator")
  {
    const auto expression = GENERATE(values<std::string>({
      "1 + 2",         "3 - 1",         "2 * 3",   "6 / 2",        "7 % 3",
      "true && false", "true || false", "12 & 10", "12 ^ 10",      "12 | 10",
      "1 << 3",        "8 >> 2",        "1 < 2",   "1 <= 2",       "1 > 2",
      "1 >= 2",        "1 == 2",        "1 != 2",  "'asdf'[1..2]", "true -> 1",
      "'ab' like 'b'", "[] contains 1", "1 is 1",  "1 in [1]",     "2 in [1]",
    }));

    CAPTURE(expression);

    withEvaluationContext([&, expression_ = expression](auto& context) {
      const auto expressionNode = parseExpression(ParseMode::Strict, expression_).value();
      CHECK(expressionNode.optimize(context) == lit(expressionNode.evaluate(context)));
    }).ignore();
  }

  SECTION("accept")
  {
    // clang-format off
    const auto
    [expression,     expectedVisits] = GENERATE(table<std::string, std::vector<std::string>>({
    {"1",            std::vector<std::string>{"1"}},
    {"a",            std::vector<std::string>{"a"}},
    {"[1, 2]",       std::vector<std::string>{"[1, 2]", "1", "2"}},
    {"{x:1, y:2}",   std::vector<std::string>{R"({ "x": 1, "y": 2 })", "1", "2"}},
    {"+1",           std::vector<std::string>{"+1", "1"}},
    {"1 + 2",        std::vector<std::string>{"1 + 2", "1", "2"}},
    {"x[1]",         std::vector<std::string>{"x[1]", "x", "1"}},
    {"{{ x -> 1 }}", std::vector<std::string>{"{{ x -> 1 }}", "x -> 1", "x", "1"}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(preorderVisit(expression) == expectedVisits);
  }
}

} // namespace tb::el

#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
