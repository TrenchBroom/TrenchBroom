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

#include "EL/ELExceptions.h"
#include "EL/EvaluationContext.h"
#include "EL/Expression.h"
#include "EL/Value.h"
#include "IO/ELParser.h"

#include <limits>
#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{

static auto evaluate(
  const std::string& str, const EL::EvaluationContext& context = EL::EvaluationContext{})
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

  const auto context = EL::EvaluationContext{};
  CHECK(evaluate(R"("asdf")") == EL::Value{"asdf"});
}

TEST_CASE("ELParserTest.parseStringLiteralWithDoubleQuotationMarks")
{
  // MSVC complains about an illegal escape sequence if we use a raw string literal for
  // the expression
  CHECK(evaluate("\"asdf\\\" \\\"asdf\"") == EL::Value(R"(asdf" "asdf)"));
}

TEST_CASE("ELParserTest.parseNumberLiteral")
{
  CHECK_THROWS_AS(evaluate("1.123.34"), ParserException);

  CHECK(evaluate("1") == EL::Value{1.0});
  CHECK(evaluate("1.0") == EL::Value{1.0});
  CHECK(evaluate("01.00") == EL::Value{1.0});
  CHECK(evaluate(".0") == EL::Value{0.0});
  CHECK(evaluate("0") == EL::Value{0.0});
}

TEST_CASE("ELParserTest.parseBooleanLiteral")
{
  CHECK(evaluate("true") == EL::Value{true});
  CHECK(evaluate("false") == EL::Value{false});
}

TEST_CASE("ELParserTest.parseArrayLiteral")
{
  CHECK(evaluate("[]") == EL::Value{EL::ArrayType{}});
  CHECK(
    evaluate(R"([ 1.0 , "test",[ true] ])")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
      EL::Value{"test"},
      EL::Value{EL::ArrayType{{
        EL::Value{true},
      }}},
    }}});

  CHECK(
    evaluate("[1..3]")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
      EL::Value{2.0},
      EL::Value{3.0},
    }}});
  CHECK(
    evaluate("[3..1]")
    == EL::Value{EL::ArrayType{{
      EL::Value{3.0},
      EL::Value{2.0},
      EL::Value{1.0},
    }}});
  CHECK(
    evaluate("[1..1]")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
    }}});
  CHECK(
    evaluate("[1..0]")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
      EL::Value{0.0},
    }}});
  CHECK(
    evaluate("[-2..1]")
    == EL::Value{EL::ArrayType{{
      EL::Value{-2.0},
      EL::Value{-1.0},
      EL::Value{0.0},
      EL::Value{1.0},
    }}});
}

TEST_CASE("ELParserTest.parseMapLiteral")
{
  CHECK(evaluate("{}") == EL::Value{EL::MapType{}});
  CHECK(
    evaluate(R"( { "testkey1": 1, "testkey2"   :"asdf", "testkey3":{"nestedKey":true} })")
    == EL::Value{EL::MapType{
      {"testkey1", EL::Value{1.0}},
      {"testkey2", EL::Value{"asdf"}},
      {"testkey3",
       EL::Value{EL::MapType{
         {"nestedKey", EL::Value{true}},
       }}},
    }});
}

TEST_CASE("ELParserTest.parseMapLiteralNestedInArray")
{
  CHECK(
    evaluate(R"([ { "key": "value" } ])")
    == EL::Value{EL::ArrayType{{
      EL::Value{EL::MapType{
        {"key", EL::Value{"value"}},
      }},
    }}});
}

