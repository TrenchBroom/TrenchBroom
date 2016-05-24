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
        TEST(ELParserTest, parseEmptyExpression) {
            ASSERT_THROW(ELParser("").parse(), ParserException);
            ASSERT_THROW(ELParser("   ").parse(), ParserException);
            ASSERT_THROW(ELParser("\n").parse(), ParserException);
        }
        
        TEST(ELParserTEst, parseStringLiteral) {
            ASSERT_THROW(ELParser("\"asdf").parse(), ParserException);
            
            EL::Expression* expression = ELParser("\"asfd\"").parse();
            ASSERT_TRUE(expression != NULL);
            ASSERT_EQ(EL::Value("asfd"), expression->evaluate(EL::EvaluationContext()));
            delete expression;
        }
        
        TEST(ELParserTest, parseNumberLiteral) {
            ASSERT_THROW(ELParser("1.123.34").parse(), ParserException);
            
            ASSERT_EQ(EL::Value(1.0), ELParser("1").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(1.0), ELParser("1.0").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(1.0), ELParser("01.00").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(0.0), ELParser(".0").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(0.0), ELParser("0").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseBooleanLiteral) {
            ASSERT_EQ(EL::Value(true), ELParser("true").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(false), ELParser("false").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseArrayLiteral) {
            ASSERT_EQ(EL::Value(EL::ArrayType()), ELParser("[]").parse()->evaluate(EL::EvaluationContext()));
            
            EL::ArrayType array, nestedArray;
            array.push_back(EL::Value(1.0));
            array.push_back(EL::Value("test"));
            nestedArray.push_back(EL::Value(true));
            array.push_back(EL::Value(nestedArray));
            
            ASSERT_EQ(array, ELParser("[ 1.0 , \"test\",[ true] ]").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseMapLiteral) {
            ASSERT_EQ(EL::Value(EL::MapType()), ELParser("{}").parse()->evaluate(EL::EvaluationContext()));
            
            EL::MapType map, nestedMap;
            map.insert(std::make_pair("testkey1", EL::Value(1.0)));
            map.insert(std::make_pair("testkey2", EL::Value("asdf")));
            nestedMap.insert(std::make_pair("nestedKey", EL::Value(true)));
            map.insert(std::make_pair("testkey3", EL::Value(nestedMap)));
            
            ASSERT_EQ(map, ELParser(" { \"testkey1\": 1, \"testkey2\"   :\"asdf\", \"testkey3\":{\"nestedKey\":true} }").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseVariable) {
            EL::EvaluationContext context;
            context.defineVariable("test", EL::Value(1.0));
            
            ASSERT_EQ(EL::Value(1.0), ELParser("test").parse()->evaluate(context));
        }
        
        TEST(ELParserTest, parseUnaryPlus) {
            ASSERT_EQ(EL::Value(1.0), ELParser("+1.0").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseUnaryMinus) {
            ASSERT_EQ(EL::Value(-1.0), ELParser("-1.0").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseAddition) {
            ASSERT_EQ(EL::Value(5.0), ELParser("2 + 3").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value("asdf"), ELParser("\"as\"+\"df\"").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(9.0), ELParser("2 + 3 + 4").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseSubtraction) {
            ASSERT_EQ(EL::Value(-1.0), ELParser("2 - 3.0").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(-5.0), ELParser("2 - 3 - 4").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(-7.0), ELParser("2 - 3 - 4 - 2").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseMultiplication) {
            ASSERT_EQ(EL::Value(6.0), ELParser("2 * 3.0").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(24.0), ELParser("2 * 3 * 4").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(48.0), ELParser("2 * 3 * 4 * 2").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseDivision) {
            ASSERT_EQ(EL::Value(6.0), ELParser("12 / 2.0").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(3.0), ELParser("12 / 2 / 2").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(1.0), ELParser("12 / 2 / 2 / 3").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseModulus) {
            ASSERT_EQ(EL::Value(0.0), ELParser("12 % 2.0").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(2.0), ELParser("12 % 5 % 3").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(2.0), ELParser("12 % 5 % 3 % 3").parse()->evaluate(EL::EvaluationContext()));
        }
        
        TEST(ELParserTest, parseSubscript) {
            ASSERT_EQ(EL::Value(1.0), ELParser("[ 1.0, 2.0, \"test\" ][0]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(2.0), ELParser("[ 1.0, 2.0, \"test\" ][1]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value("test"), ELParser("[ 1.0, 2.0, \"test\" ][2]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value("test"), ELParser("[ 1.0, 2.0, \"test\" ][-1]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(2.0), ELParser("[ 1.0, 2.0, \"test\" ][-2]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(1.0), ELParser("[ 1.0, 2.0, \"test\" ][-3]").parse()->evaluate(EL::EvaluationContext()));
            
            ASSERT_EQ(EL::Value(1.0), ELParser("{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key1\"]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(2.0), ELParser("{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key2\"]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value("test"), ELParser("{ \"key1\":1, \"key2\":2, \"key3\":\"test\"}[\"key3\"]").parse()->evaluate(EL::EvaluationContext()));

            ASSERT_EQ(EL::Value(1.0), ELParser("[ 1.0, [ 2.0, \"test\"] ][0]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value(2.0), ELParser("[ 1.0, [ 2.0, \"test\"] ][1][0]").parse()->evaluate(EL::EvaluationContext()));
            ASSERT_EQ(EL::Value("test"), ELParser("[ 1.0, [ 2.0, \"test\"] ][1][1]").parse()->evaluate(EL::EvaluationContext()));

            ASSERT_EQ(EL::Value(2.0), ELParser("{ \"key1\":1, \"key2\":2, \"key3\":[ 1, 2]}[\"key3\"][1]").parse()->evaluate(EL::EvaluationContext()));
        }
    }
}
