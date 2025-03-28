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
    const auto variables = VariableTable{};
    CHECK(interpolate({variables}, "") == "");
    CHECK(interpolate(variables, "   ") == "   ");
  }

  SECTION("interpolateStringWithoutExpression")
  {
    const auto variables = VariableTable{};
    CHECK(interpolate(variables, " asdfasdf  sdf ") == " asdfasdf  sdf ");
  }

  SECTION("interpolateStringWithSimpleExpression")
  {
    const auto variables = VariableTable{};
    CHECK(interpolate(variables, " asdfasdf ${'asdf'}  sdf ") == " asdfasdf asdf  sdf ");
    CHECK(
      interpolate(variables, " asdfasdf ${'asdf'} ${'AND'}  sdf ")
      == " asdfasdf asdf AND  sdf ");
    CHECK(
      interpolate(variables, " asdfasdf ${'asdf'}${' AND'}  sdf ")
      == " asdfasdf asdf AND  sdf ");
    CHECK(interpolate(variables, " ${ true } ") == " true ");
    CHECK(interpolate(variables, " ${ 'this'+' and ' }${'that'} ") == " this and that ");
  }

  SECTION("interpolateStringWithNestedExpression")
  {
    const auto variables = VariableTable{};
    CHECK(
      interpolate(variables, " asdfasdf ${ 'nested ${TEST} expression' }  sdf ")
      == " asdfasdf nested ${TEST} expression  sdf ");
  }

  SECTION("interpolateStringWithVariable")
  {
    const auto variables = VariableTable{{{"TEST", Value{"interesting"}}}};
    CHECK(
      interpolate(variables, " an ${TEST} expression") == " an interesting expression");
  }

  SECTION("interpolateStringWithBackslashAndVariable")
  {
    const auto variables = VariableTable{{{"TEST", Value{"interesting"}}}};
    CHECK(
      interpolate(variables, " an \\${TEST} expression")
      == " an \\interesting expression");
  }

  SECTION("interpolateStringWithUnknownVariable")
  {
    const auto variables = VariableTable{};
    CHECK(interpolate(variables, " an ${TEST} expression").is_error());
  }

  SECTION("interpolateStringWithUnterminatedEL")
  {
    const auto variables = VariableTable{};
    CHECK(interpolate(variables, " an ${TEST").is_error());
    CHECK(interpolate(variables, " an ${TEST expression").is_error());
  }
}

} // namespace tb::el
