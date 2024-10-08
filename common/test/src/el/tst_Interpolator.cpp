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

#include "el/EvaluationContext.h"
#include "el/Interpolator.h"
#include "el/Value.h"

#include <string>

#include "Catch2.h"

namespace tb::el
{
namespace
{
void interpolateAndCheck(
  const std::string& expression,
  const std::string& expected,
  const EvaluationContext& context = EvaluationContext())
{
  CHECK(Interpolator(expression).interpolate(context) == expected);
}

} // namespace

TEST_CASE("ELInterpolatorTest.interpolateEmptyString")
{
  interpolateAndCheck("", "");
  interpolateAndCheck("   ", "   ");
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithoutExpression")
{
  interpolateAndCheck(" asdfasdf  sdf ", " asdfasdf  sdf ");
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithSimpleExpression")
{
  interpolateAndCheck(" asdfasdf ${'asdf'}  sdf ", " asdfasdf asdf  sdf ");
  interpolateAndCheck(" asdfasdf ${'asdf'} ${'AND'}  sdf ", " asdfasdf asdf AND  sdf ");
  interpolateAndCheck(" asdfasdf ${'asdf'}${' AND'}  sdf ", " asdfasdf asdf AND  sdf ");
  interpolateAndCheck(" ${ true } ", " true ");
  interpolateAndCheck(" ${ 'this'+' and ' }${'that'} ", " this and that ");
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithNestedExpression")
{
  interpolateAndCheck(
    " asdfasdf ${ 'nested ${TEST} expression' }  sdf ",
    " asdfasdf nested ${TEST} expression  sdf ");
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithVariable")
{
  EvaluationContext context;
  context.declareVariable("TEST", Value("interesting"));
  interpolateAndCheck(" an ${TEST} expression", " an interesting expression", context);
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithBackslashAndVariable")
{
  EvaluationContext context;
  context.declareVariable("TEST", Value("interesting"));
  interpolateAndCheck(
    " an \\${TEST} expression", " an \\interesting expression", context);
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithUnknownVariable")
{
  EvaluationContext context;
  CHECK_THROWS(interpolate(" an ${TEST} expression", context));
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithUnterminatedEL")
{
  EvaluationContext context;
  CHECK_THROWS(interpolate(" an ${TEST", context));
  CHECK_THROWS(interpolate(" an ${TEST expression", context));
}

} // namespace tb::el
