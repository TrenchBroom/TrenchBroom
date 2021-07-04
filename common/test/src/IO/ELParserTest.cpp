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

namespace TrenchBroom {
    namespace IO {
        static auto evaluate(const std::string& str, const EL::EvaluationContext& context = EL::EvaluationContext()) {
            return ELParser::parseStrict(str).evaluate(context);
        }

        TEST_CASE("ELParserTest.parseEmptyExpression", "[ELParserTest]") {
            CHECK_THROWS_AS(evaluate(""), ParserException);
            CHECK_THROWS_AS(evaluate("    "), ParserException);
            CHECK_THROWS_AS(evaluate("\n"), ParserException);
        }

        TEST_CASE("ELParserTest.parseStringLiteral", "[ELParserTest]") {
            CHECK_THROWS_AS(evaluate("\"asdf"), ParserException);

            const auto context = EL::EvaluationContext();
            CHECK(evaluate("\"asdf\"") == EL::Value("asdf"));
        }

        TEST_CASE("ELParserTest.parseStringLiteralWithDoubleQuotationMarks", "[ELParserTest]") {
            // MSVC complains about an illegal escape sequence if we use a raw string literal for the expression
            CHECK(evaluate("\"asdf\\\" \\\"asdf\"") == EL::Value(R"(asdf" "asdf)"));
        }

        TEST_CASE("ELParserTest.parseNumberLiteral", "[ELParserTest]") {
            CHECK_THROWS_AS(evaluate("1.123.34"), ParserException);

            CHECK(evaluate("1") == EL::Value(1.0));
            CHECK(evaluate("1.0") == EL::Value(1.0));
            CHECK(evaluate("01.00") == EL::Value(1.0));
            CHECK(evaluate(".0") == EL::Value(0.0));
            CHECK(evaluate("0") == EL::Value(0.0));
        }

        TEST_CASE("ELParserTest.parseBooleanLiteral", "[ELParserTest]") {
            CHECK(evaluate("true") == EL::Value(true));
            CHECK(evaluate("false") == EL::Value(false));
        }

