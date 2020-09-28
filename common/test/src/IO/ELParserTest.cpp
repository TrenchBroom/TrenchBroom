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
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
#define ASSERT_EL_THROW(str, exception) ASSERT_THROW(ELParser::parseStrict(str).evaluate(EL::EvaluationContext()), exception)

        template <typename Exp>
        void ASSERT_EL_EQ(const Exp& expected, const std::string& str, const EL::EvaluationContext& context = EL::EvaluationContext()) {
            const EL::Expression expression = ELParser::parseStrict(str);
            ASSERT_EQ(EL::Value(expected), expression.evaluate(context));
        }

        void ASSERT_ELS_EQ(const std::string& lhs, const std::string& rhs, const EL::EvaluationContext& context = EL::EvaluationContext());
        void ASSERT_ELS_EQ(const std::string& lhs, const std::string& rhs, const EL::EvaluationContext& context) {
            const EL::Expression expression1 = ELParser::parseStrict(lhs);
            const EL::Expression expression2 = ELParser::parseStrict(rhs);
            ASSERT_EQ(expression1.evaluate(context), expression2.evaluate(context));
        }

        TEST_CASE("ELParserTest.parseEmptyExpression", "[ELParserTest]") {
            ASSERT_EL_THROW("", ParserException);
            ASSERT_EL_THROW("    ", ParserException);
            ASSERT_EL_THROW("\n", ParserException);
        }

        TEST_CASE("ELParserTest.parseStringLiteral", "[ELParserTest]") {
            ASSERT_EL_THROW("\"asdf", ParserException);
            ASSERT_EL_EQ("asdf", "\"asdf\"");
        }

        TEST_CASE("ELParserTest.parseStringLiteralWithDoubleQuotationMarks", "[ELParserTest]") {
            ASSERT_EL_EQ(R"(asdf" "asdf)", R"("asdf\" \"asdf")");
        }

        TEST_CASE("ELParserTest.parseNumberLiteral", "[ELParserTest]") {
            ASSERT_EL_THROW("1.123.34", ParserException);

            ASSERT_EL_EQ(1.0, "1");
            ASSERT_EL_EQ(1.0, "1.0");
            ASSERT_EL_EQ(1.0, "01.00");
            ASSERT_EL_EQ(0.0, ".0");
            ASSERT_EL_EQ(0.0, "0");
        }

        TEST_CASE("ELParserTest.parseBooleanLiteral", "[ELParserTest]") {
            ASSERT_EL_EQ(true, "true");
            ASSERT_EL_EQ(false, "false");
        }

        TEST_CASE("ELParserTest.parseArrayLiteral", "[ELParserTest]") {
            EL::ArrayType array, nestedArray;
            array.push_back(EL::Value(1.0));
            array.push_back(EL::Value("test"));
            nestedArray.push_back(EL::Value(true));
            array.push_back(EL::Value(nestedArray));

            ASSERT_EL_EQ(EL::ArrayType(), "[]");
            ASSERT_EL_EQ(array, "[ 1.0 , \"test\",[ true] ]");

            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value(3.0) }), "[1..3]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(3.0), EL::Value(2.0), EL::Value(1.0) }), "[3..1]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(1.0) }), "[1..1]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(1.0), EL::Value(0.0) }), "[1..0]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(-2.0), EL::Value(-1.0), EL::Value(0.0), EL::Value(1.0) }), "[-2..1]");
        }

        TEST_CASE("ELParserTest.parseMapLiteral", "[ELParserTest]") {
            EL::MapType map, nestedMap;
            map.insert(std::make_pair("testkey1", EL::Value(1.0)));
            map.insert(std::make_pair("testkey2", EL::Value("asdf")));
            nestedMap.insert(std::make_pair("nestedKey", EL::Value(true)));
            map.insert(std::make_pair("testkey3", EL::Value(nestedMap)));

            ASSERT_EL_EQ(EL::MapType(), "{}");
            ASSERT_EL_EQ(map, " { \"testkey1\": 1, \"testkey2\"   :\"asdf\", \"testkey3\":{\"nestedKey\":true} }");
        }

        TEST_CASE("ELParserTest.parseMapLiteralNestedInArray", "[ELParserTest]") {
            EL::MapType map;
            map.insert(std::make_pair("key", EL::Value("value")));

            EL::ArrayType array;
            array.push_back(EL::Value(map));

            ASSERT_EL_EQ(array, R"([ { "key": "value" } ])");
        }

        TEST_CASE("ELParserTest.parseMapLiteralNestedInArrayNestedInMap", "[ELParserTest]") {
            EL::MapType inner;
            inner.insert(std::make_pair("key", EL::Value("value")));

            EL::ArrayType array;
            array.push_back(EL::Value(inner));

            EL::MapType outer;
            outer.insert(std::make_pair("outerkey1", EL::Value(array)));
            outer.insert(std::make_pair("outerkey2", EL::Value("asdf")));

            ASSERT_EL_EQ(outer, R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })");
        }

        TEST_CASE("ELParserTest.parseMapLiteralWithTrailingGarbage", "[ELParserTest]") {
            ASSERT_EL_THROW("{\n"
                            "\t\"profiles\": [],\n"
                            "\t\"version\": 1\n"
                            "}\n"
                            "asdf", ParserException);
        }

        TEST_CASE("ELParserTest.parseVariable", "[ELParserTest]") {
            EL::EvaluationContext context;
            context.declareVariable("test", EL::Value(1.0));

            ASSERT_EL_EQ(1.0, "test", context);
        }

        TEST_CASE("ELParserTest.parseUnaryPlus", "[ELParserTest]") {
            ASSERT_EL_EQ(1.0, "+1.0");
        }

        TEST_CASE("ELParserTest.parseUnaryMinus", "[ELParserTest]") {
            ASSERT_EL_EQ(-1.0, "-1.0");
        }

        TEST_CASE("ELParserTest.parseLogicalNegation", "[ELParserTest]") {
            ASSERT_EL_EQ(false, "!true");
            ASSERT_EL_EQ(true, "!false");
            ASSERT_EL_THROW("!0", EL::ConversionError);
            ASSERT_EL_THROW("!1", EL::ConversionError);
            ASSERT_EL_THROW("!'true'", EL::ConversionError);
        }

        TEST_CASE("ELParserTest.parseBitwiseNegation", "[ELParserTest]") {
            ASSERT_EL_EQ(~393, "~393");
            ASSERT_EL_THROW("~", ParserException);
            ASSERT_EL_THROW("~~", ParserException);
        }

        TEST_CASE("ELParserTest.parseAddition", "[ELParserTest]") {
            ASSERT_EL_EQ(5.0, "2 + 3");
            ASSERT_EL_EQ("asdf", "\"as\"+\"df\"");
            ASSERT_EL_EQ(9.0, "2 + 3 + 4");
        }

        TEST_CASE("ELParserTest.parseSubtraction", "[ELParserTest]") {
            ASSERT_EL_EQ(-1.0, "2 - 3.0");
            ASSERT_EL_EQ(-5.0, "2 - 3 - 4");
            ASSERT_EL_EQ(-7.0, "2 - 3 - 4 - 2");
        }

        TEST_CASE("ELParserTest.parseMultiplication", "[ELParserTest]") {
            ASSERT_EL_EQ(6.0, "2 * 3.0");

            ASSERT_EL_EQ(24.0, "2 * 3 * 4");
            ASSERT_EL_EQ(48.0, "2 * 3 * 4 * 2");
        }

        TEST_CASE("ELParserTest.parseDivision", "[ELParserTest]") {
            ASSERT_EL_EQ(6.0, "12 / 2.0");
            ASSERT_EL_EQ(3.0, "12 / 2 / 2");
            ASSERT_EL_EQ(1.0, "12 / 2 / 2 / 3");
        }

        TEST_CASE("ELParserTest.parseModulus", "[ELParserTest]") {
            ASSERT_EL_EQ(0.0, "12 % 2.0");
            ASSERT_EL_EQ(2.0, "12 % 5 % 3");
            ASSERT_EL_EQ(2.0, "12 % 5 % 3 % 3");
        }

        TEST_CASE("ELParserTest.parseLogicalAnd", "[ELParserTest]") {
            ASSERT_EL_EQ(true, "true && true");
            ASSERT_EL_EQ(false, "false && true");
            ASSERT_EL_EQ(false, "true && false");
            ASSERT_EL_EQ(false, "false && false");
        }

        TEST_CASE("ELParserTest.parseLogicalOr", "[ELParserTest]") {
            ASSERT_EL_EQ(true, "true || true");
            ASSERT_EL_EQ(true, "false || true");
            ASSERT_EL_EQ(true, "true || false");
            ASSERT_EL_EQ(false, "false || false");
        }

        TEST_CASE("ELParserTest.parseBitwiseAnd", "[ELParserTest]") {
            ASSERT_EL_EQ(23 & 24, "23 & 24");
        }

        TEST_CASE("ELParserTest.parseBitwiseOr", "[ELParserTest]") {
            ASSERT_EL_EQ(23 | 24, "23 | 24");
        }

        TEST_CASE("ELParserTest.parseBitwiseXor", "[ELParserTest]") {
            ASSERT_EL_EQ(23 ^ 24, "23 ^ 24");
            ASSERT_EL_THROW("23 ^^ 23", ParserException);
        }

        TEST_CASE("ELParserTest.parseBitwiseShiftLeft", "[ELParserTest]") {
            ASSERT_EL_EQ(1 << 7, "1 << 7");
        }

        TEST_CASE("ELParserTest.parseBitwiseShiftRight", "[ELParserTest]") {
            ASSERT_EL_EQ(8 >> 2, "8 >> 2");
        }

        TEST_CASE("ELParserTest.parseSubscript", "[ELParserTest]") {
            ASSERT_EL_EQ(1.0, "[ 1.0, 2.0, \"test\" ][0]");
            ASSERT_EL_EQ(2.0, "[ 1.0, 2.0, \"test\" ][1]");
            ASSERT_EL_EQ("test", "[ 1.0, 2.0, \"test\" ][2]");
            ASSERT_EL_EQ("test", "[ 1.0, 2.0, \"test\" ][-1]");
            ASSERT_EL_EQ(2.0, "[ 1.0, 2.0, \"test\" ][-2]");
            ASSERT_EL_EQ(1.0, "[ 1.0, 2.0, \"test\" ][-3]");

            ASSERT_EL_EQ("test", "[ 1.0, 2.0, \"test\" ][1 + 1]");

            ASSERT_EL_EQ(1.0, "{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key1\"]");
            ASSERT_EL_EQ(2.0, "{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key2\"]");
            ASSERT_EL_EQ("test", "{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key3\"]");

            ASSERT_EL_EQ(1.0, "[ 1.0, [ 2.0, \"test\"] ][0]");
            ASSERT_EL_EQ(2.0, "[ 1.0, [ 2.0, \"test\"] ][1][0]");
            ASSERT_EL_EQ("test", "[ 1.0, [ 2.0, \"test\"] ][1][1]");

            ASSERT_EL_EQ(2.0, "{ \"key1\":1, \"key2\":2, \"key3\":[ 1, 2]}[\"key3\"][1]");

            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value("test") }), "[ 1.0, 2.0, \"test\" ][0,1,2]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value("test") }), "[ 1.0, 2.0, \"test\" ][0..2]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value("test"), EL::Value(2.0), EL::Value(1.0) }), "[ 1.0, 2.0, \"test\" ][2..0]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(1.0), EL::Value(2.0), EL::Value("test") }), "[ 1.0, 2.0, \"test\" ][0,1..2]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value(2.0), EL::Value("test") }), "[ 1.0, 2.0, \"test\" ][1..]");
            ASSERT_EL_EQ(EL::ArrayType({ EL::Value("test"), EL::Value(2.0) }), "[ 1.0, 2.0, \"test\" ][..1]");

            ASSERT_EL_EQ("tset", "\"test\"[3,2,1,0]");
            ASSERT_EL_EQ("set", "\"test\"[2,1,0]");
            ASSERT_EL_EQ("se", "\"test\"[2..1]");

            ASSERT_EL_EQ("tset", "\"test\"[..0]");
            ASSERT_EL_EQ("est", "\"test\"[1..]");
        }

        TEST_CASE("ELParserTest.parseCaseOperator", "[ELParserTest]") {
            ASSERT_EL_EQ(false, "true -> false");
            ASSERT_EL_EQ(true, "true -> true && true");
            ASSERT_EL_EQ(5, "1 < 3 -> 2 + 3");
            ASSERT_EL_EQ(EL::Value::Undefined, "false -> true");
        }

        TEST_CASE("ELParserTest.parseBinaryNegation", "[ELParserTest]") {
            ASSERT_EL_EQ(~1l, "~1");
        }

        TEST_CASE("ELParserTest.parseSwitchExpression", "[ELParserTest]") {
            ASSERT_EL_EQ(EL::Value::Undefined, "{{}}");
            ASSERT_EL_EQ("asdf", "{{'asdf'}}");
            ASSERT_EL_EQ("fdsa", "{{'fdsa', 'asdf'}}");
            ASSERT_EL_EQ("asdf", "{{false -> 'fdsa', 'asdf'}}");
            ASSERT_EL_EQ(EL::Value::Undefined, "{{false -> false}}");
        }

        TEST_CASE("ELParserTest.testComparisonOperators", "[ELParserTest]") {
            ASSERT_EL_EQ(true, "1 < 2");
            ASSERT_EL_EQ(false, "2 < 2");
            ASSERT_EL_EQ(true, "1 <= 2");
            ASSERT_EL_EQ(true, "2 <= 2");
            ASSERT_EL_EQ(false, "3 <= 2");

            ASSERT_EL_EQ(true, "\"test\" == \"test\"");
            ASSERT_EL_EQ(false, "\"test1\" == \"test\"");
            ASSERT_EL_EQ(false, "\"test\" != \"test\"");
            ASSERT_EL_EQ(true, "\"test1\" != \"test\"");

            ASSERT_EL_EQ(true, "2 > 1");
            ASSERT_EL_EQ(false, "2 > 2");
            ASSERT_EL_EQ(true, "2 >= 1");
            ASSERT_EL_EQ(true, "2 >= 2");
            ASSERT_EL_EQ(false, "2 >= 3");
        }

        TEST_CASE("ELParserTest.testOperatorPrecedence", "[ELParserTest]") {
            ASSERT_ELS_EQ("7 + 2 * 3", "2 * 3 + 7");
            ASSERT_ELS_EQ("7 + 2 * 3 + 2", "2 * 3 + 7 + 2");
            ASSERT_ELS_EQ("7 + 2 * 3 + 2 * 2", "2 * 3 + 7 + 2 * 2");
            ASSERT_ELS_EQ("7 + 2 / 3 + 2 * 2", "2 / 3 + 7 + 2 * 2");

            ASSERT_ELS_EQ("3 + 2 < 3 + 3", "(3 + 2) < (3 + 3)");
            ASSERT_ELS_EQ("3 + 2 < 3 + 3 + 0 && true", "((3 + 2) < (3 + 3 + 0)) && true");
            ASSERT_EL_EQ(true, "false && false || true");
            ASSERT_EL_EQ(false, "false && (false || true)");
        }

        TEST_CASE("ELParserTest.testParseGrouping", "[ELParserTest]") {
            ASSERT_EL_THROW("()", ParserException);
            ASSERT_EL_EQ(1.0, "(1)");
            ASSERT_EL_EQ(9.0, "(2+1)*3");
            ASSERT_EL_EQ(9.0, "(2+1)*(2+1)");
            ASSERT_EL_EQ(12.0, "(2+1)*((1+1)*2)");
        }
    }
}
