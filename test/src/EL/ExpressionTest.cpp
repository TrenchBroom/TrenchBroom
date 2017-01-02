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

#include <cmath>

#include "CollectionUtils.h"
#include "EL.h"
#include "IO/ELParser.h"

namespace TrenchBroom {
    namespace EL {
        typedef Value V;
        
        void evaluateAndAssert(const String& expression, const Value& result, const EvaluationContext& context = EvaluationContext());
        
        template <typename T>
        void evaluateAndAssert(const String& expression, const T& result, const EvaluationContext& context = EvaluationContext()) {
            evaluateAndAssert(expression, Value(result), context);
        }
        
        template <typename T, typename S>
        void evaluateAndAssert(const String& expression, const T& result, const String& n1, const S& v1) {
            VariableTable table;
            table.declare(n1);
            table.assign(n1, Value(v1));
            evaluateAndAssert(expression, result, EvaluationContext(table));
        }
        
        template <typename E>
        void evaluateAndThrow(const String& expression, const EvaluationContext& context = EvaluationContext()) {
            ASSERT_THROW(IO::ELParser(expression).parse().evaluate(context), E);
        }
        
        template <typename T1>
        ArrayType array(const T1& v1) {
            return VectorUtils::create<V>(V(v1));
        }
        
        template <typename T1, typename T2>
        ArrayType array(const T1& v1, const T2& v2) {
            return VectorUtils::create<V>(V(v1), V(v2));
        }
        
        template <typename T1, typename T2, typename T3>
        ArrayType array(const T1& v1, const T2& v2, const T3& v3) {
            return VectorUtils::create<V>(V(v1), V(v2), V(v3));
        }
        
        template <typename T1>
        MapType map(const String& k1, const T1& v1) {
            MapType m;
            m[k1] = V(v1);
            return m;
        }
        
        template <typename T1, typename T2>
        MapType map(const String& k1, const T1& v1,
                    const String& k2, const T2& v2) {
            MapType m;
            m[k1] = V(v1);
            m[k2] = V(v2);
            return m;
        }
        
        template <typename T1, typename T2, typename T3>
        MapType map(const String& k1, const T1& v1,
                    const String& k2, const T2& v2,
                    const String& k3, const T3& v3) {
            MapType m;
            m[k1] = V(v1);
            m[k2] = V(v2);
            m[k3] = V(v3);
            return m;
        }
        
        void assertOptimizable(const String& expression);
        void assertNotOptimizable(const String& expression);
        
        TEST(ExpressionTest, testValueLiterals) {
            evaluateAndAssert("true", true);
            evaluateAndAssert("false", false);
            evaluateAndAssert("'asdf'", "asdf");
            evaluateAndAssert("2", 2);
            evaluateAndAssert("-2", -2);
        }
        
        TEST(ExpressionTest, testVariableExpression) {
            evaluateAndAssert("x", true, "x", true);
            evaluateAndAssert("ohhai", 7, "ohhai", 7);
            evaluateAndAssert("x", Value::Undefined);
        }
        
        TEST(ExpressionTest, testArrayExpression) {
            evaluateAndAssert("[]", ArrayType());
            evaluateAndAssert("[1, 2, 3]", array(1, 2, 3));
            evaluateAndAssert("[1, 2, x]", array(1, 2, "test"), "x", "test");
            
            assertOptimizable("[]");
            assertOptimizable("[1, 2, 3]");
            assertNotOptimizable("[1, 2, x]");
        }
        
        TEST(ExpressionTest, testMapExpression) {
            evaluateAndAssert("{}", MapType());
            evaluateAndAssert("{ 'k': true }", map("k", true));
            evaluateAndAssert("{ 'k1': true, 'k2': 3, 'k3': 3 + 7 }", map("k1", true, "k2", 3, "k3", 10));
            evaluateAndAssert("{ 'k1': 'asdf', 'k2': x }", map("k1", "asdf", "k2", 55), "x", 55);

            assertOptimizable("{}");
            assertOptimizable("{ 'k': true }");
            assertOptimizable("{ 'k1': true, 'k2': 3, 'k3': 3 + 7 }");
            assertNotOptimizable("{ 'k1': 'asdf', 'k2': x }");
        }
        
        TEST(ExpressionTest, testAdditionOperator) {
            evaluateAndAssert("2 + 3", 5);
            evaluateAndAssert("-2 + 3", 1);
            evaluateAndAssert("2 + 3 + 4", 9);
            assertOptimizable("2 + 3");
            
            evaluateAndAssert("'as' + 'df'", "asdf");
        }
        
        TEST(ExpressionTest, testSubtractionOperator) {
            evaluateAndAssert("2 - 3", -1);
            evaluateAndAssert("-2 - 3", -5);
            evaluateAndAssert("2 - 3 - 4", -5);
            assertOptimizable("2 - 3");
        }
        