TEST_CASE("ELParserTest.parseMapLiteralNestedInArrayNestedInMap")
{
  CHECK(
    evaluate(R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })")
    == EL::Value{EL::MapType{
      {"outerkey1",
       EL::Value{EL::ArrayType{{
         EL::Value{EL::MapType{
           {"key", EL::Value{"value"}},
         }},
       }}}},
      {"outerkey2", EL::Value{"asdf"}},
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
  auto context = EL::EvaluationContext{};
  context.declareVariable("test", EL::Value{1.0});

  CHECK(evaluate("test", context) == EL::Value{1.0});
}

TEST_CASE("ELParserTest.parseUnaryPlus")
{
  CHECK(evaluate("+1.0") == EL::Value{1.0});
}

TEST_CASE("ELParserTest.parseUnaryMinus")
{
  CHECK(evaluate("-1.0") == EL::Value{-1.0});
}

TEST_CASE("ELParserTest.parseLogicalNegation")
{
  CHECK(evaluate("!true") == EL::Value{false});
  CHECK(evaluate("!false") == EL::Value{true});
  CHECK_THROWS_AS(evaluate("!0"), EL::EvaluationError);
  CHECK_THROWS_AS(evaluate("!1"), EL::EvaluationError);
  CHECK_THROWS_AS(evaluate("!'true'"), EL::EvaluationError);
}

TEST_CASE("ELParserTest.parseBitwiseNegation")
{
  CHECK(evaluate("~393") == EL::Value{~393});
  CHECK_THROWS_AS(evaluate("~"), ParserException);
  CHECK_THROWS_AS(evaluate("~~"), ParserException);
}

TEST_CASE("ELParserTest.parseAddition")
{
  CHECK(evaluate("2 + 3") == EL::Value{5.0});
  CHECK(evaluate("\"as\"+\"df\"") == EL::Value{"asdf"});
  CHECK(evaluate("2 + 3 + 4") == EL::Value{9.0});
}

TEST_CASE("ELParserTest.parseSubtraction")
{
  CHECK(evaluate("2 - 3.0") == EL::Value{-1.0});
  CHECK(evaluate("2 - 3 - 4") == EL::Value{-5.0});
  CHECK(evaluate("2 - 3 - 4 - 2") == EL::Value{-7.0});
}

TEST_CASE("ELParserTest.parseMultiplication")
{
  CHECK(evaluate("2 * 3.0") == EL::Value{6.0});

  CHECK(evaluate("2 * 3 * 4") == EL::Value{24.0});
  CHECK(evaluate("2 * 3 * 4 * 2") == EL::Value{48.0});
}

TEST_CASE("ELParserTest.parseDivision")
{
  CHECK(evaluate("12 / 2.0") == EL::Value{6.0});
  CHECK(evaluate("12 / 2 / 2") == EL::Value{3.0});
  CHECK(evaluate("12 / 2 / 2 / 3") == EL::Value{1.0});
}

TEST_CASE("ELParserTest.parseModulus")
{
  CHECK(evaluate("12 % 2.0") == EL::Value{0.0});
  CHECK(evaluate("12 % 5 % 3") == EL::Value{2.0});
  CHECK(evaluate("12 % 5 % 3 % 3") == EL::Value{2.0});
}

TEST_CASE("ELParserTest.parseLogicalAnd")
{
  CHECK(evaluate("true && true") == EL::Value{true});
  CHECK(evaluate("false && true") == EL::Value{false});
  CHECK(evaluate("true && false") == EL::Value{false});
  CHECK(evaluate("false && false") == EL::Value{false});
}

TEST_CASE("ELParserTest.parseLogicalOr")
{
  CHECK(evaluate("true || true") == EL::Value{true});
  CHECK(evaluate("false || true") == EL::Value{true});
  CHECK(evaluate("true || false") == EL::Value{true});
  CHECK(evaluate("false || false") == EL::Value{false});
}

TEST_CASE("ELParserTest.parseBitwiseAnd")
{
  CHECK(evaluate("23 & 24") == EL::Value{23 & 24});
}

TEST_CASE("ELParserTest.parseBitwiseOr")
{
  CHECK(evaluate("23 | 24") == EL::Value{23 | 24});
}

TEST_CASE("ELParserTest.parseBitwiseXor")
{
  CHECK(evaluate("23 ^ 24") == EL::Value{(23 ^ 24)});
  CHECK_THROWS_AS(evaluate("23 ^^ 23"), ParserException);
}

TEST_CASE("ELParserTest.parseBitwiseShiftLeft")
{
  CHECK(evaluate("1 << 7") == EL::Value{1 << 7});
}

TEST_CASE("ELParserTest.parseBitwiseShiftRight")
{
  CHECK(evaluate("8 >> 2") == EL::Value{8 >> 2});
}

TEST_CASE("ELParserTest.parseSubscript")
{
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][0])") == EL::Value{1.0});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][1])") == EL::Value{2.0});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][2])") == EL::Value{"test"});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-1])") == EL::Value{"test"});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-2])") == EL::Value{2.0});
  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-3])") == EL::Value{1.0});

  CHECK(evaluate(R"([ 1.0, 2.0, "test" ][1 + 1])") == EL::Value{"test"});

  CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key1"])") == EL::Value{1.0});
  CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key2"])") == EL::Value{2.0});
  CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key3"])") == EL::Value{"test"});

  CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][0])") == EL::Value{1.0});
  CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][1][0])") == EL::Value{2.0});
  CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][1][1])") == EL::Value{"test"});

  CHECK(
    evaluate(R"({ "key1":1, "key2":2, "key3":[ 1, 2]}["key3"][1])") == EL::Value{2.0});

  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][0,1,2])")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
      EL::Value{2.0},
      EL::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][0..2])")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
      EL::Value{2.0},
      EL::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][2..0])")
    == EL::Value{EL::ArrayType{{
      EL::Value{"test"},
      EL::Value{2.0},
      EL::Value{1.0},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][0,1..2])")
    == EL::Value{EL::ArrayType{{
      EL::Value{1.0},
      EL::Value{2.0},
      EL::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][1..])")
    == EL::Value{EL::ArrayType{{
      EL::Value{2.0},
      EL::Value{"test"},
    }}});
  CHECK(
    evaluate(R"([ 1.0, 2.0, "test" ][..1])")
    == EL::Value{EL::ArrayType{{
      EL::Value{"test"},
      EL::Value{2.0},
    }}});

  CHECK(evaluate(R"("test"[3,2,1,0])") == EL::Value{"tset"});
  CHECK(evaluate(R"("test"[2,1,0])") == EL::Value{"set"});
  CHECK(evaluate(R"("test"[2..1])") == EL::Value{"se"});

  CHECK(evaluate(R"("test"[..0])") == EL::Value{"tset"});
  CHECK(evaluate(R"("test"[1..])") == EL::Value{"est"});
}

