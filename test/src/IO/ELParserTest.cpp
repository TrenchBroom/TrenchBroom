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
    }
}
