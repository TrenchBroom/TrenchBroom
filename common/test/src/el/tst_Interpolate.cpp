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
#include "el/Interpolate.h"
#include "el/Value.h"
#include "el/VariableStore.h"

#include <string>

#include "Catch2.h"

namespace tb::el
{
TEST_CASE("interpolate")
{

  SECTION("interpolateEmptyString")
  {
    auto context = EvaluationContext{};
    CHECK(interpolate("", context) == "");
    CHECK(interpolate("   ", context) == "   ");
  }

  SECTION("interpolateStringWithoutExpression")
  {
    auto context = EvaluationContext{};
    CHECK(interpolate(" asdfasdf  sdf ", context) == " asdfasdf  sdf ");
  }

  SECTION("interpolateStringWithSimpleExpression")
  {
    auto context = EvaluationContext{};
    CHECK(interpolate(" asdfasdf ${'asdf'}  sdf ", context) == " asdfasdf asdf  sdf ");
    CHECK(
      interpolate(" asdfasdf ${'asdf'} ${'AND'}  sdf ", context)
      == " asdfasdf asdf AND  sdf ");
    CHECK(
      interpolate(" asdfasdf ${'asdf'}${' AND'}  sdf ", context)
      == " asdfasdf asdf AND  sdf ");
    CHECK(interpolate(" ${ true } ", context) == " true ");
    CHECK(interpolate(" ${ 'this'+' and ' }${'that'} ", context) == " this and that ");
  }

  SECTION("interpolateStringWithNestedExpression")
  {
    auto context = EvaluationContext{};
    CHECK(
      interpolate(" asdfasdf ${ 'nested ${TEST} expression' }  sdf ", context)
      == " asdfasdf nested ${TEST} expression  sdf ");
  }

  SECTION("interpolateStringWithVariable")
  {
    auto context = EvaluationContext{VariableTable{{{"TEST", Value{"interesting"}}}}};
    CHECK(interpolate(" an ${TEST} expression", context) == " an interesting expression");
  }

  SECTION("interpolateStringWithBackslashAndVariable")
  {
    auto context = EvaluationContext{VariableTable{{{"TEST", Value{"interesting"}}}}};
    CHECK(
      interpolate(" an \\${TEST} expression", context) == " an \\interesting expression");
  }

  SECTION("interpolateStringWithUnknownVariable")
  {
    auto context = EvaluationContext{};
    CHECK(interpolate(" an ${TEST} expression", context).is_error());
  }

  SECTION("interpolateStringWithUnterminatedEL")
  {
    auto context = EvaluationContext{};
    CHECK(interpolate(" an ${TEST", context).is_error());
    CHECK(interpolate(" an ${TEST expression", context).is_error());
  }
}

} // namespace tb::el
