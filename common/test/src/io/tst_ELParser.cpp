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

#include "io/ELParser.h"
#include "el/ELExceptions.h"
#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/Value.h"

#include <string>

#include "Catch2.h"

namespace tb::io
{

static auto evaluate(
  const std::string& str, const el::EvaluationContext& context = el::EvaluationContext{})
{
  return ELParser::parseStrict(str).evaluate(context);
}

TEST_CASE("ELParserTest.parseEmptyExpression")
{
  CHECK_THROWS_AS(evaluate(""), ParserException);
  CHECK_THROWS_AS(evaluate("    "), ParserException);
  CHECK_THROWS_AS(evaluate("\n"), ParserException);
}

TEST_CASE("ELParserTest.parseStringLiteral")
{
  CHECK_THROWS_AS(evaluate(R"("asdf)"), ParserException);

  const auto context = el::EvaluationContext{};
  CHECK(evaluate(R"("asdf")") == el::Value{"asdf"});
}

TEST_CASE("ELParserTest.parseStringLiteralWithDoubleQuotationMarks")
{
  // MSVC complains about an illegal escape sequence if we use a raw string literal for
  // the expression
  CHECK(evaluate("\"asdf\\\" \\\"asdf\"") == el::Value(R"(asdf" "asdf)"));
}

TEST_CASE("ELParserTest.parseNumberLiteral")
{
  CHECK_THROWS_AS(evaluate("1.123.34"), ParserException);

  CHECK(evaluate("1") == el::Value{1.0});
  CHECK(evaluate("1.0") == el::Value{1.0});
  CHECK(evaluate("01.00") == el::Value{1.0});
  CHECK(evaluate(".0") == el::Value{0.0});
  CHECK(evaluate("0") == el::Value{0.0});
}

TEST_CASE("ELParserTest.parseBooleanLiteral")
{
  CHECK(evaluate("true") == el::Value{true});
  CHECK(evaluate("false") == el::Value{false});
}

TEST_CASE("ELParserTest.parseArrayLiteral")
{
  CHECK(evaluate("[]") == el::Value{el::ArrayType{}});
  CHECK(
    evaluate(R"([ 1.0 , "test",[ true] ])")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
      el::Value{"test"},
      el::Value{el::ArrayType{{
        el::Value{true},
      }}},
    }}});

  CHECK(
    evaluate("[1..3]")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
      el::Value{2.0},
      el::Value{3.0},
    }}});
  CHECK(
    evaluate("[3..1]")
    == el::Value{el::ArrayType{{
      el::Value{3.0},
      el::Value{2.0},
      el::Value{1.0},
    }}});
  CHECK(
    evaluate("[1..1]")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
    }}});
  CHECK(
    evaluate("[1..0]")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
      el::Value{0.0},
    }}});
  CHECK(
    evaluate("[-2..1]")
    == el::Value{el::ArrayType{{
      el::Value{-2.0},
      el::Value{-1.0},
      el::Value{0.0},
      el::Value{1.0},
    }}});
}

TEST_CASE("ELParserTest.parseMapLiteral")
{
  CHECK(evaluate("{}") == el::Value{el::MapType{}});
  CHECK(
    evaluate(R"( { "testkey1": 1, "testkey2"   :"asdf", "testkey3":{"nestedKey":true} })")
    == el::Value{el::MapType{
      {"testkey1", el::Value{1.0}},
      {"testkey2", el::Value{"asdf"}},
      {"testkey3",
       el::Value{el::MapType{
         {"nestedKey", el::Value{true}},
       }}},
    }});
}

TEST_CASE("ELParserTest.parseMapLiteralNestedInArray")
{
  CHECK(
    evaluate(R"([ { "key": "value" } ])")
    == el::Value{el::ArrayType{{
      el::Value{el::MapType{
        {"key", el::Value{"value"}},
      }},
    }}});
}

TEST_CASE("ELParserTest.parseMapLiteralNestedInArrayNestedInMap")
{
  CHECK(
    evaluate(R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })")
    == el::Value{el::MapType{
      {"outerkey1",
       el::Value{el::ArrayType{{
         el::Value{el::MapType{
           {"key", el::Value{"value"}},
         }},
       }}}},
      {"outerkey2", el::Value{"asdf"}},
    }});
}

TEST_CASE("ELParserTest.parseMapLiteralWithTrailingGarbage")
{
  CHECK_THROWS_AS(
    evaluate(R"({
	"profiles": [],
	"version": 1
}
asdf)"),
    ParserException);
}

TEST_CASE("ELParserTest.parseVariable")
{
  auto context = el::EvaluationContext{};
  context.declareVariable("test", el::Value{1.0});

  CHECK(evaluate("test", context) == el::Value{1.0});
}

