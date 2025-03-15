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

#include "el/ELExceptions.h"
#include "el/EvaluationContext.h"
#include "el/Expression.h"
#include "el/Value.h"
#include "io/ELParser.h"

#include <string>

#include "Catch2.h"

namespace tb::io
{
using namespace el;

namespace
{

auto evaluate(
  const std::string& str, const EvaluationContext& context = EvaluationContext{})
{
  return ELParser::parseStrict(str).evaluate(context);
}

} // namespace

TEST_CASE("ELParser")
{
  SECTION("emptyExpression")
  {
    CHECK_THROWS_AS(evaluate(""), ParserException);
    CHECK_THROWS_AS(evaluate("    "), ParserException);
    CHECK_THROWS_AS(evaluate("\n"), ParserException);
  }

  SECTION("Literals")
  {
    SECTION("String")
    {
      CHECK_THROWS_AS(evaluate(R"("asdf)"), ParserException);

      const auto context = EvaluationContext{};
      CHECK(evaluate(R"("asdf")") == Value{"asdf"});
      // MSVC complains about an illegal escape sequence if we use a raw string literal
      // for the expression
      CHECK(evaluate("\"asdf\\\" \\\"asdf\"") == Value(R"(asdf" "asdf)"));
    }

    SECTION("Numbers")
    {
      CHECK_THROWS_AS(evaluate("1.123.34"), ParserException);

      CHECK(evaluate("1") == Value{1.0});
      CHECK(evaluate("1.0") == Value{1.0});
      CHECK(evaluate("01.00") == Value{1.0});
      CHECK(evaluate(".0") == Value{0.0});
      CHECK(evaluate("0") == Value{0.0});
    }

    SECTION("Booleans")
    {
      CHECK(evaluate("true") == Value{true});
      CHECK(evaluate("false") == Value{false});
    }

    SECTION("Arrays")
    {
      CHECK(evaluate("[]") == Value{ArrayType{}});
      CHECK(
        evaluate(R"([ 1.0 , "test",[ true] ])")
        == Value{ArrayType{{
          Value{1.0},
          Value{"test"},
          Value{ArrayType{{
            Value{true},
          }}},
        }}});

      CHECK(
        evaluate("[1..3]")
        == Value{ArrayType{{
          Value{1.0},
          Value{2.0},
          Value{3.0},
        }}});
      CHECK(
        evaluate("[3..1]")
        == Value{ArrayType{{
          Value{3.0},
          Value{2.0},
          Value{1.0},
        }}});
      CHECK(
        evaluate("[1..1]")
        == Value{ArrayType{{
          Value{1.0},
        }}});
      CHECK(
        evaluate("[1..0]")
        == Value{ArrayType{{
          Value{1.0},
          Value{0.0},
        }}});
      CHECK(
        evaluate("[-2..1]")
        == Value{ArrayType{{
          Value{-2.0},
          Value{-1.0},
          Value{0.0},
          Value{1.0},
        }}});
    }

    SECTION("Maps")
    {
      CHECK(evaluate("{}") == Value{MapType{}});
      CHECK(
        evaluate(
          R"( { "testkey1": 1, "testkey2"   :"asdf", "testkey3":{"nestedKey":true} })")
        == Value{MapType{
          {"testkey1", Value{1.0}},
          {"testkey2", Value{"asdf"}},
          {"testkey3",
           Value{MapType{
             {"nestedKey", Value{true}},
           }}},
        }});

      SECTION("Map nested in array")
      {
        CHECK(
          evaluate(R"([ { "key": "value" } ])")
          == Value{ArrayType{{
            Value{MapType{
              {"key", Value{"value"}},
            }},
          }}});
      }

      SECTION("Map nested in nested array")
      {
        CHECK(
          evaluate(R"({ "outerkey1": [ { "key": "value" } ], "outerkey2": "asdf" })")
          == Value{MapType{
            {"outerkey1",
             Value{ArrayType{{
               el::Value{el::MapType{
                 {"key", el::Value{"value"}},
               }},
             }}}},
            {"outerkey2", el::Value{"asdf"}},
          }});
      }

      SECTION("Map with trailing garbage")
      {
        CHECK_THROWS_AS(
          evaluate(R"({
	"profiles": [],
	"version": 1
}
asdf)"),
          ParserException);
      }
    }
  }

  SECTION("Variables")
  {
    auto context = el::EvaluationContext{};
    context.declareVariable("test", el::Value{1.0});

    CHECK(evaluate("test", context) == el::Value{1.0});
  }

  SECTION("Unary operators")
  {
    SECTION("Unary plus")
    {
      CHECK(evaluate("+1.0") == el::Value{1.0});
    }

    SECTION("Unary minus")
    {
      CHECK(evaluate("-1.0") == el::Value{-1.0});
    }

    SECTION("Logical negation")
    {
      CHECK(evaluate("!true") == el::Value{false});
      CHECK(evaluate("!false") == el::Value{true});
      CHECK_THROWS_AS(evaluate("!0"), el::EvaluationError);
      CHECK_THROWS_AS(evaluate("!1"), el::EvaluationError);
      CHECK_THROWS_AS(evaluate("!'true'"), el::EvaluationError);
    }

    SECTION("Bitwise negation")
    {
      CHECK(evaluate("~393") == el::Value{~393});
      CHECK(evaluate("~1") == el::Value{~1l});
      CHECK_THROWS_AS(evaluate("~"), ParserException);
      CHECK_THROWS_AS(evaluate("~~"), ParserException);
    }
  }

  SECTION("Binary operators")
  {
    SECTION("Addition")
    {
      CHECK(evaluate("2 + 3") == el::Value{5.0});
      CHECK(evaluate("\"as\"+\"df\"") == el::Value{"asdf"});
      CHECK(evaluate("2 + 3 + 4") == el::Value{9.0});
    }

    SECTION("Subtraction")
    {
      CHECK(evaluate("2 - 3.0") == el::Value{-1.0});
      CHECK(evaluate("2 - 3 - 4") == el::Value{-5.0});
      CHECK(evaluate("2 - 3 - 4 - 2") == el::Value{-7.0});
    }

    SECTION("Multiplication")
    {
      CHECK(evaluate("2 * 3.0") == el::Value{6.0});

      CHECK(evaluate("2 * 3 * 4") == el::Value{24.0});
      CHECK(evaluate("2 * 3 * 4 * 2") == el::Value{48.0});
    }

    SECTION("Division")
    {
      CHECK(evaluate("12 / 2.0") == el::Value{6.0});
      CHECK(evaluate("12 / 2 / 2") == el::Value{3.0});
      CHECK(evaluate("12 / 2 / 2 / 3") == el::Value{1.0});
    }

    SECTION("Modulus")
    {
      CHECK(evaluate("12 % 2.0") == el::Value{0.0});
      CHECK(evaluate("12 % 5 % 3") == el::Value{2.0});
      CHECK(evaluate("12 % 5 % 3 % 3") == el::Value{2.0});
    }

    SECTION("Logical and")
    {
      CHECK(evaluate("true && true") == el::Value{true});
      CHECK(evaluate("false && true") == el::Value{false});
      CHECK(evaluate("true && false") == el::Value{false});
      CHECK(evaluate("false && false") == el::Value{false});
    }

    SECTION("Logical or")
    {
      CHECK(evaluate("true || true") == el::Value{true});
      CHECK(evaluate("false || true") == el::Value{true});
      CHECK(evaluate("true || false") == el::Value{true});
      CHECK(evaluate("false || false") == el::Value{false});
    }

    SECTION("Bitwise and")
    {
      CHECK(evaluate("23 & 24") == el::Value{23 & 24});
    }

    SECTION("Bitwise or")
    {
      CHECK(evaluate("23 | 24") == el::Value{23 | 24});
    }

    SECTION("Bitwise xor")
    {
      CHECK(evaluate("23 ^ 24") == el::Value{(23 ^ 24)});
      CHECK_THROWS_AS(evaluate("23 ^^ 23"), ParserException);
    }

    SECTION("Bitwise shift left")
    {
      CHECK(evaluate("1 << 7") == el::Value{1 << 7});
    }

    SECTION("Bitwise shift right")
    {
      CHECK(evaluate("8 >> 2") == el::Value{8 >> 2});
    }

    SECTION("Case operator")
    {
      CHECK(evaluate("true -> false") == el::Value{false});
      CHECK(evaluate("true -> true && true") == el::Value{true});
      CHECK(evaluate("1 < 3 -> 2 + 3") == el::Value{5});
      CHECK(evaluate("false -> true") == el::Value::Undefined);
    }

    SECTION("Comparison operators")
    {
      CHECK(evaluate(R"(1 < 2)") == el::Value{true});
      CHECK(evaluate(R"(2 < 2)") == el::Value{false});
      CHECK(evaluate(R"(1 <= 2)") == el::Value{true});
      CHECK(evaluate(R"(2 <= 2)") == el::Value{true});
      CHECK(evaluate(R"(3 <= 2)") == el::Value{false});

      CHECK(evaluate(R"("test" == "test")") == el::Value{true});
      CHECK(evaluate(R"("test1" == "test")") == el::Value{false});
      CHECK(evaluate(R"("test" != "test")") == el::Value{false});
      CHECK(evaluate(R"("test1" != "test")") == el::Value{true});

      CHECK(evaluate(R"(2 > 1)") == el::Value{true});
      CHECK(evaluate(R"(2 > 2)") == el::Value{false});
      CHECK(evaluate(R"(2 >= 1)") == el::Value{true});
      CHECK(evaluate(R"(2 >= 2)") == el::Value{true});
      CHECK(evaluate(R"(2 >= 3)") == el::Value{false});
    }

    SECTION("Operator precedence")
    {
      CHECK(evaluate("7 + 2 * 3") == evaluate("2 * 3 + 7"));
      CHECK(evaluate("7 + 2 * 3 + 2") == evaluate("2 * 3 + 7 + 2"));
      CHECK(evaluate("7 + 2 * 3 + 2 * 2") == evaluate("2 * 3 + 7 + 2 * 2"));
      CHECK(evaluate("7 + 2 / 3 + 2 * 2") == evaluate("2 / 3 + 7 + 2 * 2"));

      CHECK(evaluate("3 + 2 < 3 + 3") == evaluate("(3 + 2) < (3 + 3)"));
      CHECK(
        evaluate("3 + 2 < 3 + 3 + 0 && true")
        == evaluate("((3 + 2) < (3 + 3 + 0)) && true"));
      CHECK(evaluate("false && false || true") == el::Value{true});
      CHECK(evaluate("false && (false || true)") == el::Value{false});
    }
  }

  SECTION("Subscript")
  {
    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][0])") == el::Value{1.0});
    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][1])") == el::Value{2.0});
    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][2])") == el::Value{"test"});
    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-1])") == el::Value{"test"});
    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-2])") == el::Value{2.0});
    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][-3])") == el::Value{1.0});

    CHECK(evaluate(R"([ 1.0, 2.0, "test" ][1 + 1])") == el::Value{"test"});

    CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key1"])") == el::Value{1.0});
    CHECK(evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key2"])") == el::Value{2.0});
    CHECK(
      evaluate(R"({ "key1":1, "key2":2, "key3":"test"}["key3"])") == el::Value{"test"});

    CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][0])") == el::Value{1.0});
    CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][1][0])") == el::Value{2.0});
    CHECK(evaluate(R"([ 1.0, [ 2.0, "test"] ][1][1])") == el::Value{"test"});

    CHECK(
      evaluate(R"({ "key1":1, "key2":2, "key3":[ 1, 2]}["key3"][1])") == el::Value{2.0});

    CHECK(
      evaluate(R"([ 1.0, 2.0, "test" ][0,1,2])")
      == el::Value{el::ArrayType{{
        el::Value{1.0},
        el::Value{2.0},
        el::Value{"test"},
      }}});
    CHECK(
      evaluate(R"([ 1.0, 2.0, "test" ][0..2])")
      == el::Value{el::ArrayType{{
        el::Value{1.0},
        el::Value{2.0},
        el::Value{"test"},
      }}});
    CHECK(
      evaluate(R"([ 1.0, 2.0, "test" ][2..0])")
      == el::Value{el::ArrayType{{
        el::Value{"test"},
        el::Value{2.0},
        el::Value{1.0},
      }}});
    CHECK(
      evaluate(R"([ 1.0, 2.0, "test" ][0,1..2])")
      == el::Value{el::ArrayType{{
        el::Value{1.0},
        el::Value{2.0},
        el::Value{"test"},
      }}});
    CHECK(
      evaluate(R"([ 1.0, 2.0, "test" ][1..])")
      == el::Value{el::ArrayType{{
        el::Value{2.0},
        el::Value{"test"},
      }}});
    CHECK(
      evaluate(R"([ 1.0, 2.0, "test" ][..1])")
      == el::Value{el::ArrayType{{
        el::Value{"test"},
        el::Value{2.0},
      }}});

    CHECK(evaluate(R"("test"[3,2,1,0])") == el::Value{"tset"});
    CHECK(evaluate(R"("test"[2,1,0])") == el::Value{"set"});
    CHECK(evaluate(R"("test"[2..1])") == el::Value{"se"});

    CHECK(evaluate(R"("test"[..0])") == el::Value{"tset"});
    CHECK(evaluate(R"("test"[1..])") == el::Value{"est"});
  }

  SECTION("Switch")
  {
    CHECK(evaluate("{{}}") == el::Value::Undefined);
    CHECK(evaluate("{{'asdf'}}") == el::Value{"asdf"});
    CHECK(evaluate("{{'fdsa', 'asdf'}}") == el::Value{"fdsa"});
    CHECK(evaluate("{{false -> 'fdsa', 'asdf'}}") == el::Value{"asdf"});
    CHECK(evaluate("{{false -> false}}") == el::Value::Undefined);
  }

  SECTION("Groups")
  {
    CHECK_THROWS_AS(evaluate("()"), ParserException);
    CHECK(evaluate("(1)") == el::Value{1.0});
    CHECK(evaluate("(2+1)*3") == el::Value{9.0});
    CHECK(evaluate("(2+1)*(2+1)") == el::Value{9.0});
    CHECK(evaluate("(2+1)*((1+1)*2)") == el::Value{12.0});
  }
}

} // namespace tb::io