        TEST_CASE("ELParserTest.parseArrayLiteral", "[ELParserTest]") {
            EL::ArrayType array, nestedArray;
            array.push_back(EL::Value(1.0));
            array.push_back(EL::Value("test"));
            nestedArray.push_back(EL::Value(true));
            array.push_back(EL::Value(nestedArray));

            CHECK(evaluate("[]") == EL::Value(EL::ArrayType()));
            CHECK(evaluate("[ 1.0 , \"test\",[ true] ]") == EL::Value(array));

            CHECK(evaluate("[1..3]") == EL::Value(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value(3.0) })));
            CHECK(evaluate("[3..1]") == EL::Value(EL::ArrayType({ EL::Value(3.0), EL::Value(2.0), EL::Value(1.0) })));
            CHECK(evaluate("[1..1]") == EL::Value(EL::ArrayType({ EL::Value(1.0) })));
            CHECK(evaluate("[1..0]") == EL::Value(EL::ArrayType({ EL::Value(1.0), EL::Value(0.0) })));
            CHECK(evaluate("[-2..1]") == EL::Value(EL::ArrayType({ EL::Value(-2.0), EL::Value(-1.0), EL::Value(0.0), EL::Value(1.0) })));
        }

        TEST_CASE("ELParserTest.parseMapLiteral", "[ELParserTest]") {
            EL::MapType map, nestedMap;
            map.insert(std::make_pair("testkey1", EL::Value(1.0)));
            map.insert(std::make_pair("testkey2", EL::Value("asdf")));
            nestedMap.insert(std::make_pair("nestedKey", EL::Value(true)));
            map.insert(std::make_pair("testkey3", EL::Value(nestedMap)));

            CHECK(evaluate("{}") == EL::Value(EL::MapType()));
            CHECK(evaluate(" { \"testkey1\": 1, \"testkey2\"   :\"asdf\", \"testkey3\":{\"nestedKey\":true} }") == EL::Value(map));
        }

        TEST_CASE("ELParserTest.parseMapLiteralNestedInArray", "[ELParserTest]") {
            EL::MapType map;
            map.insert(std::make_pair("key", EL::Value("value")));

            EL::ArrayType array;
            array.push_back(EL::Value(map));

            CHECK(evaluate(R"([ { "key": "value" } ])") == EL::Value(array));
        }

        TEST_CASE("ELParserTest.parseMapLiteralNestedInArrayNestedInMap", "[ELParserTest]") {
            EL::MapType inner;
            inner.insert(std::make_pair("key", EL::Value("value")));

            EL::ArrayType array;
            array.push_back(EL::Value(inner));

            EL::MapType outer;
            outer.insert(std::make_pair("outerkey1", EL::Value(array)));
            outer.insert(std::make_pair("outerkey2", EL::Value("asdf")));

            CHECK(evaluate(R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })") == EL::Value(outer));
        }

        TEST_CASE("ELParserTest.parseMapLiteralWithTrailingGarbage", "[ELParserTest]") {
            CHECK_THROWS_AS(evaluate("{\n"
                                     "\t\"profiles\": [],\n"
                                     "\t\"version\": 1\n"
                                     "}\n"
                                     "asdf"), ParserException);
        }

        TEST_CASE("ELParserTest.parseVariable", "[ELParserTest]") {
            EL::EvaluationContext context;
            context.declareVariable("test", EL::Value(1.0));

            CHECK(evaluate("test", context) == EL::Value(1.0));
        }

        TEST_CASE("ELParserTest.parseUnaryPlus", "[ELParserTest]") {
            CHECK(evaluate("+1.0") == EL::Value(1.0));
        }

        TEST_CASE("ELParserTest.parseUnaryMinus", "[ELParserTest]") {
            CHECK(evaluate("-1.0") == EL::Value(-1.0));
        }

        TEST_CASE("ELParserTest.parseLogicalNegation", "[ELParserTest]") {
            CHECK(evaluate("!true") == EL::Value(false));
            CHECK(evaluate("!false") == EL::Value(true));
            CHECK_THROWS_AS(evaluate("!0"), EL::EvaluationError);
            CHECK_THROWS_AS(evaluate("!1"), EL::EvaluationError);
            CHECK_THROWS_AS(evaluate("!'true'"), EL::EvaluationError);
        }

        TEST_CASE("ELParserTest.parseBitwiseNegation", "[ELParserTest]") {
            CHECK(evaluate("~393") == EL::Value(~393));
            CHECK_THROWS_AS(evaluate("~"), ParserException);
            CHECK_THROWS_AS(evaluate("~~"), ParserException);
        }

        TEST_CASE("ELParserTest.parseAddition", "[ELParserTest]") {
            CHECK(evaluate("2 + 3") == EL::Value(5.0));
            CHECK(evaluate("\"as\"+\"df\"") == EL::Value("asdf"));
            CHECK(evaluate("2 + 3 + 4") == EL::Value(9.0));
        }

        TEST_CASE("ELParserTest.parseSubtraction", "[ELParserTest]") {
            CHECK(evaluate("2 - 3.0") == EL::Value(-1.0));
            CHECK(evaluate("2 - 3 - 4") == EL::Value(-5.0));
            CHECK(evaluate("2 - 3 - 4 - 2") == EL::Value(-7.0));
        }

        TEST_CASE("ELParserTest.parseMultiplication", "[ELParserTest]") {
            CHECK(evaluate("2 * 3.0") == EL::Value(6.0));

            CHECK(evaluate("2 * 3 * 4") == EL::Value(24.0));
            CHECK(evaluate("2 * 3 * 4 * 2") == EL::Value(48.0));
        }

        TEST_CASE("ELParserTest.parseDivision", "[ELParserTest]") {
            CHECK(evaluate("12 / 2.0") == EL::Value(6.0));
            CHECK(evaluate("12 / 2 / 2") == EL::Value(3.0));
            CHECK(evaluate("12 / 2 / 2 / 3") == EL::Value(1.0));
        }

        TEST_CASE("ELParserTest.parseModulus", "[ELParserTest]") {
            CHECK(evaluate("12 % 2.0") == EL::Value(0.0));
            CHECK(evaluate("12 % 5 % 3") == EL::Value(2.0));
            CHECK(evaluate("12 % 5 % 3 % 3") == EL::Value(2.0));
        }

        TEST_CASE("ELParserTest.parseLogicalAnd", "[ELParserTest]") {
            CHECK(evaluate("true && true") == EL::Value(true));
            CHECK(evaluate("false && true") == EL::Value(false));
            CHECK(evaluate("true && false") == EL::Value(false));
            CHECK(evaluate("false && false") == EL::Value(false));
        }

        TEST_CASE("ELParserTest.parseLogicalOr", "[ELParserTest]") {
            CHECK(evaluate("true || true") == EL::Value(true));
            CHECK(evaluate("false || true") == EL::Value(true));
            CHECK(evaluate("true || false") == EL::Value(true));
            CHECK(evaluate("false || false") == EL::Value(false));
        }

        TEST_CASE("ELParserTest.parseBitwiseAnd", "[ELParserTest]") {
            CHECK(evaluate("23 & 24") == EL::Value(23 & 24));
        }

        TEST_CASE("ELParserTest.parseBitwiseOr", "[ELParserTest]") {
            CHECK(evaluate("23 | 24") == EL::Value(23 | 24));
        }

        TEST_CASE("ELParserTest.parseBitwiseXor", "[ELParserTest]") {
            CHECK(evaluate("23 ^ 24") == EL::Value((23 ^ 24)));
            CHECK_THROWS_AS(evaluate("23 ^^ 23"), ParserException);
        }

        TEST_CASE("ELParserTest.parseBitwiseShiftLeft", "[ELParserTest]") {
            CHECK(evaluate("1 << 7") == EL::Value(1 << 7));
        }

        TEST_CASE("ELParserTest.parseBitwiseShiftRight", "[ELParserTest]") {
            CHECK(evaluate("8 >> 2") == EL::Value(8 >> 2));
        }

        TEST_CASE("ELParserTest.parseSubscript", "[ELParserTest]") {
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][0]") == EL::Value(1.0));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][1]") == EL::Value(2.0));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][2]") == EL::Value("test"));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][-1]") == EL::Value("test"));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][-2]") == EL::Value(2.0));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][-3]") == EL::Value(1.0));

            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][1 + 1]") == EL::Value("test"));

            CHECK(evaluate("{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key1\"]") == EL::Value(1.0));
            CHECK(evaluate("{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key2\"]") == EL::Value(2.0));
            CHECK(evaluate("{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key3\"]") == EL::Value("test"));

            CHECK(evaluate("[ 1.0, [ 2.0, \"test\"] ][0]") == EL::Value(1.0));
            CHECK(evaluate("[ 1.0, [ 2.0, \"test\"] ][1][0]") == EL::Value(2.0));
            CHECK(evaluate("[ 1.0, [ 2.0, \"test\"] ][1][1]") == EL::Value("test"));

            CHECK(evaluate("{ \"key1\":1, \"key2\":2, \"key3\":[ 1, 2]}[\"key3\"][1]") == EL::Value(2.0));

            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][0,1,2]") ==
                           EL::Value(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value("test") })));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][0..2]") ==
                           EL::Value(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value("test") })));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][2..0]") ==
                           EL::Value(EL::ArrayType({ EL::Value("test"), EL::Value(2.0), EL::Value(1.0) })));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][0,1..2]") ==
                           EL::Value(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value("test") })));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][1..]") == EL::Value(EL::ArrayType({ EL::Value(2.0), EL::Value("test") })));
            CHECK(evaluate("[ 1.0, 2.0, \"test\" ][..1]") == EL::Value(EL::ArrayType({ EL::Value("test"), EL::Value(2.0) })));

            CHECK(evaluate("\"test\"[3,2,1,0]") == EL::Value("tset"));
            CHECK(evaluate("\"test\"[2,1,0]") == EL::Value("set"));
            CHECK(evaluate("\"test\"[2..1]") == EL::Value("se"));

            CHECK(evaluate("\"test\"[..0]") == EL::Value("tset"));
            CHECK(evaluate("\"test\"[1..]") == EL::Value("est"));
        }

        TEST_CASE("ELParserTest.parseCaseOperator", "[ELParserTest]") {
            CHECK(evaluate("true -> false") == EL::Value(false));
            CHECK(evaluate("true -> true && true") == EL::Value(true));
            CHECK(evaluate("1 < 3 -> 2 + 3") == EL::Value(5));
            CHECK(evaluate("false -> true") == EL::Value(EL::Value::Undefined));
        }

        TEST_CASE("ELParserTest.parseBinaryNegation", "[ELParserTest]") {
            CHECK(evaluate("~1") == EL::Value(~1l));
        }

        TEST_CASE("ELParserTest.parseSwitchExpression", "[ELParserTest]") {
            CHECK(evaluate("{{}}") == EL::Value::Undefined);
            CHECK(evaluate("{{'asdf'}}") == EL::Value("asdf"));
            CHECK(evaluate("{{'fdsa', 'asdf'}}") == EL::Value("fdsa"));
            CHECK(evaluate("{{false -> 'fdsa', 'asdf'}}") == EL::Value("asdf"));
            CHECK(evaluate("{{false -> false}}") == EL::Value::Undefined);
        }

        TEST_CASE("ELParserTest.testComparisonOperators", "[ELParserTest]") {
            CHECK(evaluate("1 < 2") == EL::Value(true));
            CHECK(evaluate("2 < 2") == EL::Value(false));
            CHECK(evaluate("1 <= 2") == EL::Value(true));
            CHECK(evaluate("2 <= 2") == EL::Value(true));
            CHECK(evaluate("3 <= 2") == EL::Value(false));

            CHECK(evaluate("\"test\" == \"test\"") == EL::Value(true));
            CHECK(evaluate("\"test1\" == \"test\"") == EL::Value(false));
            CHECK(evaluate("\"test\" != \"test\"") == EL::Value(false));
            CHECK(evaluate("\"test1\" != \"test\"") == EL::Value(true));

            CHECK(evaluate("2 > 1") == EL::Value(true));
            CHECK(evaluate("2 > 2") == EL::Value(false));
            CHECK(evaluate("2 >= 1") == EL::Value(true));
            CHECK(evaluate("2 >= 2") == EL::Value(true));
            CHECK(evaluate("2 >= 3") == EL::Value(false));
        }

        TEST_CASE("ELParserTest.testOperatorPrecedence", "[ELParserTest]") {
            CHECK(evaluate("7 + 2 * 3") == evaluate("2 * 3 + 7"));
            CHECK(evaluate("7 + 2 * 3 + 2") == evaluate("2 * 3 + 7 + 2"));
            CHECK(evaluate("7 + 2 * 3 + 2 * 2") == evaluate("2 * 3 + 7 + 2 * 2"));
            CHECK(evaluate("7 + 2 / 3 + 2 * 2") == evaluate("2 / 3 + 7 + 2 * 2"));

            CHECK(evaluate("3 + 2 < 3 + 3") == evaluate("(3 + 2) < (3 + 3)"));
            CHECK(evaluate("3 + 2 < 3 + 3 + 0 && true") == evaluate("((3 + 2) < (3 + 3 + 0)) && true"));
            CHECK(evaluate("false && false || true") == EL::Value(true));
            CHECK(evaluate("false && (false || true)") == EL::Value(false));
        }

        TEST_CASE("ELParserTest.testParseGrouping", "[ELParserTest]") {
            CHECK_THROWS_AS(evaluate("()"), ParserException);
            CHECK(evaluate("(1)") == EL::Value(1.0));
            CHECK(evaluate("(2+1)*3") == EL::Value(9.0));
            CHECK(evaluate("(2+1)*(2+1)") == EL::Value(9.0));
            CHECK(evaluate("(2+1)*((1+1)*2)") == EL::Value(12.0));
        }
    }
}
