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

#include "Matchers.h"
#include "el/ELTestUtils.h"
#include "el/EvaluationContext.h"
#include "el/Exceptions.h"
#include "el/Expression.h"
#include "el/Value.h"
#include "el/VariableStore.h"
#include "io/ELParser.h"

#include <fmt/ostream.h>

#include <string>
#include <vector>

#include "catch/CatchConfig.h"

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
      return io::ELParser::parseStrict(expression).value().evaluate(context);
    },
    VariableTable{variables});
}

std::vector<std::string> preorderVisit(const std::string& str)
{
  auto result = std::vector<std::string>{};

  io::ELParser::parseStrict(str).value().accept(kdl::overload(
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
    {"+'test'",         Error{}},
    {"+'2'",            Value{2}},
    {"+null",           Error{}},
    {"+[]",             Error{}},
    {"+{}",             Error{}},

    // Unary minus
    {"-true",           Value{-1}},
    {"-false",          Value{0}},
    {"-1",              Value{-1}},
    {"-'2'",            Value{-2}},
    {"-'test'",         Error{}},
    {"-null",           Error{}},
    {"-[]",             Error{}},
    {"-{}",             Error{}},

    // Addition
    {"true + true",     Value{2}},
    {"false + 3",       Value{3}},
    {"true + 'test'",   Error{}},
    {"true + '1.23'",   Value{2.23}},
    {"true + null",     Error{}},
    {"true + []",       Error{}},
    {"true + {}",       Error{}},

    {"1 + true",        Value{2}},
    {"3 + -1",          Value{2}},
    {"1 + '1.23'",      Value{2.23}},
    {"1 + 'test'",      Error{}},
    {"1 + null",        Error{}},
    {"1 + []",          Error{}},
    {"1 + {}",          Error{}},

    {"'test' + true",   Error{}},
    {"'test' + 2",      Error{}},
    {"'1.23' + 2",      Value{3.23}},
    {"'this' + 'test'", Value{"thistest"}},
    {"'this' + '1.23'", Value{"this1.23"}},
    {"'test' + null",   Error{}},
    {"'test' + []",     Error{}},
    {"'test' + {}",     Error{}},

    {"null + true",     Error{}},
    {"null + 2",        Error{}},
    {"null + 'test'",   Error{}},
    {"null + null",     Error{}},
    {"null + []",       Error{}},
    {"null + {}",       Error{}},

    {"[] + true",       Error{}},
    {"[] + 2",          Error{}},
    {"[] + 'test'",     Error{}},
    {"[] + null",       Error{}},
    {"[1, 2] + [2, 3]", Value{ArrayType{Value{1}, Value{2}, Value{2}, Value{3}}}},
    {"[] + {}",         Error{}},

    {"{} + true",       Error{}},
    {"{} + 2",          Error{}},
    {"{} + 'test'",     Error{}},
    {"{} + null",       Error{}},
    {"{} + []",         Error{}},
    {"{k1: 1, k2: 2, k3: 3} + {k3: 4, k4: 5}", Value{MapType{
        {"k1", Value{1}},
        {"k2", Value{2}},
        {"k3", Value{4}},
        {"k4", Value{5}},
    }}},

    // Subtraction
    {"true - true",     Value{0}},
    {"false - 3",       Value{-3}},
    {"true - 'test'",   Error{}},
    {"true - '2.0'",    Value{-1.0}},
    {"true - null",     Error{}},
    {"true - []",       Error{}},
    {"true - {}",       Error{}},

    {"1 - true",        Value{0}},
    {"3 - 1",           Value{2}},
    {"1 - 'test'",      Error{}},
    {"1 - '2.23'",      Value{-1.23}},
    {"1 - null",        Error{}},
    {"1 - []",          Error{}},
    {"1 - {}",          Error{}},

    {"'test' - true",   Error{}},
    {"'test' - 2",      Error{}},
    {"'3.23' - 2",      Value{1.23}},
    {"'this' - 'test'", Error{}},
    {"'test' - null",   Error{}},
    {"'test' - []",     Error{}},
    {"'test' - {}",     Error{}},

    {"null - true",     Error{}},
    {"null - 2",        Error{}},
    {"null - 'test'",   Error{}},
    {"null - null",     Error{}},
    {"null - []",       Error{}},
    {"null - {}",       Error{}},

    {"[] - true",       Error{}},
    {"[] - 2",          Error{}},
    {"[] - 'test'",     Error{}},
    {"[] - null",       Error{}},
    {"[] - []",         Error{}},
    {"[] - {}",         Error{}},

    {"{} - true",       Error{}},
    {"{} - 2",          Error{}},
    {"{} - 'test'",     Error{}},
    {"{} - null",       Error{}},
    {"{} - []",         Error{}},
    {"{} - {}",         Error{}},

    // Multiplication
    {"true * true",     Value{1}},
    {"true * false",    Value{0}},
    {"true * 3",        Value{3}},
    {"true * '3'",      Value{3}},
    {"true * 'test'",   Error{}},
    {"true * null",     Error{}},
    {"true * []",       Error{}},
    {"true * {}",       Error{}},

    {"1 * true",        Value{1}},
    {"3 * 2",           Value{6}},
    {"1 * 'test'",      Error{}},
    {"2 * '2.23'",      Value{4.46}},
    {"1 * null",        Error{}},
    {"1 * []",          Error{}},
    {"1 * {}",          Error{}},

    {"'test' * true",   Error{}},
    {"'test' * 2",      Error{}},
    {"'1.23' * 2",      Value{2.46}},
    {"'this' * 'test'", Error{}},
    {"'test' * null",   Error{}},
    {"'test' * []",     Error{}},
    {"'test' * {}",     Error{}},

    {"null * true",     Error{}},
    {"null * 2",        Error{}},
    {"null * 'test'",   Error{}},
    {"null * null",     Error{}},
    {"null * []",       Error{}},
    {"null * {}",       Error{}},

    {"[] * true",       Error{}},
    {"[] * 2",          Error{}},
    {"[] * 'test'",     Error{}},
    {"[] * null",       Error{}},
    {"[] * []",         Error{}},
    {"[] * {}",         Error{}},

    {"{} * true",       Error{}},
    {"{} * 2",          Error{}},
    {"{} * 'test'",     Error{}},
    {"{} * null",       Error{}},
    {"{} * []",         Error{}},
    {"{} * {}",         Error{}},

    // Division
    {"true / true",     Value{1}},
    {"true / false",    Value{std::numeric_limits<NumberType>::infinity()}},
    {"true / 3",        Value{1.0/3.0}},
    {"true / '2'",      Value{1.0/2.0}},
    {"true / 'test'",   Error{}},
    {"true / null",     Error{}},
    {"true / []",       Error{}},
    {"true / {}",       Error{}},

    {"1 / true",        Value{1}},
    {"3 / 2",           Value{1.5}},
    {"1 / 'test'",      Error{}},
    {"1 / '3.0'",       Value{1.0/3.0}},
    {"1 / null",        Error{}},
    {"1 / []",          Error{}},
    {"1 / {}",          Error{}},

    {"'test' / true",   Error{}},
    {"'test' / 2",      Error{}},
    {"'3' / 2",         Value{3.0/2.0}},
    {"'this' / 'test'", Error{}},
    {"'test' / null",   Error{}},
    {"'test' / []",     Error{}},
    {"'test' / {}",     Error{}},

    {"null / true",     Error{}},
    {"null / 2",        Error{}},
    {"null / 'test'",   Error{}},
    {"null / null",     Error{}},
    {"null / []",       Error{}},
    {"null / {}",       Error{}},

    {"[] / true",       Error{}},
    {"[] / 2",          Error{}},
    {"[] / 'test'",     Error{}},
    {"[] / null",       Error{}},
    {"[] / []",         Error{}},
    {"[] / {}",         Error{}},

    {"{} / true",       Error{}},
    {"{} / 2",          Error{}},
    {"{} / 'test'",     Error{}},
    {"{} / null",       Error{}},
    {"{} / []",         Error{}},
    {"{} / {}",         Error{}},


    // Modulus
    {"true % true",     Value{0}},
    {"true % -2",       Value{1}},
    {"true % 'test'",   Error{}},
    {"true % '1'",      Value{0}},
    {"true % null",     Error{}},
    {"true % []",       Error{}},
    {"true % {}",       Error{}},

    {"3 % -2",          Value{1}},
    {"3 % '-2'",        Value{1}},
    {"1 % 'test'",      Error{}},
    {"1 % null",        Error{}},
    {"1 % []",          Error{}},
    {"1 % {}",          Error{}},

    {"'test' % true",   Error{}},
    {"'test' % 2",      Error{}},
    {"'3' % 2",         Value{1}},
    {"'this' % 'test'", Error{}},
    {"'test' % null",   Error{}},
    {"'test' % []",     Error{}},
    {"'test' % {}",     Error{}},

    {"null % true",     Error{}},
    {"null % 2",        Error{}},
    {"null % 'test'",   Error{}},
    {"null % null",     Error{}},
    {"null % []",       Error{}},
    {"null % {}",       Error{}},

    {"[] % true",       Error{}},
    {"[] % 2",          Error{}},
    {"[] % 'test'",     Error{}},
    {"[] % null",       Error{}},
    {"[] % []",         Error{}},
    {"[] % {}",         Error{}},

    {"{} % true",       Error{}},
    {"{} % 2",          Error{}},
    {"{} % 'test'",     Error{}},
    {"{} % null",       Error{}},
    {"{} % []",         Error{}},
    {"{} % {}",         Error{}},

    // Logical negation
    {"!true",           Value{false}},
    {"!false",          Value{true}},
    {"!1",              Error{}},
    {"!'test'",         Error{}},
    {"!null",           Error{}},
    {"![]",             Error{}},
    {"!{}",             Error{}},

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
    {"~true",           Error{}},
    {"~'23423'",        Value{~23423}},
    {"~'asdf'",         Error{}},
    {"~null",           Error{}},
    {"~[]",             Error{}},
    {"~{}",             Error{}},

    // Bitwise and
    {"0 & 0",           Value{0 & 0}},
    {"123 & 456",       Value{123 & 456}},
    {"true & 123",      Value{1 & 123}},
    {"123 & true",      Value{123 & 1}},
    {"'asdf' & 123",    Error{}},
    {"'456' & 123",     Value{456 & 123}},
    {"123 & 'asdf'",    Error{}},
    {"123 & '456'",     Value{123 & 456}},
    {"null & 123",      Value{0 & 123}},
    {"123 & null",      Value{123 & 0}},
    {"[] & 123",        Error{}},
    {"123 & []",        Error{}},
    {"{} & 123",        Error{}},
    {"123 & {}",        Error{}},

    // Bitwise or
    {"0 | 0",           Value{0 | 0}},
    {"123 | 456",       Value{123 | 456}},
    {"true | 123",      Value{1 | 123}},
    {"123 | true",      Value{123 | 1}},
    {"'asdf' | 123",    Error{}},
    {"'456' | 123",     Value{456 | 123}},
    {"123 | 'asdf'",    Error{}},
    {"123 | '456'",     Value{123 | 456}},
    {"null | 123",      Value{0 | 123}},
    {"123 | null",      Value{123 | 0}},
    {"[] | 123",        Error{}},
    {"123 | []",        Error{}},
    {"{} | 123",        Error{}},
    {"123 | {}",        Error{}},

    // Bitwise xor
    {"0 ^ 0",           Value{0 ^ 0}},
    {"123 ^ 456",       Value{123 ^ 456}},
    {"true ^ 123",      Value{1 ^ 123}},
    {"123 ^ true",      Value{123 ^ 1}},
    {"'asdf' ^ 123",    Error{}},
    {"'456' ^ 123",     Value{456 ^ 123}},
    {"123 ^ 'asdf'",    Error{}},
    {"123 ^ '456'",     Value{123 ^ 456}},
    {"null ^ 123",      Value{0 ^ 123}},
    {"123 ^ null",      Value{123 ^ 0}},
    {"[] ^ 123",        Error{}},
    {"123 ^ []",        Error{}},
    {"{} ^ 123",        Error{}},
    {"123 ^ {}",        Error{}},

    // Bitwise shift left
    {"1 << 2",          Value{1 << 2}},
    {"true << 2",       Value{1 << 2}},
    {"1 << false",      Value{1 << 0}},
    {"'asdf' << 2",     Error{}},
    {"'1' << 2",        Value{1 << 2}},
    {"1 << 'asdf'",     Error{}},
    {"1 << '2'",        Value{1 << 2}},
    {"null << 2",       Value{0 << 2}},
    {"1 << null",       Value{1 << 0}},
    {"[] << 2",         Error{}},
    {"1 << []",         Error{}},
    {"{} << 2",         Error{}},
    {"1 << {}",         Error{}},

    // Bitwise shift right
    {"1 >> 2",          Value{1 >> 2}},
    {"true >> 2",       Value{1 >> 2}},
    {"1 >> false",      Value{1 >> 0}},
    {"'asdf' >> 2",     Error{}},
    {"'1' >> 2",        Value{1 >> 2}},
    {"1 >> 'asdf'",     Error{}},
    {"1 >> '2'",        Value{1 >> 2}},
    {"null >> 2",       Value{0 >> 2}},
    {"1 >> null",       Value{1 >> 0}},
    {"[] >> 2",         Error{}},
    {"1 >> []",         Error{}},
    {"{} >> 2",         Error{}},
    {"1 >> {}",         Error{}},

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
    {"false < []",      Error{}},
    {"false < {}",      Error{}},

    {"0 < 0",           Value{false}},
    {"0 < 1",           Value{true}},
    {"0 < 'true'",      Error{}},
    {"0 < 'false'",     Error{}},
    {"0 < ''",          Value{false}},
    {"0 < '0'",         Value{false}},
    {"0 < '1'",         Value{true}},
    {"0 < null",        Value{false}},
    {"0 < []",          Error{}},
    {"0 < {}",          Error{}},

    {"'a' < 0",         Error{}},
    {"'a' < 1",         Error{}},
    {"'a' < 'true'",    Value{true}},
    {"'a' < 'false'",   Value{true}},
    {"'a' < ''",        Value{false}},
    {"'a' < 'b'",       Value{true}},
    {"'a' < 'a'",       Value{false}},
    {"'aa' < 'ab'",     Value{true}},
    {"'a' < null",      Value{false}},
    {"'a' < []",        Error{}},
    {"'a' < {}",        Error{}},
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

    {"[] < true",       Error{}},
    {"[] < false",      Error{}},
    {"[] < 0",          Error{}},
    {"[] < 1",          Error{}},
    {"[] < ''",         Error{}},
    {"[] < 'a'",        Error{}},
    {"[] < null",       Value{false}},
    {"[] < []",         Value{false}},
    {"[1] < [1]",       Value{false}},
    {"[1] < [2]",       Value{true}},
    {"[1] < [1,2]",     Value{true}},
    {"[1,2] < [1,2]",   Value{false}},
    {"[1,2] < [1,2,3]", Value{true}},
    {"[1,2,3] < [1,2]", Value{false}},
    {"[] < {}",         Error{}},

    {"{} < true",             Error{}},
    {"{} < false",            Error{}},
    {"{} < 0",                Error{}},
    {"{} < 1",                Error{}},
    {"{} < ''",               Error{}},
    {"{} < 'a'",              Error{}},
    {"{} < null",             Value{false}},
    {"{} < []",               Error{}},
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
    {"false <= []",      Error{}},
    {"false <= {}",      Error{}},

    {"0 <= 0",           Value{true}},
    {"0 <= 1",           Value{true}},
    {"0 <= 'true'",      Error{}},
    {"0 <= 'false'",     Error{}},
    {"0 <= ''",          Value{true}},
    {"0 <= '0'",         Value{true}},
    {"0 <= '1'",         Value{true}},
    {"0 <= null",        Value{false}},
    {"0 <= []",          Error{}},
    {"0 <= {}",          Error{}},

    {"'a' <= 0",         Error{}},
    {"'a' <= 1",         Error{}},
    {"'a' <= 'true'",    Value{true}},
    {"'a' <= 'false'",   Value{true}},
    {"'a' <= ''",        Value{false}},
    {"'a' <= 'b'",       Value{true}},
    {"'a' <= 'a'",       Value{true}},
    {"'aa' <= 'ab'",     Value{true}},
    {"'a' <= null",      Value{false}},
    {"'a' <= []",        Error{}},
    {"'a' <= {}",        Error{}},
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

    {"[] <= true",       Error{}},
    {"[] <= false",      Error{}},
    {"[] <= 0",          Error{}},
    {"[] <= 1",          Error{}},
    {"[] <= ''",         Error{}},
    {"[] <= 'a'",        Error{}},
    {"[] <= null",       Value{false}},
    {"[] <= []",         Value{true}},
    {"[1] <= [1]",       Value{true}},
    {"[1] <= [2]",       Value{true}},
    {"[1] <= [1,2]",     Value{true}},
    {"[1,2] <= [1,2]",   Value{true}},
    {"[1,2] <= [1,2,3]", Value{true}},
    {"[1,2,3] <= [1,2]", Value{false}},
    {"[] <= {}",         Error{}},

    {"{} <= true",             Error{}},
    {"{} <= false",            Error{}},
    {"{} <= 0",                Error{}},
    {"{} <= 1",                Error{}},
    {"{} <= ''",               Error{}},
    {"{} <= 'a'",              Error{}},
    {"{} <= null",             Value{false}},
    {"{} <= []",               Error{}},
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
    {"[] > false",      Error{}},
    {"{} > false",      Error{}},

    {"0 > 0",           Value{false}},
    {"1 > 0",           Value{true}},
    {"'true' > 0",      Error{}},
    {"'false' > 0",     Error{}},
    {"'' > 0",          Value{false}},
    {"'0' > 0",         Value{false}},
    {"'1' > 0",         Value{true}},
    {"null > 0",        Value{false}},
    {"[] > 0",          Error{}},
    {"{} > 0",          Error{}},

    {"0 > 'a'",         Error{}},
    {"1 > 'a'",         Error{}},
    {"'true' > 'a'",    Value{true}},
    {"'false' > 'a'",   Value{true}},
    {"'' > 'a'",        Value{false}},
    {"'b' > 'a'",       Value{true}},
    {"'a' > 'a'",       Value{false}},
    {"'ab' > 'aa'",     Value{true}},
    {"null > 'a'",      Value{false}},
    {"[] > 'a'",        Error{}},
    {"{} > 'a'",        Error{}},
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

    {"true > []",       Error{}},
    {"false > []",      Error{}},
    {"0 > []",          Error{}},
    {"1 > []",          Error{}},
    {"'' > []",         Error{}},
    {"'a' > []",        Error{}},
    {"null > []",       Value{false}},
    {"[] > []",         Value{false}},
    {"[1] > [1]",       Value{false}},
    {"[2] > [1]",       Value{true}},
    {"[1,2] > [1]",     Value{true}},
    {"[1,2] > [1,2]",   Value{false}},
    {"[1,2,3] > [1,2]", Value{true}},
    {"[1,2] > [1,2,3]", Value{false}},
    {"{} > []",         Error{}},

    {"true > {}",             Error{}},
    {"false > {}",            Error{}},
    {"0 > {}",                Error{}},
    {"1 > {}",                Error{}},
    {"'' > {}",               Error{}},
    {"'a' > {}",              Error{}},
    {"null > {}",             Value{false}},
    {"[] > {}",               Error{}},
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
    {"[] >= false",      Error{}},
    {"{} >= false",      Error{}},

    {"0 >= 0",           Value{true}},
    {"1 >= 0",           Value{true}},
    {"'true' >= 0",      Error{}},
    {"'false' >= 0",     Error{}},
    {"'' >= 0",          Value{true}},
    {"'0' >= 0",         Value{true}},
    {"'1' >= 0",         Value{true}},
    {"null >= 0",        Value{false}},
    {"[] >= 0",          Error{}},
    {"{} >= 0",          Error{}},

    {"0 >= 'a'",         Error{}},
    {"1 >= 'a'",         Error{}},
    {"'true' >= 'a'",    Value{true}},
    {"'false' >= 'a'",   Value{true}},
    {"'' >= 'a'",        Value{false}},
    {"'b' >= 'a'",       Value{true}},
    {"'a' >= 'a'",       Value{true}},
    {"'ab' >= 'aa'",     Value{true}},
    {"null >= 'a'",      Value{false}},
    {"[] >= 'a'",        Error{}},
    {"{} >= 'a'",        Error{}},
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

    {"true >= []",       Error{}},
    {"false >= []",      Error{}},
    {"0 >= []",          Error{}},
    {"1 >= []",          Error{}},
    {"'' >= []",         Error{}},
    {"'a' >= []",        Error{}},
    {"null >= []",       Value{false}},
    {"[] >= []",         Value{true}},
    {"[1] >= [1]",       Value{true}},
    {"[2] >= [1]",       Value{true}},
    {"[1,2] >= [1]",     Value{true}},
    {"[1,2] >= [1,2]",   Value{true}},
    {"[1,2,3] >= [1,2]", Value{true}},
    {"[1,2] >= [1,2,3]", Value{false}},
    {"{} >= []",         Error{}},

    {"true >= {}",             Error{}},
    {"false >= {}",            Error{}},
    {"0 >= {}",                Error{}},
    {"1 >= {}",                Error{}},
    {"'' >= {}",               Error{}},
    {"'a' >= {}",              Error{}},
    {"null >= {}",             Value{false}},
    {"[] >= {}",               Error{}},
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
    {"false == []",      Error{}},
    {"false == {}",      Error{}},

    {"0 == 0",           Value{true}},
    {"0 == 1",           Value{false}},
    {"0 == 'true'",      Error{}},
    {"0 == 'false'",     Error{}},
    {"0 == ''",          Value{true}},
    {"0 == '0'",         Value{true}},
    {"0 == '1'",         Value{false}},
    {"0 == null",        Value{false}},
    {"0 == []",          Error{}},
    {"0 == {}",          Error{}},

    {"'a' == 0",         Error{}},
    {"'a' == 1",         Error{}},
    {"'a' == 'b'",       Value{false}},
    {"'a' == 'a'",       Value{true}},
    {"'aa' == 'ab'",     Value{false}},
    {"'a' == null",      Value{false}},
    {"'a' == []",        Error{}},
    {"'a' == {}",        Error{}},
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

    {"[] == true",       Error{}},
    {"[] == false",      Error{}},
    {"[] == 0",          Error{}},
    {"[] == 1",          Error{}},
    {"[] == ''",         Error{}},
    {"[] == 'a'",        Error{}},
    {"[] == null",       Value{false}},
    {"[] == []",         Value{true}},
    {"[1] == [1]",       Value{true}},
    {"[1] == [2]",       Value{false}},
    {"[1] == [1,2]",     Value{false}},
    {"[1,2] == [1,2]",   Value{true}},
    {"[1,2] == [1,2,3]", Value{false}},
    {"[1,2,3] == [1,2]", Value{false}},
    {"[] == {}",         Error{}},

    {"{} == true",             Error{}},
    {"{} == false",            Error{}},
    {"{} == 0",                Error{}},
    {"{} == 1",                Error{}},
    {"{} == ''",               Error{}},
    {"{} == 'a'",              Error{}},
    {"{} == null",             Value{false}},
    {"{} == []",               Error{}},
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

    CHECK_THAT(evaluate(expression), MatchesResult(expectedResult));
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
    {"[0, 1, 2, 3][5]",                  Error{}},
    {"{a: 1, b: 2, c: 3}['d']",          Value::Undefined},

    }));
    // clang-format on

    CAPTURE(expression);

    CHECK_THAT(evaluate(expression), MatchesResult(expectedResult));
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

    CHECK_THAT(evaluate(expression), MatchesResult(expectedResult));
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
        io::ELParser::parseStrict(expression_).value().optimize(context)
        == expectedExpression_);
    }).ignore();
  }

  SECTION("accept")
  {
    CHECK(preorderVisit("1") == std::vector<std::string>{"1"});
    CHECK(preorderVisit("a") == std::vector<std::string>{"a"});
    CHECK(preorderVisit("[1, 2]") == std::vector<std::string>{"[ 1, 2 ]", "1", "2"});
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