        TEST(ExpressionTest, testMultiplicationOperator) {
            evaluateAndAssert("2 * 3", 6);
            evaluateAndAssert("-2 * 3", -6);
            evaluateAndAssert("2 * 3 * 4", 24);
            assertOptimizable("2 * 3");
        }
        
        TEST(ExpressionTest, testDivisionOperator) {
            evaluateAndAssert("2 / 3", 2.0 / 3.0);
            evaluateAndAssert("-2 / 3", -2.0 / 3.0);
            evaluateAndAssert("2 / 3 / 4", 2.0 / 3.0 / 4.0);
            assertOptimizable("2 / 3");
        }
        
        TEST(ExpressionTest, testModulusOperator) {
            evaluateAndAssert("3 % 2", std::fmod(3.0, 2.0));
            evaluateAndAssert("-2 % 3", std::fmod(-2.0, 3.0));
            evaluateAndAssert("13 % 8 % 4", std::fmod(std::fmod(13.0, 8.0), 4.0));
            evaluateAndAssert("2 % 0", std::fmod(2.0, 0.0));
            assertOptimizable("2 % 3");
        }
        
        TEST(ExpressionTest, testLogicalNegationOperator) {
            evaluateAndAssert("!true", false);
            evaluateAndAssert("!false", true);
            evaluateAndThrow<ConversionError>("!1");
            evaluateAndThrow<ConversionError>("!'asdf'");
            evaluateAndThrow<ConversionError>("![1,2]");
            evaluateAndThrow<ConversionError>("!{}");
            evaluateAndThrow<ConversionError>("!null");
        }
        
        TEST(ExpressionTest, testLogicalAndOperator) {
            evaluateAndAssert("false && false", false);
            evaluateAndAssert("false &&  true", false);
            evaluateAndAssert(" true && false", false);
            evaluateAndAssert(" true &&  true",  true);
            assertOptimizable("true && false");
        }
        
        TEST(ExpressionTest, testLogicalOrOperator) {
            evaluateAndAssert("false || false", false);
            evaluateAndAssert("false ||  true",  true);
            evaluateAndAssert(" true || false",  true);
            evaluateAndAssert(" true ||  true",  true);
            assertOptimizable("true || false");
        }
        
        void evalutateComparisonAndAssert(const String& op, bool result);
        
        TEST(ExpressionTest, testComparisonOperators) {
            evalutateComparisonAndAssert("<",   true);
            evalutateComparisonAndAssert("<=",  true);
            evalutateComparisonAndAssert("==", false);
            evalutateComparisonAndAssert("!=",  true);
            evalutateComparisonAndAssert(">",  false);
            evalutateComparisonAndAssert(">=", false);
        }

        TEST(ExpressionTest, testBitwiseNegationOperator) {
            evaluateAndAssert("~23423", ~23423);
            evaluateAndAssert("~23423.1", ~23423);
            evaluateAndAssert("~23423.8", ~23423);
            evaluateAndThrow<ConversionError>("~true");
            evaluateAndThrow<ConversionError>("~'asdf'");
            evaluateAndThrow<ConversionError>("~[]");
            evaluateAndThrow<ConversionError>("~{}");
            evaluateAndThrow<ConversionError>("~null");
        }
        
        TEST(ExpressionTest, testBitwiseAndOperator) {
            evaluateAndAssert("0 & 0", 0 & 0);
            evaluateAndAssert("123 & 456", 123 & 456);
            evaluateAndThrow<EvaluationError>("true & 123");
            evaluateAndThrow<EvaluationError>("'asdf' & 123");
            evaluateAndThrow<EvaluationError>("[] & 123");
            evaluateAndThrow<EvaluationError>("{} & 123");
            evaluateAndThrow<EvaluationError>("null & 123");
        }
        
        TEST(ExpressionTest, testBitwiseOrOperator) {
            evaluateAndAssert("0 | 0", 0 | 0);
            evaluateAndAssert("123 | 456", 123 | 456);
            evaluateAndThrow<EvaluationError>("true | 123");
            evaluateAndThrow<EvaluationError>("'asdf' | 123");
            evaluateAndThrow<EvaluationError>("[] | 123");
            evaluateAndThrow<EvaluationError>("{} | 123");
            evaluateAndThrow<EvaluationError>("null | 123");
        }
        
        TEST(ExpressionTest, testBitwiseXorOperator) {
            evaluateAndAssert("0 ^ 0", 0 ^ 0);
            evaluateAndAssert("123 ^ 456", 123 ^ 456);
            evaluateAndThrow<EvaluationError>("true ^ 123");
            evaluateAndThrow<EvaluationError>("'asdf' ^ 123");
            evaluateAndThrow<EvaluationError>("[] ^ 123");
            evaluateAndThrow<EvaluationError>("{} ^ 123");
            evaluateAndThrow<EvaluationError>("null ^ 123");
        }
        