TEST_CASE("ELParserTest.parseCaseOperator")
{
  CHECK(evaluate("true -> false") == EL::Value{false});
  CHECK(evaluate("true -> true && true") == EL::Value{true});
  CHECK(evaluate("1 < 3 -> 2 + 3") == EL::Value{5});
  CHECK(evaluate("false -> true") == EL::Value::Undefined);
}

TEST_CASE("ELParserTest.parseBinaryNegation")
{
  CHECK(evaluate("~1") == EL::Value{~1l});
}

TEST_CASE("ELParserTest.parseSwitchExpression")
{
  CHECK(evaluate("{{}}") == EL::Value::Undefined);
  CHECK(evaluate("{{'asdf'}}") == EL::Value{"asdf"});
  CHECK(evaluate("{{'fdsa', 'asdf'}}") == EL::Value{"fdsa"});
  CHECK(evaluate("{{false -> 'fdsa', 'asdf'}}") == EL::Value{"asdf"});
  CHECK(evaluate("{{false -> false}}") == EL::Value::Undefined);
}

TEST_CASE("ELParserTest.testComparisonOperators")
{
  CHECK(evaluate(R"(1 < 2)") == EL::Value{true});
  CHECK(evaluate(R"(2 < 2)") == EL::Value{false});
  CHECK(evaluate(R"(1 <= 2)") == EL::Value{true});
  CHECK(evaluate(R"(2 <= 2)") == EL::Value{true});
  CHECK(evaluate(R"(3 <= 2)") == EL::Value{false});

  CHECK(evaluate(R"("test" == "test")") == EL::Value{true});
  CHECK(evaluate(R"("test1" == "test")") == EL::Value{false});
  CHECK(evaluate(R"("test" != "test")") == EL::Value{false});
  CHECK(evaluate(R"("test1" != "test")") == EL::Value{true});

  CHECK(evaluate(R"(2 > 1)") == EL::Value{true});
  CHECK(evaluate(R"(2 > 2)") == EL::Value{false});
  CHECK(evaluate(R"(2 >= 1)") == EL::Value{true});
  CHECK(evaluate(R"(2 >= 2)") == EL::Value{true});
  CHECK(evaluate(R"(2 >= 3)") == EL::Value{false});
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
  CHECK(evaluate("false && false || true") == EL::Value{true});
  CHECK(evaluate("false && (false || true)") == EL::Value{false});
}

TEST_CASE("ELParserTest.testParseGrouping")
{
  CHECK_THROWS_AS(evaluate("()"), ParserException);
  CHECK(evaluate("(1)") == EL::Value{1.0});
  CHECK(evaluate("(2+1)*3") == EL::Value{9.0});
  CHECK(evaluate("(2+1)*(2+1)") == EL::Value{9.0});
  CHECK(evaluate("(2+1)*((1+1)*2)") == EL::Value{12.0});
}

} // namespace TrenchBroom::IO
