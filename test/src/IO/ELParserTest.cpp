/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <gtest/gtest.h>

#include "EL.h"
#include "IO/ELParser.h"

#include <limits>

namespace TrenchBroom {
    namespace IO {
#define ASSERT_EL_THROW(str, exception) ASSERT_THROW(ELParser(str).parse(), exception)
        
        template <typename Exp>
        void ASSERT_EL_EQ(const Exp& expected, const String& str, const EL::EvaluationContext& context = EL::EvaluationContext()) {
            const EL::Expression* expression = ELParser(str).parse();
            ASSERT_EQ(EL::Value(expected), expression->evaluate(context));
            delete expression;
        }
        
        void ASSERT_ELS_EQ(const String& lhs, const String& rhs, const EL::EvaluationContext& context = EL::EvaluationContext());
        void ASSERT_ELS_EQ(const String& lhs, const String& rhs, const EL::EvaluationContext& context) {
            const EL::Expression* expression1 = ELParser(lhs).parse();
            const EL::Expression* expression2 = ELParser(rhs).parse();
            ASSERT_EQ(expression1->evaluate(context), expression2->evaluate(context));
            delete expression1;
            delete expression2;
        }

        TEST(ELParserTest, parseEmptyExpression) {
            ASSERT_EL_THROW("", ParserException);
            ASSERT_EL_THROW("    ", ParserException);
            ASSERT_EL_THROW("\n", ParserException);
        }
        
        TEST(ELParserTEst, parseStringLiteral) {
            ASSERT_EL_THROW("\"asdf", ParserException);
            ASSERT_EL_EQ("asdf", "\"asdf\"");
        }
        
        TEST(ELParserTest, parseNumberLiteral) {
            ASSERT_EL_THROW("1.123.34", ParserException);
            
            ASSERT_EL_EQ(1.0, "1");
            ASSERT_EL_EQ(1.0, "1.0");
            ASSERT_EL_EQ(1.0, "01.00");
            ASSERT_EL_EQ(0.0, ".0");
            ASSERT_EL_EQ(0.0, "0");
        }
        
        TEST(ELParserTest, parseBooleanLiteral) {
            ASSERT_EL_EQ(true, "true");
            ASSERT_EL_EQ(false, "false");
        }
        
        TEST(ELParserTest, parseArrayLiteral) {
            EL::ArrayType array, nestedArray;
            array.push_back(EL::Value(1.0));
            array.push_back(EL::Value("test"));
            nestedArray.push_back(EL::Value(true));
            array.push_back(EL::Value(nestedArray));
            
            ASSERT_EL_EQ(EL::ArrayType(), "[]");
            ASSERT_EL_EQ(array, "[ 1.0 , \"test\",[ true] ]");
        }
        
        TEST(ELParserTest, parseMapLiteral) {
            EL::MapType map, nestedMap;
            map.insert(std::make_pair("testkey1", EL::Value(1.0)));
            map.insert(std::make_pair("testkey2", EL::Value("asdf")));
            nestedMap.insert(std::make_pair("nestedKey", EL::Value(true)));
            map.insert(std::make_pair("testkey3", EL::Value(nestedMap)));
            
            ASSERT_EL_EQ(EL::MapType(), "{}");
            ASSERT_EL_EQ(map, " { \"testkey1\": 1, \"testkey2\"   :\"asdf\", \"testkey3\":{\"nestedKey\":true} }");
        }
        
        TEST(ELParserTest, parseVariable) {
            EL::EvaluationContext context;
            context.defineVariable("test", EL::Value(1.0));

            ASSERT_EL_EQ(1.0, "test", context);
        }
        
        TEST(ELParserTest, parseUnaryPlus) {
            ASSERT_EL_EQ(1.0, "+1.0");
        }
        
        TEST(ELParserTest, parseUnaryMinus) {
            ASSERT_EL_EQ(-1.0, "-1.0");
        }
        
        TEST(ELParserTest, parseAddition) {
            ASSERT_EL_EQ(5.0, "2 + 3");
            ASSERT_EL_EQ("asdf", "\"as\"+\"df\"");
            ASSERT_EL_EQ(9.0, "2 + 3 + 4");
        }
        
        TEST(ELParserTest, parseSubtraction) {
            ASSERT_EL_EQ(-1.0, "2 - 3.0");
            ASSERT_EL_EQ(-5.0, "2 - 3 - 4");
            ASSERT_EL_EQ(-7.0, "2 - 3 - 4 - 2");
        }
        
        TEST(ELParserTest, parseMultiplication) {
            ASSERT_EL_EQ(6.0, "2 * 3.0");
            
            ASSERT_EL_EQ(24.0, "2 * 3 * 4");
            ASSERT_EL_EQ(48.0, "2 * 3 * 4 * 2");
        }
        
        TEST(ELParserTest, parseDivision) {
            ASSERT_EL_EQ(6.0, "12 / 2.0");
            ASSERT_EL_EQ(3.0, "12 / 2 / 2");
            ASSERT_EL_EQ(1.0, "12 / 2 / 2 / 3");
        }
        
        TEST(ELParserTest, parseModulus) {
            ASSERT_EL_EQ(0.0, "12 % 2.0");
            ASSERT_EL_EQ(2.0, "12 % 5 % 3");
            ASSERT_EL_EQ(2.0, "12 % 5 % 3 % 3");
        }
        
        TEST(ELParserTest, parseSubscript) {
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
        }
        
        TEST(ELParserTest, testOperatorPrecedence) {
            ASSERT_ELS_EQ("7 + 2 * 3", "2 * 3 + 7");
            ASSERT_ELS_EQ("7 + 2 * 3 + 2", "2 * 3 + 7 + 2");
            ASSERT_ELS_EQ("7 + 2 * 3 + 2 * 2", "2 * 3 + 7 + 2 * 2");
            ASSERT_ELS_EQ("7 + 2 / 3 + 2 * 2", "2 / 3 + 7 + 2 * 2");
        }
        
        TEST(ELParserTest, testParseGrouping) {
            ASSERT_EL_THROW("()", ParserException);
            ASSERT_EL_EQ(1.0, "(1)");
            ASSERT_EL_EQ(9.0, "(2+1)*3");
            ASSERT_EL_EQ(9.0, "(2+1)*(2+1)");
        }
    }
}
