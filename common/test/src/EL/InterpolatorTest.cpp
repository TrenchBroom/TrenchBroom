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

#include "EL/Interpolator.h"
#include "EL/EvaluationContext.h"
#include "EL/Value.h"

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace EL
{
static void interpolateAndCheck(
  const std::string& expression,
  const std::string& expected,
  const EvaluationContext& context = EvaluationContext())
{
  CHECK(Interpolator(expression).interpolate(context) == expected);
}

TEST_CASE("ELInterpolatorTest.interpolateEmptyString", "[ELInterpolatorTest]")
{
  interpolateAndCheck("", "");
  interpolateAndCheck("   ", "   ");
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithoutExpression", "[ELInterpolatorTest]")
{
  interpolateAndCheck(" asdfasdf  sdf ", " asdfasdf  sdf ");
}

TEST_CASE(
  "ELInterpolatorTest.interpolateStringWithSimpleExpression", "[ELInterpolatorTest]")
{
  interpolateAndCheck(" asdfasdf ${'asdf'}  sdf ", " asdfasdf asdf  sdf ");
  interpolateAndCheck(" asdfasdf ${'asdf'} ${'AND'}  sdf ", " asdfasdf asdf AND  sdf ");
  interpolateAndCheck(" asdfasdf ${'asdf'}${' AND'}  sdf ", " asdfasdf asdf AND  sdf ");
  interpolateAndCheck(" ${ true } ", " true ");
  interpolateAndCheck(" ${ 'this'+' and ' }${'that'} ", " this and that ");
}

TEST_CASE(
  "ELInterpolatorTest.interpolateStringWithNestedExpression", "[ELInterpolatorTest]")
{
  interpolateAndCheck(
    " asdfasdf ${ 'nested ${TEST} expression' }  sdf ",
    " asdfasdf nested ${TEST} expression  sdf ");
}

TEST_CASE("ELInterpolatorTest.interpolateStringWithVariable", "[ELInterpolatorTest]")
{
  EvaluationContext context;
  context.declareVariable("TEST", Value("interesting"));
  interpolateAndCheck(" an ${TEST} expression", " an interesting expression", context);
}

TEST_CASE(
  "ELInterpolatorTest.interpolateStringWithBackslashAndVariable", "[ELInterpolatorTest]")
{
  EvaluationContext context;
  context.declareVariable("TEST", Value("interesting"));
  interpolateAndCheck(
    " an \\${TEST} expression", " an \\interesting expression", context);
}

TEST_CASE(
  "ELInterpolatorTest.interpolateStringWithUnknownVariable", "[ELInterpolatorTest]")
{
  EvaluationContext context;
  CHECK_THROWS(interpolate(" an ${TEST} expression", context));
}

TEST_CASE(
  "ELInterpolatorTest.interpolateStringWithUnterminatedEL", "[ELInterpolatorTest]")
{
  EvaluationContext context;
  CHECK_THROWS(interpolate(" an ${TEST", context));
  CHECK_THROWS(interpolate(" an ${TEST expression", context));
}
} // namespace EL
} // namespace TrenchBroom
