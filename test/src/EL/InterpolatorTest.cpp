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

#include <gtest/gtest.h>

#include "EL.h"
#include "EL/Interpolator.h"

namespace TrenchBroom {
    namespace EL {

        void ASSERT_EL(const String& expected, const String& expression, const EvaluationContext& context = EvaluationContext());
        void ASSERT_EL(const String& expected, const String& expression, const EvaluationContext& context) {
            ASSERT_EQ(expected, Interpolator(expression).interpolate(context));
        }
        
        TEST(ELInterpolatorTest, interpolateEmptyString) {
            ASSERT_EL("", "");
            ASSERT_EL("   ", "   ");
        }
        
        TEST(ELInterpolatorTest, interpolateStringWithoutExpression) {
            ASSERT_EL(" asdfasdf  sdf ", " asdfasdf  sdf ");
        }
        
        TEST(ELInterpolatorTest, interpolateStringWithSimpleExpression) {
            ASSERT_EL(" asdfasdf asdf  sdf ", " asdfasdf ${'asdf'}  sdf ");
            ASSERT_EL(" asdfasdf asdf AND  sdf ", " asdfasdf ${'asdf'} ${'AND'}  sdf ");
            ASSERT_EL(" asdfasdf asdf AND  sdf ", " asdfasdf ${'asdf'}${' AND'}  sdf ");
            ASSERT_EL(" true ", " ${ true } ");
            ASSERT_EL(" this and that ", " ${ 'this'+' and ' }${'that'} ");
        }
        
        TEST(ELInterpolatorTest, interpolateStringWithNestedExpression) {
            ASSERT_EL(" asdfasdf nested ${TEST} expression  sdf ", " asdfasdf ${ 'nested ${TEST} expression' }  sdf ");
        }
        
        TEST(ELInterpolatorTest, interpolateStringWithVariable) {
            EvaluationContext context;
            context.declareVariable("TEST", Value("interesting"));
            ASSERT_EL(" an interesting expression", " an ${TEST} expression", context);
        }
        
        TEST(ELInterpolatorTest, interpolateStringWithBackslashAndVariable) {
            EvaluationContext context;
            context.declareVariable("TEST", Value("interesting"));
            ASSERT_EL(" an \\interesting expression", " an \\${TEST} expression", context);
        }
    }
}
