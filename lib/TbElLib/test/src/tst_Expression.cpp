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

    // Logical disjunction
    {"false || false",  Value{false}},
    {"false || true",   Value{true}},
    {"true || false",   Value{true}},
    {"true || true",    Value{true}},

    // Logical short circuit evaluation
    {"false && x[-1]",  Value{false}},
    {"true || x[-1]",   Value{true}},

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

    // Case
    {"true -> 'asdf'",  Value{"asdf"}},
    {"false -> 'asdf'", Value::Undefined},
    {"false -> x[-1]",  Value::Undefined},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedResult);
  }

  SECTION("Subscript")
  {
    using T = std::tuple<std::string, Result<Value>>;

    // clang-format off
    const auto
    [expression,                         expectedResult] = GENERATE(values<T>({
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

    // For Maps
    {"{a: 1, b: 2, c: 3}['a']",          Value{1}},
    {"{a: 1, b: 2, c: 3}['b']",          Value{2}},
    {"{a: 1, b: 2, c: 3}['c']",          Value{3}},

    // Out of bounds
    {"'asdf'[5]",                        Value{""}},
    {"[0, 1, 2, 3][5]",                  Error{"At line 1, column 13: Cannot evaluate expression '[0, 1, 2, 3][5]': 4 is out of bounds for '[0, 1, 2, 3]'"}},
    {"{a: 1, b: 2, c: 3}['d']",          Value::Undefined},

    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression) == expectedResult);
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
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(evaluate(expression, variables) == expectedValue);
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

  SECTION("accept")
  {
    CHECK(preorderVisit("1") == std::vector<std::string>{"1"});
    CHECK(preorderVisit("a") == std::vector<std::string>{"a"});
    CHECK(preorderVisit("[1, 2]") == std::vector<std::string>{"[1, 2]", "1", "2"});
    CHECK(
      preorderVisit("{x:1, y:2}")
      == std::vector<std::string>{R"({ "x": 1, "y": 2 })", "1", "2"});
    CHECK(preorderVisit("+1") == std::vector<std::string>{"+1", "1"});
    CHECK(preorderVisit("1 + 2") == std::vector<std::string>{"1 + 2", "1", "2"});
    CHECK(preorderVisit("x[1]") == std::vector<std::string>{"x[1]", "x", "1"});
    CHECK(
      preorderVisit("{{ x -> 1 }}")
      == std::vector<std::string>{"{{ x -> 1 }}", "x -> 1", "x", "1"});
  }
}

} // namespace tb::el

#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