        TEST(ExpressionTest, testBitwiseShiftLeftOperator) {
            evaluateAndAssert("1 << 2", 1 << 2);
#ifndef _WIN32
            evaluateAndAssert("1 << 33", 1l << 33);
#endif
            evaluateAndThrow<EvaluationError>("true << 2");
            evaluateAndThrow<EvaluationError>("1 << false");
            evaluateAndThrow<EvaluationError>("'asdf' << 2");
            evaluateAndThrow<EvaluationError>("1 << 'asdf'");
            evaluateAndThrow<EvaluationError>("[] << 2");
            evaluateAndThrow<EvaluationError>("1 << []");
            evaluateAndThrow<EvaluationError>("{} << 2");
            evaluateAndThrow<EvaluationError>("1 << {}");
            evaluateAndThrow<EvaluationError>("null << 2");
            evaluateAndThrow<EvaluationError>("1 << null");
        }
        
        TEST(ExpressionTest, testBitwiseShiftRightOperator) {
            evaluateAndAssert("1 >> 2", 1 >> 2);
#ifndef _WIN32
            evaluateAndAssert("1 >> 33", 1l >> 33);
#endif
            evaluateAndThrow<EvaluationError>("true >> 2");
            evaluateAndThrow<EvaluationError>("1 >> false");
            evaluateAndThrow<EvaluationError>("'asdf' >> 2");
            evaluateAndThrow<EvaluationError>("1 >> 'asdf'");
            evaluateAndThrow<EvaluationError>("[] >> 2");
            evaluateAndThrow<EvaluationError>("1 >> []");
            evaluateAndThrow<EvaluationError>("{} >> 2");
            evaluateAndThrow<EvaluationError>("1 >> {}");
            evaluateAndThrow<EvaluationError>("null >> 2");
            evaluateAndThrow<EvaluationError>("1 >> null");
        }
        
        TEST(ExpressionTest, testArithmeticPrecedence) {
            evaluateAndAssert("1 + 2 - 3", 1.0 + 2.0 - 3.0);
            evaluateAndAssert("1 - 2 + 3", 1.0 - 2.0 + 3.0);
            
            evaluateAndAssert("2 * 3 + 4", 2.0 * 3.0 + 4.0);
            evaluateAndAssert("2 + 3 * 4", 2.0 + 3.0 * 4.0);
            
            evaluateAndAssert("2 * 3 - 4", 2.0 * 3.0 - 4.0);
            evaluateAndAssert("2 - 3 * 4", 2.0 - 3.0 * 4.0);
 
            evaluateAndAssert("6 / 2 + 4", 6.0 / 2.0 + 4);
            evaluateAndAssert("6 + 2 / 4", 6.0 + 2.0 / 4.0);
            
            evaluateAndAssert("6 / 2 - 4", 6.0 / 2.0 - 4.0);
            evaluateAndAssert("6 - 2 / 4", 6.0 - 2.0 / 4.0);

            evaluateAndAssert("2 * 6 / 4", 2.0 * 6.0 / 4.0);
            evaluateAndAssert("2 / 6 * 4", 2.0 / 6.0 * 4.0);
        }
        
        TEST(ExpressionTest, testLogicalPrecedence) {
            evaluateAndAssert("false && false || true",   true);
            evaluateAndAssert("!true && !true || !false", true);
        }
        
        TEST(ExpressionTest, testLogicalAndComparisonPrecedence) {
            evaluateAndAssert("3 < 10 || 10 > 2", true);
        }
        
        TEST(ExpressionTest, testArithmeticAndComparisonPrecedence) {
            evaluateAndAssert("2 + 3 < 2 + 4", 1);
        }

        TEST(ExpressionTest, testCaseExpression) {
            evaluateAndAssert("true && false -> true", Value::Undefined);
            evaluateAndAssert("true && true -> false", false);
            evaluateAndAssert("2 + 3 < 2 + 4 -> 6 % 5", 1);
        }
        
        void evalutateComparisonAndAssert(const String& op, bool result) {
            const String expression = "4 " + op + " 5";
            evaluateAndAssert(expression, result);
            assertOptimizable(expression);
        }
        
        void evaluateAndAssert(const String& expression, const Value& result, const EvaluationContext& context) {
            ASSERT_EQ(result, IO::ELParser(expression).parse().evaluate(context));
        }
        
        void assertOptimizable(const String& expression) {
            ASSERT_TRUE(IO::ELParser(expression).parse().optimize());
        }
        
        void assertNotOptimizable(const String& expression) {
            ASSERT_FALSE(IO::ELParser(expression).parse().optimize());
        }
    }
}