TEST_CASE("ELParserTest.parseUnaryPlus")
{
  CHECK(evaluate("+1.0") == el::Value{1.0});
}

TEST_CASE("ELParserTest.parseUnaryMinus")
{
  CHECK(evaluate("-1.0") == el::Value{-1.0});
}

TEST_CASE("ELParserTest.parseLogicalNegation")
{
  CHECK(evaluate("!true") == el::Value{false});
  CHECK(evaluate("!false") == el::Value{true});
  CHECK_THROWS_AS(evaluate("!0"), el::EvaluationError);
  CHECK_THROWS_AS(evaluate("!1"), el::EvaluationError);
  CHECK_THROWS_AS(evaluate("!'true'"), el::EvaluationError);
}

TEST_CASE("ELParserTest.parseBitwiseNegation")
{
  CHECK(evaluate("~393") == el::Value{~393});
  CHECK_THROWS_AS(evaluate("~"), ParserException);
  CHECK_THROWS_AS(evaluate("~~"), ParserException);
}

TEST_CASE("ELParserTest.parseAddition")
{
  CHECK(evaluate("2 + 3") == el::Value{5.0});
  CHECK(evaluate("\"as\"+\"df\"") == el::Value{"asdf"});
  CHECK(evaluate("2 + 3 + 4") == el::Value{9.0});
}

TEST_CASE("ELParserTest.parseSubtraction")
{
  CHECK(evaluate("2 - 3.0") == el::Value{-1.0});
  CHECK(evaluate("2 - 3 - 4") == el::Value{-5.0});
  CHECK(evaluate("2 - 3 - 4 - 2") == el::Value{-7.0});
}

TEST_CASE("ELParserTest.parseMultiplication")
{
  CHECK(evaluate("2 * 3.0") == el::Value{6.0});

  CHECK(evaluate("2 * 3 * 4") == el::Value{24.0});
  CHECK(evaluate("2 * 3 * 4 * 2") == el::Value{48.0});
}

TEST_CASE("ELParserTest.parseDivision")
{
  CHECK(evaluate("12 / 2.0") == el::Value{6.0});
  CHECK(evaluate("12 / 2 / 2") == el::Value{3.0});
  CHECK(evaluate("12 / 2 / 2 / 3") == el::Value{1.0});
}

TEST_CASE("ELParserTest.parseModulus")
{
  CHECK(evaluate("12 % 2.0") == el::Value{0.0});
  CHECK(evaluate("12 % 5 % 3") == el::Value{2.0});
  CHECK(evaluate("12 % 5 % 3 % 3") == el::Value{2.0});
}

TEST_CASE("ELParserTest.parseLogicalAnd")
{
  CHECK(evaluate("true && true") == el::Value{true});
  CHECK(evaluate("false && true") == el::Value{false});
  CHECK(evaluate("true && false") == el::Value{false});
  CHECK(evaluate("false && false") == el::Value{false});
}

TEST_CASE("ELParserTest.parseLogicalOr")
{
  CHECK(evaluate("true || true") == el::Value{true});
  CHECK(evaluate("false || true") == el::Value{true});
  CHECK(evaluate("true || false") == el::Value{true});
  CHECK(evaluate("false || false") == el::Value{false});
}

TEST_CASE("ELParserTest.parseBitwiseAnd")
{
  CHECK(evaluate("23 & 24") == el::Value{23 & 24});
}

TEST_CASE("ELParserTest.parseBitwiseOr")
{
  CHECK(evaluate("23 | 24") == el::Value{23 | 24});
}

TEST_CASE("ELParserTest.parseBitwiseXor")
{
  CHECK(evaluate("23 ^ 24") == el::Value{(23 ^ 24)});
  CHECK_THROWS_AS(evaluate("23 ^^ 23"), ParserException);
}

TEST_CASE("ELParserTest.parseBitwiseShiftLeft")
{
  CHECK(evaluate("1 << 7") == el::Value{1 << 7});
}

TEST_CASE("ELParserTest.parseBitwiseShiftRight")
{
  CHECK(evaluate("8 >> 2") == el::Value{8 >> 2});
}

