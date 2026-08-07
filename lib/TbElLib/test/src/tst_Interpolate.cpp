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

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{
TEST_CASE("Interpolate")
{
  SECTION("interpolate")
  {
    SECTION("empty string")
    {
      const auto variables = VariableTable{};
      CHECK(interpolate({variables}, "") == "");
      CHECK(interpolate(variables, "   ") == "   ");
    }

    SECTION("string without expression")
    {
      const auto variables = VariableTable{};
      CHECK(interpolate(variables, " asdfasdf  sdf ") == " asdfasdf  sdf ");
    }

    SECTION("string with simple expression")
    {
      const auto variables = VariableTable{};
      CHECK(
        interpolate(variables, " asdfasdf ${'asdf'}  sdf ") == " asdfasdf asdf  sdf ");
      CHECK(
        interpolate(variables, " asdfasdf ${'asdf'} ${'AND'}  sdf ")
        == " asdfasdf asdf AND  sdf ");
      CHECK(
        interpolate(variables, " asdfasdf ${'asdf'}${' AND'}  sdf ")
        == " asdfasdf asdf AND  sdf ");
      CHECK(interpolate(variables, " ${ true } ") == " true ");
      CHECK(
        interpolate(variables, " ${ 'this'+' and ' }${'that'} ") == " this and that ");
    }

    SECTION("string with nested expression")
    {
      const auto variables = VariableTable{};
      CHECK(
        interpolate(variables, " asdfasdf ${ 'nested ${TEST} expression' }  sdf ")
        == " asdfasdf nested ${TEST} expression  sdf ");
    }

    SECTION("string with variable")
    {
      const auto variables = VariableTable{{{"TEST", Value{"interesting"}}}};
      CHECK(
        interpolate(variables, " an ${TEST} expression") == " an interesting expression");
    }

    SECTION("string with backslash and variable")
    {
      const auto variables = VariableTable{{{"TEST", Value{"interesting"}}}};
      CHECK(
        interpolate(variables, " an \\${TEST} expression")
        == " an \\interesting expression");
    }

    SECTION("string with unknown variable")
    {
      const auto variables = VariableTable{};
      CHECK(interpolate(variables, " an ${TEST} expression").is_error());
    }

    SECTION("string with unterminated EL")
    {
      const auto variables = VariableTable{};
      CHECK(interpolate(variables, " an ${TEST").is_error());
      CHECK(interpolate(variables, " an ${TEST expression").is_error());
    }
  }
}

} // namespace tb::el
