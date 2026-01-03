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

#include "el/ELParser.h"
#include "el/TestUtils.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::el
{
namespace
{

auto parse(const std::string& str)
{
  return ELParser{ParseMode::Strict, str}.parse();
}

} // namespace

TEST_CASE("ELParser")
{
  SECTION("emptyExpression")
  {
    CHECK(parse("").is_error());
    CHECK(parse("    ").is_error());
    CHECK(parse("\n").is_error());
  }

  SECTION("Literals")
  {
    SECTION("Strings")
    {
      CHECK(parse(R"("asdf)").is_error());
      CHECK(parse(R"("asdf")") == lit("asdf"));
      // MSVC complains about an illegal escape sequence if we use a raw string literal
      // for the expression
      CHECK(parse("\"asdf\\\" \\\"asdf\"") == lit(R"(asdf" "asdf)"));
    }

    SECTION("Numbers")
    {
      CHECK(parse("1.123.34").is_error());

      CHECK(parse("1") == lit(1.0));
      CHECK(parse("1.0") == lit(1.0));
      CHECK(parse("01.00") == lit(1.0));
      CHECK(parse(".0") == lit(0.0));
      CHECK(parse("0") == lit(0.0));
    }

    SECTION("Booleans")
    {
      CHECK(parse("true") == lit(true));
      CHECK(parse("false") == lit(false));
    }

    SECTION("Arrays")
    {
      CHECK(parse("[]") == arr());
      CHECK(
        parse(R"([ 1.0 , "test",[ true] ])")
        == arr({
          lit(1.0),
          lit("test"),
          arr({lit(true)}),
        }));
    }

    SECTION("Ranges")
    {
      CHECK(parse("[1..3]") == arr({bRng(lit(1.0), lit(3.0))}));
      CHECK(parse("[][1..3]") == scr(arr(), bRng(lit(1.0), lit(3.0))));
      CHECK(parse("[][..3]") == scr(arr(), rbRng(lit(3.0))));
      CHECK(parse("[][1..]") == scr(arr(), lbRng(lit(1.0))));
    }

    SECTION("Maps")
    {
      CHECK(parse("{}") == map());
      CHECK(
        parse(
          R"( { "testkey1": 1, "testkey2"   :"asdf", "testkey3":{"nestedKey":true} })")
        == map({
          {"testkey1", lit(1.0)},
          {"testkey2", lit("asdf")},
          {"testkey3", map({{"nestedKey", lit(true)}})},
        }));
      CHECK(parse(R"([ { "key": "value" } ])") == arr({map({{"key", lit("value")}})}));
      CHECK(
        parse(R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })")
        == map({
          {"outerkey1", arr({map({{"key", lit("value")}})})},
          {"outerkey2", lit("asdf")},
        }));

      CHECK(parse(R"({
  "profiles": [],
  "version": 1
}
asdf)")
              .is_error());
    }
  }

  SECTION("Variables")
  {
    CHECK(parse("test") == var("test"));
  }

  SECTION("Unary operators")
  {
    SECTION("Unary plus")
    {
      CHECK(parse("+1.0") == plus(lit(1)));
    }

    SECTION("Unary minus")
    {
      CHECK(parse("-1.0") == minus(lit(1)));
    }

    SECTION("Logical negation")
    {
      CHECK(parse("!true") == logNeg(lit(true)));
      CHECK(parse("!false") == logNeg(lit(false)));
      CHECK(parse("!0") == logNeg(lit(0)));
    }

    SECTION("Bitwise negation")
    {
      CHECK(parse("~393") == bitNeg(lit(393)));
      CHECK(parse("~").is_error());
      CHECK(parse("~~").is_error());
    }
  }

  SECTION("Binary operators")
  {
    SECTION("Addition")
    {
      CHECK(parse("2 + 3") == add(lit(2.0), lit(3.0)));
      CHECK(parse("\"as\"+\"df\"") == add(lit("as"), lit("df")));
      CHECK(parse("2 + 3 + 4") == add(add(lit(2.0), lit(3.0)), lit(4.0)));
    }

    SECTION("Subtraction")
    {
      CHECK(parse("2-3.0") == sub(lit(2.0), lit(3.0)));
      CHECK(parse("2-3 -  4") == sub(sub(lit(2.0), lit(3.0)), lit(4.0)));
      CHECK(parse("2-3-4-2") == sub(sub(sub(lit(2.0), lit(3.0)), lit(4.0)), lit(2.0)));
    }

    SECTION("Multiplication")
    {
      CHECK(parse("2 * 3.0") == mul(lit(2.0), lit(3.0)));
      CHECK(parse("2 * 3 * 4") == mul(mul(lit(2.0), lit(3.0)), lit(4.0)));
    }

    SECTION("Division")
    {
      CHECK(parse("12 / 2.0") == div(lit(12.0), lit(2.0)));
      CHECK(parse("12 / 2 / 2") == div(div(lit(12.0), lit(2.0)), lit(2.0)));
    }

    SECTION("Modulus")
    {
      CHECK(parse("12 % 2.0") == mod(lit(12.0), lit(2.0)));
      CHECK(parse("12 % 5 % 3") == mod(mod(lit(12.0), lit(5.0)), lit(3.0)));
    }

    SECTION("Logical and")
    {
      CHECK(parse("true && true") == logAnd(lit(true), lit(true)));
    }

    SECTION("Logical or")
    {
      CHECK(parse("true || true") == logOr(lit(true), lit(true)));
    }

    SECTION("Bitwise and")
    {
      CHECK(parse("23 & 24") == bitAnd(lit(23), lit(24)));
    }

    SECTION("Bitwise or")
    {
      CHECK(parse("23 | 24") == bitOr(lit(23), lit(24)));
    }

    SECTION("Bitwise xor")
    {
      CHECK(parse("23 ^ 24") == bitXOr(lit(23), lit(24)));
      CHECK(parse("23 ^^ 23").is_error());
    }

    SECTION("Bitwise shift left")
    {
      CHECK(parse("1 << 7") == bitShL(lit(1), lit(7)));
    }

    SECTION("Bitwise shift right")
    {
      CHECK(parse("8 >> 2") == bitShR(lit(8), lit(2)));
    }

    SECTION("Case operator")
    {
      CHECK(parse("true -> 1") == cs(lit(true), lit(1)));
    }

    SECTION("Comparison operators")
    {
      CHECK(parse("1 < 2") == ls(lit(1), lit(2)));
      CHECK(parse("1 <= 2") == lsEq(lit(1), lit(2)));
      CHECK(parse("1 > 2") == gr(lit(1), lit(2)));
      CHECK(parse("1 >= 2") == grEq(lit(1), lit(2)));
      CHECK(parse("1 == 2") == eq(lit(1), lit(2)));
      CHECK(parse("1 != 2") == neq(lit(1), lit(2)));
    }

    SECTION("Operator combinations")
    {
      CHECK(parse("1 + 2 * 3") == add(lit(1), mul(lit(2), lit(3))));
      CHECK(parse("1 * 2 + 3") == add(mul(lit(1), lit(2)), lit(3)));
      CHECK(parse("1 + 2 * 3 + 2") == add(add(lit(1), mul(lit(2), lit(3))), lit(2)));
      CHECK(
        parse("1 + 2 * 3 + 2 * 2")
        == add(add(lit(1), mul(lit(2), lit(3))), mul(lit(2), lit(2))));
      CHECK(parse("3 + 2 < 3 + 3") == ls(add(lit(3), lit(2)), add(lit(3), lit(3))));
      CHECK(
        parse("3 + 2 < 3 + 3 + 0 && true")
        == logAnd(ls(add(lit(3), lit(2)), add(add(lit(3), lit(3)), lit(0))), lit(true)));
      CHECK(
        parse("false && false || true")
        == logOr(logAnd(lit(false), lit(false)), lit(true)));
      CHECK(
        parse("false && (false || true)")
        == logAnd(lit(false), grp(logOr(lit(false), lit(true)))));
    }
  }

  SECTION("Subscript")
  {
    CHECK(
      parse(R"([ 1.0, 2.0, "test" ][0])")
      == scr(arr({lit(1.0), lit(2.0), lit("test")}), lit(0)));
    CHECK(
      parse(R"([ 1.0, 2.0, "test" ][1+1])")
      == scr(arr({lit(1.0), lit(2.0), lit("test")}), add(lit(1), lit(1))));
    CHECK(
      parse(R"({ "key1":1, "key2":2, "key3":"test"}["key1"])")
      == scr(
        map({
          {"key1", lit(1.0)},
          {"key2", lit(2.0)},
          {"key3", lit("test")},
        }),
        lit("key1")));
    CHECK(
      parse(R"([ 1.0, [ 2.0, "test"] ][1][0])")
      == scr(scr(arr({lit(1.0), arr({lit(2.0), lit("test")})}), lit(1)), lit(0)));
    CHECK(
      parse(R"([ 1.0, 2.0, "test" ][0,1,2])")
      == scr(arr({lit(1.0), lit(2.0), lit("test")}), arr({lit(0), lit(1), lit(2)})));
    CHECK(
      parse(R"([ 1.0, 2.0, "test" ][0..2])")
      == scr(arr({lit(1.0), lit(2.0), lit("test")}), bRng(lit(0), lit(2))));
    CHECK(
      parse(R"([ 1.0, 2.0, "test" ][0..2,3])")
      == scr(
        arr({lit(1.0), lit(2.0), lit("test")}), arr({bRng(lit(0), lit(2)), lit(3)})));
  }

  SECTION("Switch")
  {
    CHECK(parse("{{}}") == swt({}));
    CHECK(parse("{{'asdf'}}") == swt({lit("asdf")}));
    CHECK(parse("{{'fdsa', 'asdf'}}") == swt({lit("fdsa"), lit("asdf")}));
    CHECK(
      parse("{{false -> 'fdsa', 'asdf'}}")
      == swt({cs(lit(false), lit("fdsa")), lit("asdf")}));
  }

  SECTION("Groups")
  {
    CHECK(parse("()").is_error());
    CHECK(parse("(1)") == grp(lit(1)));
    CHECK(parse("(2+1)*3") == mul(grp(add(lit(2), lit(1))), lit(3)));
    CHECK(
      parse("(2+1)*(2+1)") == mul(grp(add(lit(2), lit(1))), grp(add(lit(2), lit(1)))));
    CHECK(
      parse("(2+1)*((1+1)*2)")
      == mul(grp(add(lit(2), lit(1))), grp(mul(grp(add(lit(1), lit(1))), lit(2)))));
  }
}

} // namespace tb::el