TEST_CASE("ELParserTest.parseSubscript")
{
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][0])") == el::Value{1.0});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][1])") == el::Value{2.0});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][2])") == el::Value{"test"});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-1])") == el::Value{"test"});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-2])") == el::Value{2.0});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-3])") == el::Value{1.0});

  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][1 + 1])") == el::Value{"test"});

  CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key1"])") == el::Value{1.0});
  CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key2"])") == el::Value{2.0});
  CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key3"])") == el::Value{"test"});

  CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][0])") == el::Value{1.0});
  CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][1][0])") == el::Value{2.0});
  CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][1][1])") == el::Value{"test"});

  CHECK(
    evaluate(R"({ "key1":1, "key2":2, "key3":[ 1, 2]}["key3"][1])") == el::Value{2.0});

  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][0,1,2])")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
      el::Value{2.0},
      el::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][0..2])")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
      el::Value{2.0},
      el::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][2..0])")
    == el::Value{el::ArrayType{{
      el::Value{"test"},
      el::Value{2.0},
      el::Value{1.0},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][0,1..2])")
    == el::Value{el::ArrayType{{
      el::Value{1.0},
      el::Value{2.0},
      el::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][1..])")
    == el::Value{el::ArrayType{{
      el::Value{2.0},
      el::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][..1])")
    == el::Value{el::ArrayType{{
      el::Value{"test"},
      el::Value{2.0},
    }}});

  CHECK(evaluate(R"("test"[3,2,1,0])") == el::Value{"tset"});
  CHECK(evaluate(R"("test"[2,1,0])") == el::Value{"set"});
  CHECK(evaluate(R"("test"[2..1])") == el::Value{"se"});

  CHECK(evaluate(R"("test"[..0])") == el::Value{"tset"});
  CHECK(evaluate(R"("test"[1..])") == el::Value{"est"});
}

TEST_CASE("ELParserTest.parseCaseOperator")
{
  CHECK(evaluate("true -> false") == el::Value{false});
  CHECK(evaluate("true -> true && true") == el::Value{true});
  CHECK(evaluate("1 < 3 -> 2 + 3") == el::Value{5});
  CHECK(evaluate("false -> true") == el::Value::Undefined);
}

TEST_CASE("ELParserTest.parseBinaryNegation")
{
  CHECK(evaluate("~1") == el::Value{~1l});
}

TEST_CASE("ELParserTest.parseSwitchExpression")
{
  CHECK(evaluate("{{}}") == el::Value::Undefined);
  CHECK(evaluate("{{'asdf'}}") == el::Value{"asdf"});
  CHECK(evaluate("{{'fdsa', 'asdf'}}") == el::Value{"fdsa"});
  CHECK(evaluate("{{false -> 'fdsa', 'asdf'}}") == el::Value{"asdf"});
  CHECK(evaluate("{{false -> false}}") == el::Value::Undefined);
}

TEST_CASE("ELParserTest.testComparisonOperators")
{
  CHECK(evaluate(R"(1 < 2)") == el::Value{true});
  CHECK(evaluate(R"(2 < 2)") == el::Value{false});
  CHECK(evaluate(R"(1 <= 2)") == el::Value{true});
  CHECK(evaluate(R"(2 <= 2)") == el::Value{true});
  CHECK(evaluate(R"(3 <= 2)") == el::Value{false});

  CHECK(evaluate(R"("test" == "test")") == el::Value{true});
  CHECK(evaluate(R"("test1" == "test")") == el::Value{false});
  CHECK(evaluate(R"("test" != "test")") == el::Value{false});
  CHECK(evaluate(R"("test1" != "test")") == el::Value{true});

  CHECK(evaluate(R"(2 > 1)") == el::Value{true});
  CHECK(evaluate(R"(2 > 2)") == el::Value{false});
  CHECK(evaluate(R"(2 >= 1)") == el::Value{true});
  CHECK(evaluate(R"(2 >= 2)") == el::Value{true});
  CHECK(evaluate(R"(2 >= 3)") == el::Value{false});
}

TEST_CASE("ELParserTest.testOperatorPrecedence")
{
  CHECK(evaluate("7 + 2 * 3") == evaluate("2 * 3 + 7"));
  CHECK(evaluate("7 + 2 * 3 + 2") == evaluate("2 * 3 + 7 + 2"));
  CHECK(evaluate("7 + 2 * 3 + 2 * 2") == evaluate("2 * 3 + 7 + 2 * 2"));
  CHECK(evaluate("7 + 2 / 3 + 2 * 2") == evaluate("2 / 3 + 7 + 2 * 2"));

  CHECK(evaluate("3 + 2 < 3 + 3") == evaluate("(3 + 2) < (3 + 3)"));
  CHECK(
    evaluate("3 + 2 < 3 + 3 + 0 && true") == evaluate("((3 + 2) < (3 + 3 + 0)) && true"));
  CHECK(evaluate("false && false || true") == el::Value{true});
  CHECK(evaluate("false && (false || true)") == el::Value{false});
}

TEST_CASE("ELParserTest.testParseGrouping")
{
  CHECK_THROWS_AS(evaluate("()"), ParserException);
  CHECK(evaluate("(1)") == el::Value{1.0});
  CHECK(evaluate("(2+1)*3") == el::Value{9.0});
  CHECK(evaluate("(2+1)*(2+1)") == el::Value{9.0});
  CHECK(evaluate("(2+1)*((1+1)*2)") == el::Value{12.0});
}

} // namespace tb::io
