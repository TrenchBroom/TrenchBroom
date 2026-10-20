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

#include "el/Parser.h"
#include "el/TestUtils.h"

#include "kd/result_io.h" // IWYU pragma: keep

#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::el
{
namespace
{

auto parse(const std::string& str)
{
  return Parser{ParseMode::Strict, str}.parse();
}

} // namespace

TEST_CASE("Parser")
{
  SECTION("emptyExpression")
  {
    // clang-format off
    const auto
    [expression, expectedError] = GENERATE(table<std::string, Error>({
    {"",         Error{"At line 1, column 1: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
    {"    ",     Error{"At line 1, column 5: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
    {"\n",       Error{"At line 2, column 1: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedError);
  }

  SECTION("Literals")
  {
    SECTION("Strings")
    {
      // clang-format off
      const auto
      [expression,            expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
      {R"("asdf)",            Error{"Unexpected end of file"}},
      {R"("asdf")",           lit("asdf")},
      {R"("asdf\" \"asdf")",  lit(R"(asdf" "asdf)")},
      {R"("asdf\' \'asdf")",  lit(R"(asdf\' \'asdf)")},

      {R"('asdf)",            Error{"Unexpected end of file"}},
      {R"('asdf')",           lit("asdf")},
      {R"('asdf\' \'asdf')",  lit("asdf' 'asdf")},
      {R"('asdf\" \"asdf')",  lit(R"(asdf\" \"asdf)")},

      {R"("C:\\quake")",      lit(R"(C:\quake)")},
      {R"("C:\quake")",       lit(R"(C:\quake)")},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedResult);
    }

    SECTION("Numbers")
    {
      // clang-format off
      const auto
      [expression, expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
      {"1.123.34", Error{"At line 1, column 2: Unexpected character: '.'"}},
      {"1",        lit(1.0)},
      {"1.0",      lit(1.0)},
      {"01.00",    lit(1.0)},
      {".0",       lit(0.0)},
      {"0",        lit(0.0)},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedResult);
    }

    SECTION("Booleans")
    {
      // clang-format off
      const auto
      [expression, expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"true",     lit(true)},
      {"false",    lit(false)},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Null and undefined")
    {
      // clang-format off
      const auto
      [expression,  expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"null",      lit(Value::Null)},
      {"undefined", lit(Value::Undefined)},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Arrays")
    {
      // clang-format off
      const auto
      [expression,                    expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"[]",                          arr()},
      {R"([ 1.0 , "test",[ true] ])", arr({lit(1.0), lit("test"), arr({lit(true)})})},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Ranges")
    {
      // clang-format off
      const auto
      [expression, expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"[1..3]",   arr({bRng(lit(1.0), lit(3.0))})},
      {"[][1..3]", scr(arr(), bRng(lit(1.0), lit(3.0)))},
      {"[][..3]",  scr(arr(), rbRng(lit(3.0)))},
      {"[][1..]",  scr(arr(), lbRng(lit(1.0)))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Maps")
    {
      // clang-format off
      const auto
      [expression,                                                                   expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
      {"{}",                                                                         map()},
      {R"( { "testkey1": 1, "testkey2"   :"asdf", "testkey3":{"nestedKey":true} })", map({{"testkey1", lit(1.0)}, {"testkey2", lit("asdf")}, {"testkey3", map({{"nestedKey", lit(true)}})}})},
      {R"([ { "key": "value" } ])",                                                  arr({map({{"key", lit("value")}})})},
      {R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })",            map({{"outerkey1", arr({map({{"key", lit("value")}})})}, {"outerkey2", lit("asdf")}})},
      {R"({
  "profiles": [],
  "version": 1
}
asdf)",                                                                              Error{"At line 5, column 1: Expected end of file, but got variable (raw data: 'asdf')"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedResult);
    }
  }

  SECTION("Variables")
  {
    CHECK(parse("test") == var("test"));
  }

  SECTION("Unary operators")
  {
    SECTION("Unary plus")
    {
      CHECK(parse("+1.0") == plus(lit(1)));
    }

    SECTION("Unary minus")
    {
      CHECK(parse("-1.0") == minus(lit(1)));
    }

    SECTION("Logical negation")
    {
      // clang-format off
      const auto
      [expression, expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"!true",    logNeg(lit(true))},
      {"!false",   logNeg(lit(false))},
      {"!0",       logNeg(lit(0))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Bitwise negation")
    {
      // clang-format off
      const auto
      [expression, expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
      {"~393",     bitNeg(lit(393))},
      {"~",        Error{"At line 1, column 2: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
      {"~~",       Error{"At line 1, column 3: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedResult);
    }
  }

  SECTION("Binary operators")
  {
    SECTION("Addition")
    {
      // clang-format off
      const auto
      [expression,     expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"2 + 3",        add(lit(2.0), lit(3.0))},
      {R"("as"+"df")", add(lit("as"), lit("df"))},
      {"2 + 3 + 4",    add(add(lit(2.0), lit(3.0)), lit(4.0))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Subtraction")
    {
      // clang-format off
      const auto
      [expression, expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"2-3.0",    sub(lit(2.0), lit(3.0))},
      {"2-3 -  4", sub(sub(lit(2.0), lit(3.0)), lit(4.0))},
      {"2-3-4-2",  sub(sub(sub(lit(2.0), lit(3.0)), lit(4.0)), lit(2.0))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Multiplication")
    {
      // clang-format off
      const auto
      [expression,  expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"2 * 3.0",   mul(lit(2.0), lit(3.0))},
      {"2 * 3 * 4", mul(mul(lit(2.0), lit(3.0)), lit(4.0))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Division")
    {
      // clang-format off
      const auto
      [expression,   expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"12 / 2.0",   div(lit(12.0), lit(2.0))},
      {"12 / 2 / 2", div(div(lit(12.0), lit(2.0)), lit(2.0))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Modulus")
    {
      // clang-format off
      const auto
      [expression,   expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"12 % 2.0",   mod(lit(12.0), lit(2.0))},
      {"12 % 5 % 3", mod(mod(lit(12.0), lit(5.0)), lit(3.0))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Logical and")
    {
      CHECK(parse("true && true") == logAnd(lit(true), lit(true)));
    }

    SECTION("Logical or")
    {
      CHECK(parse("true || true") == logOr(lit(true), lit(true)));
    }

    SECTION("Bitwise and")
    {
      CHECK(parse("23 & 24") == bitAnd(lit(23), lit(24)));
    }

    SECTION("Bitwise or")
    {
      CHECK(parse("23 | 24") == bitOr(lit(23), lit(24)));
    }

    SECTION("Bitwise xor")
    {
      // clang-format off
      const auto
      [expression, expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
      {"23 ^ 24",  bitXOr(lit(23), lit(24))},
      {"23 ^^ 23", Error{"At line 1, column 5: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got unknown token type (raw data: '^')"}},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedResult);
    }

    SECTION("Bitwise shift left")
    {
      CHECK(parse("1 << 7") == bitShL(lit(1), lit(7)));
    }

    SECTION("Bitwise shift right")
    {
      CHECK(parse("8 >> 2") == bitShR(lit(8), lit(2)));
    }

    SECTION("Case operator")
    {
      CHECK(parse("true -> 1") == cs(lit(true), lit(1)));
    }

    SECTION("Comparison operators")
    {
      // clang-format off
      const auto
      [expression, expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"1 < 2",    ls(lit(1), lit(2))},
      {"1 <= 2",   lsEq(lit(1), lit(2))},
      {"1 > 2",    gr(lit(1), lit(2))},
      {"1 >= 2",   grEq(lit(1), lit(2))},
      {"1 == 2",   eq(lit(1), lit(2))},
      {"1 != 2",   neq(lit(1), lit(2))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }

    SECTION("Operator combinations")
    {
      // clang-format off
      const auto
      [expression,                  expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
      {"1 + 2 * 3",                 add(lit(1), mul(lit(2), lit(3)))},
      {"1 * 2 + 3",                 add(mul(lit(1), lit(2)), lit(3))},
      {"1 + 2 * 3 + 2",             add(add(lit(1), mul(lit(2), lit(3))), lit(2))},
      {"1 + 2 * 3 + 2 * 2",         add(add(lit(1), mul(lit(2), lit(3))), mul(lit(2), lit(2)))},
      {"3 + 2 < 3 + 3",             ls(add(lit(3), lit(2)), add(lit(3), lit(3)))},
      {"3 + 2 < 3 + 3 + 0 && true", logAnd(ls(add(lit(3), lit(2)), add(add(lit(3), lit(3)), lit(0))), lit(true))},
      {"false && false || true",    logOr(logAnd(lit(false), lit(false)), lit(true))},
      {"false && (false || true)",  logAnd(lit(false), grp(logOr(lit(false), lit(true))))},
      }));
      // clang-format on

      CAPTURE(expression);

      CHECK(parse(expression) == expectedExpression);
    }
  }

  SECTION("Subscript")
  {
    // clang-format off
    const auto
    [expression,                                        expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
    {R"([ 1.0, 2.0, "test" ][0])",                      scr(arr({lit(1.0), lit(2.0), lit("test")}), lit(0))},
    {R"([ 1.0, 2.0, "test" ][1+1])",                    scr(arr({lit(1.0), lit(2.0), lit("test")}), add(lit(1), lit(1)))},
    {R"({ "key1":1, "key2":2, "key3":"test"}["key1"])", scr(map({{"key1", lit(1.0)}, {"key2", lit(2.0)}, {"key3", lit("test")}}), lit("key1"))},
    {R"([ 1.0, [ 2.0, "test"] ][1][0])",                scr(scr(arr({lit(1.0), arr({lit(2.0), lit("test")})}), lit(1)), lit(0))},
    {R"([ 1.0, 2.0, "test" ][0,1,2])",                  scr(arr({lit(1.0), lit(2.0), lit("test")}), arr({lit(0), lit(1), lit(2)}))},
    {R"([ 1.0, 2.0, "test" ][0..2])",                   scr(arr({lit(1.0), lit(2.0), lit("test")}), bRng(lit(0), lit(2)))},
    {R"([ 1.0, 2.0, "test" ][0..2,3])",                 scr(arr({lit(1.0), lit(2.0), lit("test")}), arr({bRng(lit(0), lit(2)), lit(3)}))},

    // empty subscript
    {R"([ 1.0 ][])",                                    scr(arr({lit(1.0)}), arr({}))},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedExpression);
  }

  SECTION("Dot access")
  {
    // clang-format off
    const auto
    [expression,        expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
    {"a.b",             dot(var("a"), "b")},
    {R"({ "b": 1 }.b)", dot(map({{"b", lit(1.0)}}), "b")},

    {"a.b.c",           dot(dot(var("a"), "b"), "c")},
    {"a.b[0]",          scr(dot(var("a"), "b"), lit(0))},
    {"a[0].b",          dot(scr(var("a"), lit(0)), "b")},

    {"1.5",             lit(1.5)},
    {"1.5 + a.b",       add(lit(1.5), dot(var("a"), "b"))},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedExpression);
  }

  SECTION("Function calls")
  {
    // clang-format off
    const auto
    [expression,   expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
    {"f()",        call("f")},
    {"f(1)",       call("f", {lit(1)})},
    {"f(1, 2, 3)", call("f", {lit(1), lit(2), lit(3)})},
    {"f(1 + 2)",   call("f", {add(lit(1), lit(2))})},

    // nested calls
    {"f(g())",     call("f", {call("g")})},
    {"f(g(1), 2)", call("f", {call("g", {lit(1)}), lit(2)})},

    // composes with dot-access and bracket subscript
    {"f().x",      dot(call("f"), "x")},
    {"f()[0]",     scr(call("f"), lit(0))},
    {"f(a.b)",     call("f", {dot(var("a"), "b")})},

    // whitespace between the name and '(' doesn't change anything
    {"f (1)",      call("f", {lit(1)})},

    // a Name NOT followed by '(' is still a plain variable, unaffected by any of this
    {"f",          var("f")},
    {"f + 1",      add(var("f"), lit(1))},

    // malformed call syntax is still a parse error
    {"f(",         Error{"At line 1, column 3: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
    {"f(1,)",      Error{"At line 1, column 5: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got ')' (raw data: ')')"}},
    {"f(,1)",      Error{"At line 1, column 3: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got ',' (raw data: ',')"}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedResult);
  }

  SECTION("Infix calls")
  {
    // clang-format off
    const auto
    [expression,                      expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
    {"a distanceTo b",                inf("distanceTo", var("a"), var("b"))},
    {"1 intersects 2",                inf("intersects", lit(1), lit(2))},

    // composes with prefix calls
    {"distanceTo(a, b) distanceTo c", inf("distanceTo", call("distanceTo", {var("a"), var("b")}), var("c"))},
    {"f(a intersects b)",             call("f", {inf("intersects", var("a"), var("b"))})},

    // chains left-associatively, like every other binary operator
    {"a intersects b intersects c",   inf("intersects", inf("intersects", var("a"), var("b")), var("c"))},

    // an infix call binds tighter than every real binary operator
    {"1 + 2 intersects 3 + 4",        add(add(lit(1), inf("intersects", lit(2), lit(3))), lit(4))},
    {"1 * 2 intersects 3 * 4",        mul(mul(lit(1), inf("intersects", lit(2), lit(3))), lit(4))},
    {"a && b intersects c && d",      logAnd(logAnd(var("a"), inf("intersects", var("b"), var("c"))), var("d"))},

    // even ==, so the result of the call is what gets compared
    {"a intersects b == c",           eq(inf("intersects", var("a"), var("b")), var("c"))},
    {"a == b intersects c == d",      eq(eq(var("a"), inf("intersects", var("b"), var("c"))), var("d"))},

    // a Name NOT preceded by a complete term is unaffected -- still a plain variable or
    // prefix call
    {"a",                             var("a")},
    {"f(a)",                          call("f", {var("a")})},

    // a Name that isn't a built-in function ends the term instead of being called, which
    // leaves trailing garbage in strict mode
    {"a f b",                         Error{"At line 1, column 3: Expected end of file, but got variable (raw data: 'f')"}},

    // so does a built-in function that doesn't take two arguments
    {"a vec b",                       Error{"At line 1, column 3: Expected end of file, but got variable (raw data: 'vec')"}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedResult);
  }

  SECTION("Infix calls in lenient mode")
  {
    // a Name that isn't a built-in function ends the term instead of being called
    CHECK(Parser{ParseMode::Lenient, "a f b"}.parse() == var("a"));
  }

  SECTION("Switch")
  {
    // clang-format off
    const auto
    [expression,                    expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
    {"{{}}",                        swt({})},
    {"{{'asdf'}}",                  swt({lit("asdf")})},
    {"{{'fdsa', 'asdf'}}",          swt({lit("fdsa"), lit("asdf")})},
    {"{{false -> 'fdsa', 'asdf'}}", swt({cs(lit(false), lit("fdsa")), lit("asdf")})},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedExpression);
  }

  SECTION("Comments")
  {
    // clang-format off
    const auto
    [expression,          expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
    {"1 // comment",      lit(1)},
    {"1 // comment\n",    lit(1)},
    {"// comment\n1",     lit(1)},
    {"1 + // comment\n2", add(lit(1), lit(2))},
    {"// comment",        Error{"At line 1, column 11: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},

    // a single slash is still division
    {"4 / 2",             div(lit(4), lit(2))},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedResult);
  }

  SECTION("Whitespace")
  {
    // clang-format off
    const auto
    [expression, expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
    {"\t1",      lit(1)},
    {"\r1",      lit(1)},
    {"1\t+\r2",  add(lit(1), lit(2))},
    {"\t\r\n ",  Error{"At line 2, column 2: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got end of file (raw data: '')"}},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedResult);
  }

  SECTION("Names")
  {
    // clang-format off
    const auto
    [expression,    expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
    {"_a",          var("_a")},
    {"a_b",         var("a_b")},
    {"a1",          var("a1")},
    {"_1",          var("_1")},
    {"a_1b",        var("a_1b")},

    // a name may begin with a keyword
    {"trueValue",   var("trueValue")},
    {"false_alarm", var("false_alarm")},
    {"nullable",    var("nullable")},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedExpression);
  }

  SECTION("Malformed input")
  {
    // clang-format off
    const auto
    [expression, expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
    // a number cannot be followed by another decimal point
    {"1.2.3",    Error{"At line 1, column 2: Unexpected character: '.'"}},

    {"$",        Error{"At line 1, column 1: Unexpected character: '$'"}},

    // a lone '=' is not an operator, and must not consume the character after it
    {"1 = 2",    Error{"At line 1, column 3: Unexpected character: '='"}},
    {"1 =2",     Error{"At line 1, column 3: Unexpected character: '='"}},
    {"1 =+ 2",   Error{"At line 1, column 3: Unexpected character: '='"}},
    {"1 =* 2",   Error{"At line 1, column 3: Unexpected character: '='"}},
    {"=",        Error{"At line 1, column 1: Unexpected character: '='"}},

    // but '==' still is, including when the right operand follows immediately
    {"1 == 2",   eq(lit(1), lit(2))},
    {"1 ==2",    eq(lit(1), lit(2))},
    {"a==b",     eq(var("a"), var("b"))},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedResult);
  }

  SECTION("Numbers need not be separated from the following operator")
  {
    // clang-format off
    const auto
    [expression,    expectedExpression] = GENERATE(table<std::string, ExpressionNode>({
    {"1+2",         add(lit(1), lit(2))},
    {"1-2",         sub(lit(1), lit(2))},
    {"1*2",         mul(lit(1), lit(2))},
    {"1/2",         div(lit(1), lit(2))},
    {"1%2",         mod(lit(1), lit(2))},
    {"1==2",        eq(lit(1), lit(2))},
    {"1!=2",        neq(lit(1), lit(2))},
    {"1<2",         ls(lit(1), lit(2))},
    {"1<=2",        lsEq(lit(1), lit(2))},
    {"1>2",         gr(lit(1), lit(2))},
    {"1>=2",        grEq(lit(1), lit(2))},
    {"1&2",         bitAnd(lit(1), lit(2))},
    {"1|2",         bitOr(lit(1), lit(2))},
    {"1^2",         bitXOr(lit(1), lit(2))},
    {"1<<2",        bitShL(lit(1), lit(2))},
    {"1>>2",        bitShR(lit(1), lit(2))},
    {"1&&2",        logAnd(lit(1), lit(2))},
    {"1||2",        logOr(lit(1), lit(2))},
    {"1->2",        cs(lit(1), lit(2))},

    // decimals and exponents are read before the delimiter is checked
    {"1.5<2",       ls(lit(1.5), lit(2))},
    {"1e2<2",       ls(lit(100.0), lit(2))},
    {"1e-2<2",      ls(lit(0.01), lit(2))},

    // a range in a subscript still ends the integer before the dots
    {"[1,2][0..1]", scr(arr({lit(1), lit(2)}), bRng(lit(0), lit(1)))},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedExpression);
  }

  SECTION("tokenizerState")
  {
    // in lenient mode, parsing stops at the first token that cannot continue the
    // expression, and the state says where the caller should resume
    auto parser = Parser{ParseMode::Lenient, "1 + 2 ) rest"};
    REQUIRE(parser.parse() == add(lit(1), lit(2)));

    // the state points just past the last consumed token, not past the whitespace
    const auto state = parser.tokenizerState();
    CHECK(state.line == 1u);
    CHECK(state.column == 6u);
    CHECK(std::string{state.cur} == " ) rest");
  }

  SECTION("Groups")
  {
    // clang-format off
    const auto
    [expression,         expectedResult] = GENERATE(table<std::string, Result<ExpressionNode>>({
    {"()",               Error{"At line 1, column 2: Expected variable, string, number, boolean, '[', '{', '(', '+', '-', '!', '~', '{{', 'null', or 'undefined', but got ')' (raw data: ')')"}},
    {"(1)",              grp(lit(1))},
    {"(2+1)*3",          mul(grp(add(lit(2), lit(1))), lit(3))},
    {"(2+1)*(2+1)",      mul(grp(add(lit(2), lit(1))), grp(add(lit(2), lit(1))))},
    {"(2+1)*((1+1)*2)",  mul(grp(add(lit(2), lit(1))), grp(mul(grp(add(lit(1), lit(1))), lit(2))))},

    // a unary operator applies only to the group, not to the term that follows it
    {"!(true) && false", logAnd(logNeg(grp(lit(true))), lit(false))},
    {"-(1) + 2",         add(minus(grp(lit(1))), lit(2))},
    }));
    // clang-format on

    CAPTURE(expression);

    CHECK(parse(expression) == expectedResult);
  }
}

} // namespace tb::el
