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

#include <cmath>

#include "EL/ELExceptions.h"
#include "EL/EvaluationContext.h"
#include "EL/Expression.h"
#include "EL/Expressions.h"
#include "EL/Value.h"
#include "EL/VariableStore.h"
#include "IO/ELParser.h"

#include <kdl/overload.h>

#include <map>
#include <string>
#include <variant>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace EL
{
using V = Value;

static Value evaluate(const std::string& expression, const MapType& variables = {})
{
  const auto context = EvaluationContext{VariableTable{variables}};
  return IO::ELParser::parseStrict(expression).evaluate(context);
}

TEST_CASE("ExpressionTest.testValueLiterals", "[ExpressionTest]")
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

TEST_CASE("ExpressionTest.testVariableExpression", "[ExpressionTest]")
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

TEST_CASE("ExpressionTest.testArrayExpression", "[ExpressionTest]")
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

TEST_CASE("ExpressionTest.testMapExpression", "[ExpressionTest]")
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

TEST_CASE("ExpressionTest.testOperators", "[ExpressionTest]")
{
  using T = std::tuple<std::string, std::variant<Value, EvaluationError>>;

  // clang-format off
  const auto
  [expression,        expectedValueOrError] = GENERATE(values<T>({
  // Unary plus
  {"+true",           Value{1}},
  {"+false",          Value{0}},
  {"+1",              Value{1}},
  {"+'test'",         EvaluationError{}},
  {"+null",           EvaluationError{}},
  {"+[]",             EvaluationError{}},
  {"+{}",             EvaluationError{}},

  // Unary minus
  {"-true",           Value{-1}},
  {"-false",          Value{0}},
  {"-1",              Value{-1}},
  {"-'test'",         EvaluationError{}},
  {"-null",           EvaluationError{}},
  {"-[]",             EvaluationError{}},
  {"-{}",             EvaluationError{}},

  // Addition
  {"true + true",     Value{2}},
  {"false + 3",       Value{3}},
  {"true + 'test'",   EvaluationError{}},
  {"true + null",     EvaluationError{}},
  {"true + []",       EvaluationError{}},
  {"true + {}",       EvaluationError{}},

  {"1 + true",        Value{2}},
  {"3 + -1",          Value{2}},
  {"1 + 'test'",      EvaluationError{}},
  {"1 + null",        EvaluationError{}},
  {"1 + []",          EvaluationError{}},
  {"1 + {}",          EvaluationError{}},

  {"'test' + true",   EvaluationError{}},
  {"'test' + 2",      EvaluationError{}},
  {"'this' + 'test'", Value{"thistest"}},
  {"'test' + null",   EvaluationError{}},
  {"'test' + []",     EvaluationError{}},
  {"'test' + {}",     EvaluationError{}},

  {"null + true",     EvaluationError{}},
  {"null + 2",        EvaluationError{}},
  {"null + 'test'",   EvaluationError{}},
  {"null + null",     EvaluationError{}},
  {"null + []",       EvaluationError{}},
  {"null + {}",       EvaluationError{}},

  {"[] + true",       EvaluationError{}},
  {"[] + 2",          EvaluationError{}},
  {"[] + 'test'",     EvaluationError{}},
  {"[] + null",       EvaluationError{}},
  {"[1, 2] + [2, 3]", Value{ArrayType{Value{1}, Value{2}, Value{2}, Value{3}}}},
  {"[] + {}",         EvaluationError{}},

  {"{} + true",       EvaluationError{}},
  {"{} + 2",          EvaluationError{}},
  {"{} + 'test'",     EvaluationError{}},
  {"{} + null",       EvaluationError{}},
  {"{} + []",         EvaluationError{}},
  {"{k1: 1, k2: 2, k3: 3} + {k3: 4, k4: 5}", Value{MapType{
      {"k1", Value{1}},
      {"k2", Value{2}},
      {"k3", Value{4}},
      {"k4", Value{5}},
  }}},

  // Subtraction
  {"true - true",     Value{0}},
  {"false - 3",       Value{-3}},
  {"true - 'test'",   EvaluationError{}},
  {"true - null",     EvaluationError{}},
  {"true - []",       EvaluationError{}},
  {"true - {}",       EvaluationError{}},

  {"1 - true",        Value{0}},
  {"3 - 1",           Value{2}},
  {"1 - 'test'",      EvaluationError{}},
  {"1 - null",        EvaluationError{}},
  {"1 - []",          EvaluationError{}},
  {"1 - {}",          EvaluationError{}},

  {"'test' - true",   EvaluationError{}},
  {"'test' - 2",      EvaluationError{}},
  {"'this' - 'test'", EvaluationError{}},
  {"'test' - null",   EvaluationError{}},
  {"'test' - []",     EvaluationError{}},
  {"'test' - {}",     EvaluationError{}},

  {"null - true",     EvaluationError{}},
  {"null - 2",        EvaluationError{}},
  {"null - 'test'",   EvaluationError{}},
  {"null - null",     EvaluationError{}},
  {"null - []",       EvaluationError{}},
  {"null - {}",       EvaluationError{}},

  {"[] - true",       EvaluationError{}},
  {"[] - 2",          EvaluationError{}},
  {"[] - 'test'",     EvaluationError{}},
  {"[] - null",       EvaluationError{}},
  {"[] - []",         EvaluationError{}},
  {"[] - {}",         EvaluationError{}},

  {"{} - true",       EvaluationError{}},
  {"{} - 2",          EvaluationError{}},
  {"{} - 'test'",     EvaluationError{}},
  {"{} - null",       EvaluationError{}},
  {"{} - []",         EvaluationError{}},
  {"{} - {}",         EvaluationError{}},

  // Multiplication
  {"true * true",     Value{1}},
  {"true * false",    Value{0}},
  {"true * 3",        Value{3}},
  {"true * 'test'",   EvaluationError{}},
  {"true * null",     EvaluationError{}},
  {"true * []",       EvaluationError{}},
  {"true * {}",       EvaluationError{}},

  {"1 * true",        Value{1}},
  {"3 * 2",           Value{6}},
  {"1 * 'test'",      EvaluationError{}},
  {"1 * null",        EvaluationError{}},
  {"1 * []",          EvaluationError{}},
  {"1 * {}",          EvaluationError{}},

  {"'test' * true",   EvaluationError{}},
  {"'test' * 2",      EvaluationError{}},
  {"'this' * 'test'", EvaluationError{}},
  {"'test' * null",   EvaluationError{}},
  {"'test' * []",     EvaluationError{}},
  {"'test' * {}",     EvaluationError{}},

  {"null * true",     EvaluationError{}},
  {"null * 2",        EvaluationError{}},
  {"null * 'test'",   EvaluationError{}},
  {"null * null",     EvaluationError{}},
  {"null * []",       EvaluationError{}},
  {"null * {}",       EvaluationError{}},

  {"[] * true",       EvaluationError{}},
  {"[] * 2",          EvaluationError{}},
  {"[] * 'test'",     EvaluationError{}},
  {"[] * null",       EvaluationError{}},
  {"[] * []",         EvaluationError{}},
  {"[] * {}",         EvaluationError{}},

  {"{} * true",       EvaluationError{}},
  {"{} * 2",          EvaluationError{}},
  {"{} * 'test'",     EvaluationError{}},
  {"{} * null",       EvaluationError{}},
  {"{} * []",         EvaluationError{}},
  {"{} * {}",         EvaluationError{}},

  // Division
  {"true / true",     Value{1}},
  {"true / false",    Value{std::numeric_limits<NumberType>::infinity()}},
  {"true / 3",        Value{1.0/3.0}},
  {"true / 'test'",   EvaluationError{}},
  {"true / null",     EvaluationError{}},
  {"true / []",       EvaluationError{}},
  {"true / {}",       EvaluationError{}},

  {"1 / true",        Value{1}},
  {"3 / 2",           Value{1.5}},
  {"1 / 'test'",      EvaluationError{}},
  {"1 / null",        EvaluationError{}},
  {"1 / []",          EvaluationError{}},
  {"1 / {}",          EvaluationError{}},

  {"'test' / true",   EvaluationError{}},
  {"'test' / 2",      EvaluationError{}},
  {"'this' / 'test'", EvaluationError{}},
  {"'test' / null",   EvaluationError{}},
  {"'test' / []",     EvaluationError{}},
  {"'test' / {}",     EvaluationError{}},

  {"null / true",     EvaluationError{}},
  {"null / 2",        EvaluationError{}},
  {"null / 'test'",   EvaluationError{}},
  {"null / null",     EvaluationError{}},
  {"null / []",       EvaluationError{}},
  {"null / {}",       EvaluationError{}},

  {"[] / true",       EvaluationError{}},
  {"[] / 2",          EvaluationError{}},
  {"[] / 'test'",     EvaluationError{}},
  {"[] / null",       EvaluationError{}},
  {"[] / []",         EvaluationError{}},
  {"[] / {}",         EvaluationError{}},

  {"{} / true",       EvaluationError{}},
  {"{} / 2",          EvaluationError{}},
  {"{} / 'test'",     EvaluationError{}},
  {"{} / null",       EvaluationError{}},
  {"{} / []",         EvaluationError{}},
  {"{} / {}",         EvaluationError{}},


  // Modulus
  {"true % true",     Value{0}},
  {"true % -2",       Value{1}},
  {"true % 'test'",   EvaluationError{}},
  {"true % null",     EvaluationError{}},
  {"true % []",       EvaluationError{}},
  {"true % {}",       EvaluationError{}},

  {"3 % -2",          Value{1}},
  {"1 % 'test'",      EvaluationError{}},
  {"1 % null",        EvaluationError{}},
  {"1 % []",          EvaluationError{}},
  {"1 % {}",          EvaluationError{}},

  {"'test' % true",   EvaluationError{}},
  {"'test' % 2",      EvaluationError{}},
  {"'this' % 'test'", EvaluationError{}},
  {"'test' % null",   EvaluationError{}},
  {"'test' % []",     EvaluationError{}},
  {"'test' % {}",     EvaluationError{}},

  {"null % true",     EvaluationError{}},
  {"null % 2",        EvaluationError{}},
  {"null % 'test'",   EvaluationError{}},
  {"null % null",     EvaluationError{}},
  {"null % []",       EvaluationError{}},
  {"null % {}",       EvaluationError{}},

  {"[] % true",       EvaluationError{}},
  {"[] % 2",          EvaluationError{}},
  {"[] % 'test'",     EvaluationError{}},
  {"[] % null",       EvaluationError{}},
  {"[] % []",         EvaluationError{}},
  {"[] % {}",         EvaluationError{}},

  {"{} % true",       EvaluationError{}},
  {"{} % 2",          EvaluationError{}},
  {"{} % 'test'",     EvaluationError{}},
  {"{} % null",       EvaluationError{}},
  {"{} % []",         EvaluationError{}},
  {"{} % {}",         EvaluationError{}},

  // Logical negation
  {"!true",           Value{false}},
  {"!false",          Value{true}},
  {"!1",              EvaluationError{}},
  {"!'test'",         EvaluationError{}},
  {"!null",           EvaluationError{}},
  {"![]",             EvaluationError{}},
  {"!{}",             EvaluationError{}},

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
  {"~true",           EvaluationError{}},
  {"~'asdf'",         EvaluationError{}},
  {"~null",           EvaluationError{}},
  {"~[]",             EvaluationError{}},
  {"~{}",             EvaluationError{}},

  // Bitwise and
  {"0 & 0",           Value{0 & 0}},
  {"123 & 456",       Value{123 & 456}},
  {"true & 123",      Value{1 & 123}},
  {"123 & true",      Value{123 & 1}},
  {"'asdf' & 123",    EvaluationError{}},
  {"123 & 'asdf'",    EvaluationError{}},
  {"null & 123",      Value{0 & 123}},
  {"123 & null",      Value{123 & 0}},
  {"[] & 123",        EvaluationError{}},
  {"123 & []",        EvaluationError{}},
  {"{} & 123",        EvaluationError{}},
  {"123 & {}",        EvaluationError{}},

  // Bitwise or
  {"0 | 0",           Value{0 | 0}},
  {"123 | 456",       Value{123 | 456}},
  {"true | 123",      Value{1 | 123}},
  {"123 | true",      Value{123 | 1}},
  {"'asdf' | 123",    EvaluationError{}},
  {"123 | 'asdf'",    EvaluationError{}},
  {"null | 123",      Value{0 | 123}},
  {"123 | null",      Value{123 | 0}},
  {"[] | 123",        EvaluationError{}},
  {"123 | []",        EvaluationError{}},
  {"{} | 123",        EvaluationError{}},
  {"123 | {}",        EvaluationError{}},

  // Bitwise xor
  {"0 ^ 0",           Value{0 ^ 0}},
  {"123 ^ 456",       Value{123 ^ 456}},
  {"true ^ 123",      Value{1 ^ 123}},
  {"123 ^ true",      Value{123 ^ 1}},
  {"'asdf' ^ 123",    EvaluationError{}},
  {"123 ^ 'asdf'",    EvaluationError{}},
  {"null ^ 123",      Value{0 ^ 123}},
  {"123 ^ null",      Value{123 ^ 0}},
  {"[] ^ 123",        EvaluationError{}},
  {"123 ^ []",        EvaluationError{}},
  {"{} ^ 123",        EvaluationError{}},
  {"123 ^ {}",        EvaluationError{}},

  // Bitwise shift left
  {"1 << 2",          Value{1 << 2}},
  {"true << 2",       Value{1 << 2}},
  {"1 << false",      Value{1 << 0}},
  {"'asdf' << 2",     EvaluationError{}},
  {"1 << 'asdf'",     EvaluationError{}},
  {"null << 2",       Value{0 << 2}},
  {"1 << null",       Value{1 << 0}},
  {"[] << 2",         EvaluationError{}},
  {"1 << []",         EvaluationError{}},
  {"{} << 2",         EvaluationError{}},
  {"1 << {}",         EvaluationError{}},

  // Bitwise shift right
  {"1 >> 2",          Value{1 >> 2}},
  {"true >> 2",       Value{1 >> 2}},
  {"1 >> false",      Value{1 >> 0}},
  {"'asdf' >> 2",     EvaluationError{}},
  {"1 >> 'asdf'",     EvaluationError{}},
  {"null >> 2",       Value{0 >> 2}},
  {"1 >> null",       Value{1 >> 0}},
  {"[] >> 2",         EvaluationError{}},
  {"1 >> []",         EvaluationError{}},
  {"{} >> 2",         EvaluationError{}},
  {"1 >> {}",         EvaluationError{}},

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
  {"false < []",      EvaluationError{}},
  {"false < {}",      EvaluationError{}},

  {"0 < 0",           Value{false}},
  {"0 < 1",           Value{true}},
  {"0 < 'true'",      EvaluationError{}},
  {"0 < 'false'",     EvaluationError{}},
  {"0 < ''",          Value{false}},
  {"0 < '1'",         Value{true}},
  {"0 < null",        Value{false}},
  {"0 < []",          EvaluationError{}},
  {"0 < {}",          EvaluationError{}},

  {"'a' < 0",         EvaluationError{}},
  {"'a' < 1",         EvaluationError{}},
  {"'a' < 'true'",    Value{true}},
  {"'a' < 'false'",   Value{true}},
  {"'a' < ''",        Value{false}},
  {"'a' < 'b'",       Value{true}},
  {"'a' < 'a'",       Value{false}},
  {"'aa' < 'ab'",     Value{true}},
  {"'a' < null",      Value{false}},
  {"'a' < []",        EvaluationError{}},
  {"'a' < {}",        EvaluationError{}},

  {"null < true",     Value{true}},
  {"null < false",    Value{true}},
  {"null < 0",        Value{true}},
  {"null < 1",        Value{true}},
  {"null < ''",       Value{true}},
  {"null < 'a'",      Value{true}},
  {"null < null",     Value{false}},
  {"null < []",       Value{true}},
  {"null < {}",       Value{true}},

  {"[] < true",       EvaluationError{}},
  {"[] < false",      EvaluationError{}},
  {"[] < 0",          EvaluationError{}},
  {"[] < 1",          EvaluationError{}},
  {"[] < ''",         EvaluationError{}},
  {"[] < 'a'",        EvaluationError{}},
  {"[] < null",       Value{false}},
  {"[] < []",         Value{false}},
  {"[1] < [1]",       Value{false}},
  {"[1] < [2]",       Value{true}},
  {"[1] < [1,2]",     Value{true}},
  {"[1,2] < [1,2]",   Value{false}},
  {"[1,2] < [1,2,3]", Value{true}},
  {"[1,2,3] < [1,2]", Value{false}},
  {"[] < {}",         EvaluationError{}},

  {"{} < true",             EvaluationError{}},
  {"{} < false",            EvaluationError{}},
  {"{} < 0",                EvaluationError{}},
  {"{} < 1",                EvaluationError{}},
  {"{} < ''",               EvaluationError{}},
  {"{} < 'a'",              EvaluationError{}},
  {"{} < null",             Value{false}},
  {"{} < []",               EvaluationError{}},
  {"{} < {}",               Value{false}},
  {"{k1:1} < {k1:1}",       Value{false}},
  {"{k1:1} < {k2:1}",       Value{true}},
  {"{k2:1} < {k1:1}",       Value{false}},
  {"{k1:1} < {k1:2}",       Value{true}},
  {"{k1:1} < {k1:1, k2:2}", Value{true}},
  {"{k1:1} < {k1:2, k2:2}", Value{true}},

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
  {"false == []",      EvaluationError{}},
  {"false == {}",      EvaluationError{}},

  {"0 == 0",           Value{true}},
  {"0 == 1",           Value{false}},
  {"0 == 'true'",      EvaluationError{}},
  {"0 == 'false'",     EvaluationError{}},
  {"0 == ''",          Value{true}},
  {"0 == '1'",         Value{false}},
  {"0 == null",        Value{false}},
  {"0 == []",          EvaluationError{}},
  {"0 == {}",          EvaluationError{}},

  {"'a' == 0",         EvaluationError{}},
  {"'a' == 1",         EvaluationError{}},
  {"'a' == 'b'",       Value{false}},
  {"'a' == 'a'",       Value{true}},
  {"'aa' == 'ab'",     Value{false}},
  {"'a' == null",      Value{false}},
  {"'a' == []",        EvaluationError{}},
  {"'a' == {}",        EvaluationError{}},

  {"null == true",     Value{false}},
  {"null == false",    Value{false}},
  {"null == 0",        Value{false}},
  {"null == 1",        Value{false}},
  {"null == ''",       Value{false}},
  {"null == 'a'",      Value{false}},
  {"null == null",     Value{true}},
  {"null == []",       Value{false}},
  {"null == {}",       Value{false}},

  {"[] == true",       EvaluationError{}},
  {"[] == false",      EvaluationError{}},
  {"[] == 0",          EvaluationError{}},
  {"[] == 1",          EvaluationError{}},
  {"[] == ''",         EvaluationError{}},
  {"[] == 'a'",        EvaluationError{}},
  {"[] == null",       Value{false}},
  {"[] == []",         Value{true}},
  {"[1] == [1]",       Value{true}},
  {"[1] == [2]",       Value{false}},
  {"[1] == [1,2]",     Value{false}},
  {"[1,2] == [1,2]",   Value{true}},
  {"[1,2] == [1,2,3]", Value{false}},
  {"[1,2,3] == [1,2]", Value{false}},
  {"[] == {}",         EvaluationError{}},

  {"{} == true",             EvaluationError{}},
  {"{} == false",            EvaluationError{}},
  {"{} == 0",                EvaluationError{}},
  {"{} == 1",                EvaluationError{}},
  {"{} == ''",               EvaluationError{}},
  {"{} == 'a'",              EvaluationError{}},
  {"{} == null",             Value{false}},
  {"{} == []",               EvaluationError{}},
  {"{} == {}",               Value{true}},
  {"{k1:1} == {k1:1}",       Value{true}},
  {"{k1:1} == {k2:1}",       Value{false}},
  {"{k2:1} == {k1:1}",       Value{false}},
  {"{k1:1} == {k1:2}",       Value{false}},
  {"{k1:1} == {k1:1, k2:2}", Value{false}},
  {"{k1:1} == {k1:2, k2:2}", Value{false}},

  {"true -> 'asdf'",  Value{"asdf"}},
  {"false -> 'asdf'", Value::Undefined},
  {"false -> x[-1]",  Value::Undefined},
  }));
  // clang-format on

  CAPTURE(expression);

  if (std::holds_alternative<Value>(expectedValueOrError))
  {
    const auto expectedValue = std::get<Value>(expectedValueOrError);
    CHECK(evaluate(expression) == expectedValue);
  }
  else
  {
    CHECK_THROWS_AS(evaluate(expression), EvaluationError);
  }
}

TEST_CASE("ExpressionTest.testOperatorPrecedence", "[ExpressionTest]")
{
  using T = std::tuple<std::string, Value>;

  // clang-format off
  const auto 
  [expression,                 expectedValue] = GENERATE(values<T>({
  {"1 + 2 - 3",                Value{1.0 + 2.0 - 3.0}},
  {"1 - 2 + 3",                Value{1.0 - 2.0 + 3.0}},
  {"2 * 3 + 4",                Value{2.0 * 3.0 + 4.0}},
  {"2 + 3 * 4",                Value{2.0 + 3.0 * 4.0}},
  {"2 * 3 - 4",                Value{2.0 * 3.0 - 4.0}},
  {"2 - 3 * 4",                Value{2.0 - 3.0 * 4.0}},
  {"6 / 2 + 4",                Value{6.0 / 2.0 + 4}},
  {"6 + 2 / 4",                Value{6.0 + 2.0 / 4.0}},
  {"6 / 2 - 4",                Value{6.0 / 2.0 - 4.0}},
  {"6 - 2 / 4",                Value{6.0 - 2.0 / 4.0}},
  {"2 * 6 / 4",                Value{2.0 * 6.0 / 4.0}},
  {"2 / 6 * 4",                Value{2.0 / 6.0 * 4.0}},
  {"2 + 3 * 4 + 5",            Value{2 + 3 * 4 + 5}},
  {"2 * 3 + 4 + 5",            Value{2 * 3 + 4 + 5}},
  {"2 * 3 + 4 & 5",            Value{2 * 3 + 4 & 5}},

  {"false && false || true",   Value{true}},
  {"!true && !true || !false", Value{true}},
  
  {"3 < 10 || 10 > 2",         Value{true}},

  {"2 + 3 < 2 + 4",            Value{true}},

  {"true && false -> true",    Value::Undefined},
  {"true && true -> false",    Value{false}},
  {"2 + 3 < 2 + 4 -> 6 % 5",   Value{1}},
  }));
  // clang-format on

  CAPTURE(expression);

  CHECK(evaluate(expression) == expectedValue);
}

TEST_CASE("ExpressionTest.testOptimize")
{
  using T = std::tuple<std::string, Expression>;

  // clang-format off
  const auto
  [expression,        expectedExpression] = GENERATE(values<T>({
  {"3 + 7",           Expression{LiteralExpression{Value{10}}, 0, 0}},
  {"[1, 2, 3]",       Expression{LiteralExpression{Value{ArrayType{Value{1}, Value{2}, Value{3}}}}, 0, 0}},
  {"[1 + 2, 2, a]",   Expression{ArrayExpression{{
                          Expression{LiteralExpression{Value{3}}, 0, 0}, 
                          Expression{LiteralExpression{Value{2}}, 0, 0}, 
                          Expression{VariableExpression{"a"}, 0, 0}}
                      }, 0, 0}},
  {"{a:1, b:2, c:3}", Expression{LiteralExpression{Value{MapType{{"a", Value{1}}, {"b", Value{2}}, {"c", Value{3}}}}}, 0, 0}},
  }));
  // clang-format on

  CAPTURE(expression);

  CHECK(IO::ELParser::parseStrict(expression).optimize() == expectedExpression);
}
} // namespace EL
} // namespace TrenchBroom
